#pragma once

//ns
#include "Window.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Object3d.h"
#include "Scene.h"
#include "SkyMapRenderer.h"
#include <configNoisy.hpp>

//stl
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>

namespace ns {

	struct Renderer3dCreateInfo {
		Renderer3dCreateInfo(const std::string& envHdrMapPath = NS_PATH"assets/textures/HDR_029_Sky_Cloudy_Ref.hdr") {
			directionalLightsMax = 5;
			pointLightsMax = 100;
			spotLightsMax = 50;
			environmentMap = envHdrMapPath;
			FXAA = true;
			bloomIteration = 3;
			showNormals = false;
			renderSkybox = false;
			shadowPrecision = 1000;
			shadowSize = 100;
		}

		unsigned int directionalLightsMax;
		unsigned int pointLightsMax;
		unsigned int spotLightsMax;
		std::string environmentMap;
		bool FXAA;
		int bloomIteration;
		float bloomThreshold;
		bool showNormals;
		bool renderSkybox;
		int shadowPrecision;
		int shadowSize;
	};

	class Renderer3d
	{
	public:
		Renderer3d(Window& window, Camera& camera, Scene& scene, const Renderer3dCreateInfo& info = Renderer3dCreateInfo());
		~Renderer3d();

		void startRendering();
		void finishRendering();

		void updateStationaries();

		void setCamera(Camera& camera);
		void setScene(Scene& scene);
		Renderer3dCreateInfo& settings();

		void importFromYAML();
		void exportIntoYAML();

	protected:
		Renderer3dCreateInfo info_;

		//physically based rendering system
		void draw();

		unsigned int environmentMap_;
		unsigned int preFilteredEnvironmentMap_;
		unsigned int irradianceMap_;
		unsigned int brdfMap_;

		void loadEnvironmentMap(const char* path, int res);
		void updateBRDFpreComputing();
		void updateIrradianceMap();
		void updatePreFilteredEnvironmentMap();
		void sendFixDataToShader() const;

		//cube VAO
		GLuint cube_;
		GLuint cubeBuffer_;
		void createCube();

		//plane VAO
		GLuint plane_;
		GLuint planeBuffer_;
		void createPlaneMesh();

		void initPhysicallyBasedRenderingSystem(const std::string& envHdrMapPath);

		//rendering system
		std::unique_ptr<ns::Shader> pbr_;
		Camera& cam_;
		Window& win_;
		Scene& scene_;
		glm::ivec2 previousResolution_;

		std::shared_ptr<std::thread> tickThread_;
		std::mutex sceneMutex_;
		void launchTickThread();
		void tickThreadFunction();
		bool runTicks;

		void setDynamicUniforms(ns::Shader& shader) const;

		//shadows
		uint16_t shadowMapRes_;
		GLuint shadowMap_;
		GLuint shadowFramebuffer_;
		glm::mat4 lightMatrix_;
		std::unique_ptr<ns::Shader> shadowShader_;

		void initShadowPipeline();
		void updateDynamicShadow(const glm::vec3& position, const glm::vec3& lightDir);

		//post postprocessing
		std::unique_ptr<ns::Shader> screenShader_;
		std::unique_ptr<ns::Shader> bloomPrefilteringStage_;
		std::unique_ptr<ns::Shader> bloomDownsamplingStage_;
		std::unique_ptr<ns::Shader> bloomUpsamplingStage_;
		std::unique_ptr<ns::Shader> FinalPostProcessingStage_;
		

		struct Image2d {
			GLuint id;
			glm::ivec2 size;
		};

		struct DoubleImage {
			GLuint id;
			GLuint id2;
			glm::ivec2 size;
		};

		Image2d bloomThresholdFiltered_;		//prefiltering and downsampled
		std::vector<DoubleImage> bloomDownsampled_;	//all the downsampled textures
		std::vector<Image2d> bloomUpsampled_;	//all the upsampled textures
		GLuint result_;							//texture with the window resolution
		int previousBloomIterationSetting_;

		void initBloomPipeline();
		void resizeBloomPipeline();
		void destroyBloomPipeline();

		//custom framebuffer
		GLuint framebuffer_;
		GLuint colorAttachement_;
		GLuint depthAttachement_;

		void createFramebuffer();


		SkyMapRenderer skyBox;

		friend class Debug;
#		ifndef NDEBUG
		std::unique_ptr<ns::Shader> normalVisualizer_;
#		endif
	};
}
