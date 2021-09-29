#include "Renderer3d.h"

#include <stb_image.h>
#include <iostream>
#include <glm/glm.hpp>
#include <Utils/Timer.h>
#include <configNoisy.hpp>
#include "BillboardRenderer.h"
#include <fstream>

#include <Utils/DebugLayer.h>
#include <Utils/utils.h>

#define NS_IRRADIANCE_MAP_SAMPLER				28
#define NS_PREFILTERED_ENVIRONMENT_MAP_SAMPLER	29
#define NS_BRDF_LUT_MAP							30
#define NS_SHADOW_MAP_SAMPLER					31

ns::Renderer3d::Renderer3d(Window& window, Camera& camera, Scene& scene, const Renderer3dConfigInfo& info)
	:
	cam_(camera),
	win_(window),
	scene_(&scene),
	info_(info),
	skyBox(cam_, 0),
	previousResolution_(win_.size()),
	dirtMask_(NS_PATH"assets/textures/dirtMask.jpg")
{
	std::vector<ns::Shader::Define> defines{
		{"MAX_DIR_LIGHTS", std::to_string(info_.directionalLightsMax), ns::Shader::Stage::Fragment},
		{"MAX_POINT_LIGHTS", std::to_string(info_.pointLightsMax), ns::Shader::Stage::Fragment},
		{"MAX_SPOT_LIGHTS", std::to_string(info_.spotLightsMax), ns::Shader::Stage::Fragment},
	};
	pbr_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/main/renderer.vert", NS_PATH"assets/shaders/main/renderer.frag", nullptr, defines, true);

#	ifndef NDEBUG
	normalVisualizer_ = std::make_unique<ns::Shader>(
		NS_PATH"assets/shaders/debug/normalVisual.vert", 
		NS_PATH"assets/shaders/debug/normalVisual.frag", 
		NS_PATH"assets/shaders/debug/normalVisual.geom");
#	endif // !NDEBUG


	initPhysicallyBasedRenderingSystem(info.environmentMap);

	glDisable(GL_MULTISAMPLE);
	//transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//avoid visible cube edges of the cubemaps 
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glClearColor(0.f, 0.f, 0.f, 1.f);

	skyBox.setCubeMapTexture(environmentMap_);

	importFromYAML();
}

ns::Renderer3d::~Renderer3d()
{

	destroyBloomPipeline();

	exportIntoYAML();

	glDeleteTextures(1, &irradianceMap_);
	glDeleteBuffers(1, &planeBuffer_);
	glDeleteVertexArrays(1, &plane_);

	glDeleteFramebuffers(1, &shadowFramebuffer_);
	glDeleteTextures(1, &shadowMap_);
}

