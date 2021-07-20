#pragma once

#include "Shader.h"
#include <memory>



namespace ns{
	struct PostProcessingLayerInfo {
		PostProcessingLayerInfo() :
			ComputeShaderNumWorkGroupX(16),
			ComputeShaderNumWorkGroupY(16),
			getDepthMap(false),
			samples(2)
		{}

		unsigned ComputeShaderNumWorkGroupX;
		unsigned ComputeShaderNumWorkGroupY;
		bool getDepthMap;							//depth will be send in the compute shader as a uniform called : depthMap
		uint8_t samples;
	};

	class PostProcessingLayer
	{
	public:
		PostProcessingLayer(const Window& window, const PostProcessingLayerInfo& info, const std::string& computeShaderFilepath, const std::vector<ns::Shader::Define>& defines = {});
		~PostProcessingLayer();

		void setComputeShader(const std::string& computeShaderFilepath, const std::vector<ns::Shader::Define>& defines = {});

		/**
		 * @brief bind the framebuffer in the aim of applying the post-processing compute shader after it
		 */
		void bind() const;
		/**
		 * @brief start the computing in the GPU but do not wait for it
		 */
		void startProcessing();
		/**
		 * @brief wait for the computing to finish and then send the result texture id
		 * \return 
		 */
		int result() const;
		/**
		 * @brief use result() to draw it on a quad
		 */
		void draw() const;
		unsigned colorMap() const;
		unsigned colorMap2() const;
		unsigned depthMap() const;

		const Shader& computeShader() const;

		static void createScreen();
		static void deleteScreen();

	protected:
		const Window& win_;
		const PostProcessingLayerInfo info_;

		std::unique_ptr<Shader> computeShader_;
		unsigned resultTexture_;
		
		unsigned framebuffer_;

		unsigned colorAttachement_;
		unsigned secondColorAttachement_;
		unsigned depthAttachement_;

	protected:
		static struct Quad {
			unsigned VAO;
			unsigned VBO;

			bool isCreated;

			std::unique_ptr<Shader> shader;

			Quad() : VAO(0), VBO(0), isCreated(false) {}
		} screen;
	};
}
