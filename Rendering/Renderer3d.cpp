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

ns::Renderer3d::Renderer3d(Window& window, Camera& camera, Scene& scene, const Renderer3dCreateInfo& info)
	:
	cam_(camera),
	win_(window),
	scene_(scene),
	info_(info),
	runTicks(true),
	skyBox(cam_, 0)
{
	std::vector<ns::Shader::Define> defines{
		{"MAX_DIR_LIGHTS", std::to_string(info_.directionalLightsMax), ns::Shader::Stage::Fragment},
		{"MAX_POINT_LIGHTS", std::to_string(info_.pointLightsMax), ns::Shader::Stage::Fragment},
		{"MAX_SPOT_LIGHTS", std::to_string(info_.spotLightsMax), ns::Shader::Stage::Fragment},
	};
	pbr_ = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/main/renderer.vert", NS_PATH"assets/shaders/main/renderer.frag", nullptr, defines, true);

	initPhysicallyBasedRenderingSystem(info.environmentMap);
	launchTickThread();

	glDisable(GL_MULTISAMPLE);
	//transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//avoid visible cube edges of the cubemaps 
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glClearColor(0.f, 0.f, 0.f, 1.f);

	skyBox.setCubeMapTexture(environmentMap_);

	importFromYAML(CONFIG_FILE);
}

ns::Renderer3d::~Renderer3d()
{
	runTicks = false;
	exportIntoYAML(CONFIG_FILE);
	glDeleteTextures(1, &irradianceMap_);
}

void ns::Renderer3d::startRendering()
{
	postProcess->bind();
	draw();
}

void ns::Renderer3d::finishRendering()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postProcess->colorMap2());
	glBindImageTexture(0, postProcess->colorMap2(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	gaussianBlur->use();


	const int resx = win_.size().x + 32 - win_.size().x % 32, resy = win_.size().y + 32 - win_.size().y % 32;
	
	for (uint8_t i = 0; i < info_.bloomIteration; i++)
	{
		gaussianBlur->set("horizontal", i % 2);
		glDispatchCompute(resx / 32, resy / 32, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	postProcess->computeShader().set("enableFXAA", info_.FXAA);
	postProcess->startProcessing();

	//final rendering on a quad
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, win_.width(), win_.height());

	postProcess->draw();
}

void ns::Renderer3d::draw()
{
	cam_.calculateMatrix(win_);
	//skyBox.draw();

	scene_.sendLights(*pbr_);

	setDynamicUniforms();

	scene_.draw(*pbr_);
}

void ns::Renderer3d::updateStationaries()
{
	scene_.updateStationaries();
}

void ns::Renderer3d::setCamera(Camera& camera)
{
	cam_ = camera;
}

void ns::Renderer3d::setScene(Scene& scene)
{
	scene_ = scene;
}

ns::Renderer3dCreateInfo& ns::Renderer3d::settings()
{
	return info_;
}

void ns::Renderer3d::importFromYAML(const std::string filename)
{
	YAML::Node config;
	try {
		config = YAML::LoadFile(filename);
	}
	catch (...) { std::cerr << "failed to import renderer settings !\n"; return; }

	try {
		auto clear = config["renderer"]["clearColor"].as<glm::vec4>();
		glClearColor(clear.r, clear.g, clear.b, clear.a);

		info_.bloomIteration = config["renderer"]["bloomIteration"].as<int>();
		info_.FXAA = config["renderer"]["enableFXAA"].as<bool>();
	}
	catch (...) {
		return;
	}
}

void ns::Renderer3d::exportIntoYAML(const std::string filename)
{
	YAML::Node conf;

	conf["renderer"]["clearColor"] = ns::getClearColor();
	conf["renderer"]["bloomIteration"] = info_.bloomIteration;
	conf["renderer"]["enableFXAA"] = info_.FXAA;

	std::ofstream file(filename, std::ios::app);
	file << "\n\n";
	file << conf;
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
	glDeleteBuffers(1, &planeBuffer_);
	glDeleteVertexArrays(1, &plane_);

	PostProcessingLayerInfo info;
	info.samples = 2;
	info.ComputeShaderNumWorkGroupX = 32;
	info.ComputeShaderNumWorkGroupY = 32;

	postProcess = std::make_unique<ns::PostProcessingLayer>(win_, info, NS_PATH"assets/shaders/compute/postProcess.comp");
	gaussianBlur = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/compute/gaussianBlur.comp");
}

void ns::Renderer3d::launchTickThread()
{
	using namespace std;
	guardian_ = make_shared<recursive_mutex>();
	tickThread_ = make_shared<thread>(&tickThread, this);
	tickThread_->detach();
}

void ns::Renderer3d::tickThread(ns::Renderer3d* renderer)
{
	using namespace std;
	using namespace std::chrono;
	using namespace std::chrono_literals;
	constexpr auto delta = duration_cast<nanoseconds>(2ms);

	while (renderer->runTicks){
		const auto time = high_resolution_clock::now();
		try {
			renderer->guardian_->lock();
		}
		catch (std::system_error) {
			Debug::get() << "remaking the recursive_mutex !\n";
			renderer->guardian_ = std::make_shared<std::recursive_mutex>();
			continue;
		}

		renderer->scene_.update();

		renderer->guardian_->unlock();

		this_thread::sleep_for(delta - (time - high_resolution_clock::now()));
	}
}

void ns::Renderer3d::setDynamicUniforms() const
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

	pbr_->set("projView", cam_.projectionView());
	pbr_->set("model", glm::scale(glm::vec3(1)));
	pbr_->set("camPos", cam_.position());

	pbr_->set<int>("dirLightNumber", DirectionalLight::number());
	pbr_->set<int>("pointLightNumber", PointLight::number());
	pbr_->set<int>("spotLightNumber", SpotLight::number());
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