void ns::Renderer3d::startRendering()
{
	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	//render dynamic shadows
	if (info_.shadows)
		updateDynamicShadow(cam_.position(), scene_->getDirectionalLight().direction());

	//bind our custom FBO
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	//clear framebuffer color and depth attachements
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set viewport size
	glViewport(0, 0, win_.width(), win_.height());

	//if resolution changed
	if (previousResolution_ != win_.size()) {

		//resize framebuffer attachements
		glBindTexture(GL_TEXTURE_2D, colorAttachement_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.width(), win_.height(), 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, depthAttachement_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, win_.width(), win_.height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		//the name of the function is self-explanatory ;)
		resizeBloomPipeline();

		previousResolution_ = win_.size();
	}
	if (shadowMapRes_ != info_.shadowPrecision) {
		initShadowPipeline();
	}

	//render the scene in the main FBO
	draw();

	
}

void ns::Renderer3d::finishRendering()
{
	//BLOOM
	if (previousBloomIterationSetting_ != info_.bloomIteration) {
		initBloomPipeline();
	}

	if (info_.bloomIteration) {
		bloomPrefilteringStage_->set("threshold", info_.bloomThreshold);
		//prefiltering
		glActiveTexture(GL_TEXTURE2);
		dirtMask_.bind();
		bloomPrefilteringStage_->set("dirtMask", 2);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colorAttachement_);
		bloomPrefilteringStage_->set("inputTexture", 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bloomThresholdFiltered_.id);
		glBindImageTexture(0, bloomThresholdFiltered_.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

		bloomPrefilteringStage_->use();
		glDispatchCompute(bloomThresholdFiltered_.size.x / 32 + 1, bloomThresholdFiltered_.size.y / 32 + 1, 1);


		//downsampling
		for (size_t i = 0; i < bloomDownsampled_.size(); i++)
		{
			//choose input texture to blur
			GLuint inputTexture;
			if (i == 0)
				inputTexture = bloomThresholdFiltered_.id;
			else
				inputTexture = bloomDownsampled_[i - 1].id;

			//horizontal gaussian blur
			bloomDownsamplingStage_->set("inputTexture", 1);
			bloomDownsamplingStage_->set("horizontal", true);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, inputTexture);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id2);
			glBindImageTexture(0, bloomDownsampled_[i].id2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			bloomDownsamplingStage_->use();

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glDispatchCompute(bloomDownsampled_[i].size.x / 32 + 1, bloomDownsampled_[i].size.y / 32 + 1, 1);

			//vertical gaussian blur
			bloomDownsamplingStage_->set("horizontal", false);

			glActiveTexture(GL_TEXTURE2);
			dirtMask_.bind();
			bloomDownsamplingStage_->set("dirtMask", 2);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id2);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id);
			glBindImageTexture(0, bloomDownsampled_[i].id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			bloomDownsamplingStage_->use();

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glDispatchCompute(bloomDownsampled_[i].size.x / 32 + 1, bloomDownsampled_[i].size.y / 32 + 1, 1);

		}

		//upsampling
		for (size_t i = 0; i < bloomUpsampled_.size(); i++)
		{
			//choose first input texture
			GLuint inputTexture, inputTextureWithSameRes;
			if (i == 0)
				inputTexture = bloomDownsampled_.back().id;
			else
				inputTexture = bloomUpsampled_[i - 1].id;

			bloomUpsamplingStage_->set("inputTexture", 1);

			//choose second input texture
			if (i == bloomUpsampled_.size() - 1)
				inputTextureWithSameRes = colorAttachement_;
			else
				inputTextureWithSameRes = bloomDownsampled_[bloomDownsampled_.size() - 2].id;

			bloomUpsamplingStage_->set("inputTextureWithSameRes", 2);

			//compute the texture
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, inputTexture);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, inputTextureWithSameRes);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, bloomUpsampled_[i].id);
			glBindImageTexture(0, bloomUpsampled_[i].id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			bloomUpsamplingStage_->use();

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glDispatchCompute(bloomUpsampled_[i].size.x / 32 + 1, bloomUpsampled_[i].size.y / 32 + 1, 1);
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bloomUpsampled_.back().id);
	}
	else {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colorAttachement_);
	}

	//final post-processing layer
	FinalPostProcessingStage_->use();
	FinalPostProcessingStage_->set("inputTexture", 1);
	FinalPostProcessingStage_->set("enableFXAA", info_.FXAA);
	FinalPostProcessingStage_->set("exposure", info_.exposure);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, result_);
	glBindImageTexture(0, result_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glDispatchCompute(win_.width() / 32 + 1, win_.height() / 32 + 1, 1);
	
	//bind default framebuffer
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, win_.width(), win_.height());

	//draw result texture on a quad
	glBindVertexArray(plane_);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, result_);
	//glBindTexture(GL_TEXTURE_2D, colorAttachement_);

	screenShader_->use();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void ns::Renderer3d::draw()
{
	cam_.calculateMatrix(win_);
	
	if (info_.renderSkybox) skyBox.draw();

	scene_->sendLights(*pbr_);

	setDynamicUniforms(*pbr_);

	scene_->draw(*pbr_);

#	ifndef NDEBUG

	if (info_.showNormals) {
		normalVisualizer_->set<glm::mat4>("view", cam_.view());
		normalVisualizer_->set<glm::mat4>("projection", cam_.projection());
		scene_->draw(*normalVisualizer_);
	}

#	endif // !NDEBUG
}

