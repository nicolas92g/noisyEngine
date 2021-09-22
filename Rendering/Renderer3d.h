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
	/**
	 * @brief 3D renderer configuration
	 */
	struct Renderer3dConfigInfo {
		Renderer3dConfigInfo(const std::string& envHdrMapPath = NS_PATH"assets/textures/HDR_029_Sky_Cloudy_Ref.hdr") {
			directionalLightsMax = 5;
			pointLightsMax = 100;
			spotLightsMax = 50;
			environmentMap = envHdrMapPath;
			FXAA = true;
			bloomIteration = 3;
			showNormals = false;
			renderSkybox = false;
			shadows = true;
			shadowPrecision = 1000;
			shadowSize = 100;
			exposure = 1.f;
			ambientIntensity = 1.f;
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
		bool shadows;
		int shadowPrecision;
		int shadowSize;
		float exposure;
		float ambientIntensity;
	};
	/**
	 * @brief allow to render a scene with a pbr simulation.
	 * this renderer implement :
	 * - pbr material system
	 * - normal mapping
	 * - directional light shadow mapping
	 * - hdr pipeline & bloom
	 * - Fast approximate Anti-Aliasing
	 * - exposure tone-mapping
	 */
	class Renderer3d
	{
	public:
		/**
		 * @brief create the renderer with the object needed by it, (those object have to survive all along the renderer life)
		 * this constructor take also the configuration
		 * \param window
		 * \param camera
		 * \param scene
		 * \param info
		 */
		Renderer3d(Window& window, Camera& camera, Scene& scene, const Renderer3dConfigInfo& info = Renderer3dConfigInfo());
		/**
		 * @brief delete all the object allocated by the renderer on the GPU and the CPU
		 */
		~Renderer3d();
		/**
		 * @brief render the rasterization on a custom framebuffer
		 * rendering is separated in 2 methods to allow to render other things on this custom FBO
		 */
		void startRendering();
		/**
		 * @brief finish the rendering by doing the post-processing (bloom, FXAA, etc...)
		 */
		void finishRendering();
		/**
		 * @brief change the camera of the renderer
		 * \param camera
		 */
		void setCamera(Camera& camera);
		/**
		 * @brief change the scene container
		 * \param scene
		 */
		void setScene(Scene& scene);
		/**
		 * @brief get the settings of this renderer
		 * \return 
		 */
		Renderer3dConfigInfo& settings();
		/**
		 * @brief import the renderer settings from the configuration file
		 */
		void importFromYAML();
		/**
		 * @brief export the renderer settings into the configuration file
		 */
		void exportIntoYAML();

	protected:
		Renderer3dConfigInfo info_;

		//physically based rendering system
		void draw();

		GLuint environmentMap_;
		GLuint preFilteredEnvironmentMap_;
		GLuint irradianceMap_;
		GLuint brdfMap_;

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

		//rendering system
		void initPhysicallyBasedRenderingSystem(const std::string& envHdrMapPath);
		std::unique_ptr<ns::Shader> pbr_;
		Camera& cam_;
		Window& win_;
		const Scene* scene_;
		glm::ivec2 previousResolution_;

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
		Texture dirtMask_;

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
