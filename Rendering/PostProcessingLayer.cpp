#include "PostProcessingLayer.h"
#include "configNoisy.hpp"
#include <Utils/DebugLayer.h>

ns::PostProcessingLayer::Quad ns::PostProcessingLayer::screen;

ns::PostProcessingLayer::PostProcessingLayer(const Window& window, const PostProcessingLayerInfo& info, const std::string& computeShaderFilepath, const std::vector<ns::Shader::Define>& defines)
	:
	resultTexture_(0U),
	win_(window),
	info_(info),
	secondColorAttachement_(0),
	depthAttachement_(0)
{
	setComputeShader(computeShaderFilepath, defines);
	
	//create a quad to render textures if this func has never been called before
	ns::PostProcessingLayer::createScreen();

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

	//attach it to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachement_, 0);

	//main color attachement
	glGenTextures(1, &secondColorAttachement_);
	glBindTexture(GL_TEXTURE_2D, secondColorAttachement_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.size().x, win_.size().y, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//attach it to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondColorAttachement_, 0);

	//add a draw buffer
	unsigned attachements[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachements);
	

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

	//create compute result texture
	glGenTextures(1, &resultTexture_);
	glBindTexture(GL_TEXTURE_2D, resultTexture_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 100, 100, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

ns::PostProcessingLayer::~PostProcessingLayer()
{
	glDeleteFramebuffers(1, &framebuffer_);
	glDeleteTextures(1, &colorAttachement_);
	glDeleteTextures(1, &depthAttachement_);

	glDeleteTextures(1, &resultTexture_);
}

int ns::PostProcessingLayer::result() const
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	return resultTexture_;
}

void ns::PostProcessingLayer::bind() const
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glBlitFramebuffer(0, 0, win_.size().x, win_.size().y, 0, 0, win_.size().x, win_.size().y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	
	//clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//color attachement
	glBindTexture(GL_TEXTURE_2D, colorAttachement_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.size().x, win_.size().y, 0, GL_RGBA, GL_FLOAT, NULL);

	//depth attachement
	glBindTexture(GL_TEXTURE_2D, depthAttachement_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, win_.size().x, win_.size().y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//second color attachement
	glBindTexture(GL_TEXTURE_2D, secondColorAttachement_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.size().x, win_.size().y, 0, GL_RGBA, GL_FLOAT, NULL);
}

void ns::PostProcessingLayer::startProcessing()
{
	const int X = info_.ComputeShaderNumWorkGroupX, Y = info_.ComputeShaderNumWorkGroupY;
	const int resx = win_.size().x + X - win_.size().x % X, resy = win_.size().y + Y - win_.size().y % Y;

	computeShader_->use();
	computeShader_->set("viewportExtent", win_.size());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, colorAttachement_);
	computeShader_->set("colorMap", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthAttachement_);
	computeShader_->set("depthMap", 2);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, secondColorAttachement_);
	computeShader_->set("colorMap2", 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, resultTexture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_.size().x, win_.size().y, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, resultTexture_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	glDispatchCompute(resx / X, resy / Y, 1);
}

void ns::PostProcessingLayer::draw() const
{
	glBindVertexArray(screen.VAO);
	screen.shader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, result());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
}

unsigned ns::PostProcessingLayer::depthMap() const
{
	return depthAttachement_;
}

unsigned ns::PostProcessingLayer::colorMap() const
{
	return colorAttachement_;
}

unsigned ns::PostProcessingLayer::colorMap2() const
{
	return secondColorAttachement_;
}

void ns::PostProcessingLayer::deleteScreen() {
	glDeleteBuffers(1, &screen.VBO);
	glDeleteVertexArrays(1, &screen.VAO);
	screen.shader.reset();
}

void ns::PostProcessingLayer::setComputeShader(const std::string& computeShaderFilepath, const std::vector<ns::Shader::Define>& defines)
{
	//set somes defines in the computes shader
	std::vector<ns::Shader::Define> copy(defines);
	copy.push_back({ "NS_NUM_WORK_GROUP_X", std::to_string(info_.ComputeShaderNumWorkGroupX), Shader::Stage::Compute });
	copy.push_back({ "NS_NUM_WORK_GROUP_Y", std::to_string(info_.ComputeShaderNumWorkGroupY), Shader::Stage::Compute });

	computeShader_ = std::make_unique<ns::Shader>(computeShaderFilepath.c_str(), copy, false);
}

void ns::PostProcessingLayer::createScreen()
{
	if (screen.isCreated) return;

	//create screen vertex buffer
	const float vertices[8] = {
		 1, 1,
		-1, 1,
		 1,-1,
		-1,-1
	};

	glGenVertexArrays(1, &screen.VAO);
	glBindVertexArray(screen.VAO);

	glGenBuffers(1, &screen.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, screen.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);

	//create screen shader
	screen.shader = std::make_unique<ns::Shader>(NS_PATH"assets/shaders/main/screen.vert", NS_PATH"assets/shaders/main/screen.frag");

	screen.isCreated = true;
}