void ns::Renderer3d::setCamera(Camera& camera)
{
	cam_ = camera;
}

void ns::Renderer3d::setScene(Scene& scene)
{
	scene_ = &scene;
}

ns::Renderer3dConfigInfo& ns::Renderer3d::settings()
{
	return info_;
}

void ns::Renderer3d::importFromYAML()
{
	try {
		auto clear = conf["renderer"]["clearColor"].as<glm::vec4>();
		glClearColor(clear.r, clear.g, clear.b, clear.a);
	
		info_.bloomIteration = conf["renderer"]["bloomIteration"].as<int>();
		info_.FXAA = conf["renderer"]["enableFXAA"].as<bool>();
		info_.showNormals = conf["renderer"]["showNormals"].as<bool>();
		info_.renderSkybox = conf["renderer"]["renderSkybox"].as<bool>();
		info_.bloomThreshold = conf["renderer"]["bloomThreshold"].as<float>();
		info_.shadowPrecision = conf["renderer"]["shadowPrecision"].as<int>();
		info_.shadowSize = conf["renderer"]["shadowSize"].as<int>();
		info_.exposure = conf["renderer"]["exposure"].as<float>();
		info_.shadows = conf["renderer"]["shadows"].as<bool>();
		info_.ambientIntensity = conf["renderer"]["ambientIntensity"].as<float>();
	}
	catch(...){}
}

void ns::Renderer3d::exportIntoYAML()
{
	conf["renderer"]["clearColor"] = ns::getClearColor();
	conf["renderer"]["bloomIteration"] = info_.bloomIteration;
	conf["renderer"]["enableFXAA"] = info_.FXAA;
	conf["renderer"]["showNormals"] = info_.showNormals;
	conf["renderer"]["renderSkybox"] = info_.renderSkybox;
	conf["renderer"]["bloomThreshold"] = info_.bloomThreshold;
	conf["renderer"]["shadowPrecision"] = info_.shadowPrecision;
	conf["renderer"]["shadowSize"] = info_.shadowSize;
	conf["renderer"]["exposure"] = info_.exposure;
	conf["renderer"]["shadows"] = info_.shadows;
	conf["renderer"]["ambientIntensity"] = info_.ambientIntensity;
}

void ns::Renderer3d::initPhysicallyBasedRenderingSystem(const std::string& envHdrMapPath)
{
	createCube();
	createPlaneMesh();

	loadEnvironmentMap(envHdrMapPath.c_str(), 1024);

	updateIrradianceMap();
	updatePreFilteredEnvironmentMap();
	updateBRDFpreComputing();
	sendFixDataToShader();

	glDeleteBuffers(1, &cubeBuffer_);
	glDeleteVertexArrays(1, &cube_);

	screenShader_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/main/screen.vert", NS_PATH"assets/shaders/main/screen.frag");
	bloomPrefilteringStage_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/compute/bloomPrefiltering.comp", std::vector<ns::Shader::Define>(), true);
	bloomDownsamplingStage_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/compute/gaussianBlur.comp", std::vector<ns::Shader::Define>(), true);
	bloomUpsamplingStage_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/compute/upsampling.comp", std::vector<ns::Shader::Define>(), true);
	FinalPostProcessingStage_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/compute/postProcess.comp", std::vector<ns::Shader::Define>(), true);

	shadowShader_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/main/shadow.vert", NS_PATH"assets/shaders/main/shadow.frag");

	initBloomPipeline();

	glGenFramebuffers(1, &shadowFramebuffer_);
	glGenTextures(1, &shadowMap_);
	initShadowPipeline();

	createFramebuffer();
}

void ns::Renderer3d::setDynamicUniforms(ns::Shader& shader) const
{
#	ifdef RUNTIME_SHADER_RECOMPILATION
	sendFixDataToShader();
#	endif // RUNTIME_SHADER_RECOMPILATION

	glActiveTexture(GL_TEXTURE0 + NS_IRRADIANCE_MAP_SAMPLER);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap_);

	glActiveTexture(GL_TEXTURE0 + NS_PREFILTERED_ENVIRONMENT_MAP_SAMPLER);
	glBindTexture(GL_TEXTURE_CUBE_MAP, preFilteredEnvironmentMap_);

	glActiveTexture(GL_TEXTURE0 + NS_BRDF_LUT_MAP);
	glBindTexture(GL_TEXTURE_2D, brdfMap_);

	glActiveTexture(GL_TEXTURE0 + NS_SHADOW_MAP_SAMPLER);
	if(info_.shadows) glBindTexture(GL_TEXTURE_2D, shadowMap_);
	else glBindTexture(GL_TEXTURE_2D, 0);

	shader.set("projView", cam_.projectionView());
	shader.set("model", glm::scale(glm::vec3(1)));
	shader.set("camPos", cam_.position());

	shader.set<int>("dirLightNumber", DirectionalLight::number());
	shader.set<int>("pointLightNumber", PointLight::number());
	shader.set<int>("spotLightNumber", SpotLight::number());

	shader.set("lightSpaceMatrix", lightMatrix_);
	shader.set("shadows", info_.shadows);
	shader.set("ambientIntensity", info_.ambientIntensity);
}

void ns::Renderer3d::initShadowPipeline()
{
	//this store the resolution of the depth buffer(two vars are needed to store the resolution needed and the actual resolution)
	shadowMapRes_ = info_.shadowPrecision;

	//configure the texture that will be fill by the framebuffer
	
	glBindTexture(GL_TEXTURE_2D, shadowMap_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapRes_, shadowMapRes_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//depth map parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//set the outer depth map color
	const float borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//attach the shadow map to the generated framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap_, 0);
	
	//remove color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind the shadow framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ns::Renderer3d::updateDynamicShadow(const glm::vec3& position, const glm::vec3& lightDir)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer_);
	glViewport(0, 0, shadowMapRes_, shadowMapRes_);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glm::mat4 lightProjection = glm::ortho<float>(-info_.shadowSize, info_.shadowSize, -info_.shadowSize, info_.shadowSize, -info_.shadowSize, info_.shadowSize);
	glm::mat4 lightView = glm::lookAt<float>(-lightDir + position, position, glm::vec3(0, 1, 0));

	lightMatrix_ = lightProjection * lightView;

	shadowShader_->set("lightSpaceMatrix", lightMatrix_);

	scene_->draw(*shadowShader_);

	glCullFace(GL_BACK);
}

void ns::Renderer3d::initBloomPipeline()
{
	destroyBloomPipeline();
	glGenTextures(1, &bloomThresholdFiltered_.id);
	glBindTexture(GL_TEXTURE_2D, bloomThresholdFiltered_.id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1000, 1000, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	bloomDownsampled_.resize(info_.bloomIteration);
	bloomUpsampled_.resize(info_.bloomIteration);

	for (size_t i = 0; i < bloomDownsampled_.size(); i++)
	{
		glGenTextures(1, &bloomDownsampled_[i].id);
		glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 100, 100, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenTextures(1, &bloomDownsampled_[i].id2);
		glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id2);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 100, 100, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	for (size_t i = 0; i < bloomUpsampled_.size(); i++)
	{
		glGenTextures(1, &bloomUpsampled_[i].id);
		glBindTexture(GL_TEXTURE_2D, bloomUpsampled_[i].id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 100, 100, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glGenTextures(1, &result_);
	glBindTexture(GL_TEXTURE_2D, result_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 100, 100, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	resizeBloomPipeline();
	previousBloomIterationSetting_ = info_.bloomIteration;
}

void ns::Renderer3d::resizeBloomPipeline()
{
	glm::ivec2 resolution = win_.size();

	glBindTexture(GL_TEXTURE_2D, bloomThresholdFiltered_.id);

	resolution /= 2;
	bloomThresholdFiltered_.size = resolution;

	//dout << "prefiltering res = " << ns::to_string(resolution) << '\n';

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bloomThresholdFiltered_.size.x, bloomThresholdFiltered_.size.y, 0, GL_RGBA, GL_FLOAT, NULL);

	for (size_t i = 0; i < bloomDownsampled_.size(); i++)
	{
		resolution /= 2;
		resolution.x = std::max(resolution.x, 5);
		resolution.y = std::max(resolution.y, 5);

		//dout << "downsampling n " << i << " = " << ns::to_string(resolution) << '\n';

		bloomDownsampled_[i].size = resolution;

		glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindTexture(GL_TEXTURE_2D, bloomDownsampled_[i].id2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	for (size_t i = 0; i < bloomUpsampled_.size(); i++)
	{
		int index = bloomDownsampled_.size() - i - 2;

		if (index < 0)
			resolution = win_.size();
		else
			resolution = bloomDownsampled_[index].size;

		//dout << "upsampling n " << i << " = " << ns::to_string(resolution) << '\n';

		bloomUpsampled_[i].size = resolution;
		glBindTexture(GL_TEXTURE_2D, bloomUpsampled_[i].id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	glBindTexture(GL_TEXTURE_2D, result_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.width(), win_.height(), 0, GL_RGBA, GL_FLOAT, NULL);
}

void ns::Renderer3d::destroyBloomPipeline()
{
	//delete pre filtering texture
	glDeleteTextures(1, &bloomThresholdFiltered_.id);

	//delete all the down samples textures
	for (size_t i = 0; i < bloomDownsampled_.size(); i++)
	{
		glDeleteTextures(1, &bloomDownsampled_[i].id);
	}
	bloomDownsampled_.clear();

	//delete all the up samples textures
	for (size_t i = 0; i < bloomUpsampled_.size(); i++)
	{
		glDeleteTextures(1, &bloomUpsampled_[i].id);
	}
	bloomUpsampled_.clear();

	//delete the result texture
	glDeleteTextures(1, &result_);
}

void ns::Renderer3d::createFramebuffer()
{
	//create layer framebuffer
	glGenFramebuffers(1, &framebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	//main color attachement
	glGenTextures(1, &colorAttachement_);
	glBindTexture(GL_TEXTURE_2D, colorAttachement_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.size().x, win_.size().y, 0, GL_RGBA, GL_FLOAT, NULL);
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, info_.samples, colorAttachementFormat_, win_.size().x, win_.size().y, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	float borderColor[] = { .0f, .0f, .0f, .0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach it to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachement_, 0);

	//create depth attachement
	glGenTextures(1, &depthAttachement_);
	glBindTexture(GL_TEXTURE_2D, depthAttachement_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, win_.size().x, win_.size().y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//attach depth map to the fbo 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachement_, 0);

	//check that the framebuffer is valid
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Debug::get() << "failed to create the postProcessing layer framebuffer !\n";
		return;
	}
}

void ns::Renderer3d::loadEnvironmentMap(const char* path, int res)
{
	ns::Shader equiRectToCubeMap(NS_PATH"assets/shaders/pbr/equirectangularToCubemap.vert", NS_PATH"assets/shaders/pbr/equirectangularToCubemap.frag");

	stbi_set_flip_vertically_on_load(true);

	int width, height;
	float* data = stbi_loadf(path, &width, &height, nullptr, 3);
	unsigned int hdrTexture;

	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		Debug::get() << "CubeMap error : can't load file : " << path << std::endl;
		return;
	}

	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, res, res);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	glGenTextures(1, &environmentMap_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap_);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			res, res, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// convert HDR equirectangular environment map to cubemap equivalent
	equiRectToCubeMap.set("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, res, res); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equiRectToCubeMap.set<glm::mat4>("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentMap_, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bool cull = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		glBindVertexArray(cube_);
		equiRectToCubeMap.use();
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6 * 3);

		if (cull)
			glEnable(GL_CULL_FACE);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteTextures(1, &hdrTexture);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	
}

void ns::Renderer3d::updateIrradianceMap()
{
	ns::Shader irradiance(NS_PATH"assets/shaders/pbr/irradianceMap.vert", NS_PATH"assets/shaders/pbr/irradianceMap.frag");

	unsigned int captureFBO;
	unsigned int captureRBO;

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	glGenTextures(1, &irradianceMap_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap_);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
			GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};


	irradiance.set("projection", captureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap_);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradiance.set("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap_, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bool cull = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		glBindVertexArray(cube_);
		irradiance.use();
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6 * 3);

		if (cull)
			glEnable(GL_CULL_FACE);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);
}

void ns::Renderer3d::updatePreFilteredEnvironmentMap()
{
	ns::Shader prefilter(NS_PATH"assets/shaders/pbr/preFilter.vert", NS_PATH"assets/shaders/pbr/preFilter.frag");
	constexpr int TEX_RES = 256;

	unsigned int captureFBO;
	unsigned int captureRBO;

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glGenTextures(1, &preFilteredEnvironmentMap_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, preFilteredEnvironmentMap_);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, TEX_RES, TEX_RES, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	prefilter.set("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap_);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// resize framebuffer according to mip-level size
		unsigned int mipWidth = TEX_RES * std::pow(.5f, mip);
		unsigned int mipHeight = TEX_RES * std::pow(.5f, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilter.set("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilter.set("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preFilteredEnvironmentMap_, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			bool cull = glIsEnabled(GL_CULL_FACE);
			glDisable(GL_CULL_FACE);

			glBindVertexArray(cube_);
			prefilter.use();
			glDrawArrays(GL_TRIANGLES, 0, 6 * 6 * 3);

			if (cull)
				glEnable(GL_CULL_FACE);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ns::Renderer3d::sendFixDataToShader() const
{
	pbr_->set("irradianceMap", NS_IRRADIANCE_MAP_SAMPLER);
	pbr_->set("prefilteredEnvironmentMap", NS_PREFILTERED_ENVIRONMENT_MAP_SAMPLER);
	pbr_->set("brdfLutMap", NS_BRDF_LUT_MAP);
	pbr_->set("shadowMap", NS_SHADOW_MAP_SAMPLER);
}

void ns::Renderer3d::updateBRDFpreComputing()
{
	ns::Shader brdf(NS_PATH"assets/shaders/pbr/brdf.vert", NS_PATH"assets/shaders/pbr/brdf.frag");

	constexpr int TEX_RES = 1024;
	bool blend = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);

	unsigned int captureFBO = NULL;
	unsigned int captureRBO = NULL;

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glGenTextures(1, &brdfMap_);

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfMap_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, TEX_RES, TEX_RES, 0, GL_RG, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, TEX_RES, TEX_RES);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfMap_, 0);

	glViewport(0, 0, TEX_RES, TEX_RES);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bool cull = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	brdf.use();

	glBindVertexArray(plane_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (cull)
		glEnable(GL_CULL_FACE);

	if (blend)
		glEnable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ns::Renderer3d::createCube()
{
	std::vector<float> vertices = {
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,

		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,

		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,

		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,

		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f
	};

	glGenVertexArrays(1, &cube_);
	glBindVertexArray(cube_);

	glGenBuffers(1, &cubeBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

void ns::Renderer3d::createPlaneMesh()
{
	std::vector<float> vert = {
		// positions			// texture Coords
		-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
	};

	glGenVertexArrays(1, &plane_);
	glBindVertexArray(plane_);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}