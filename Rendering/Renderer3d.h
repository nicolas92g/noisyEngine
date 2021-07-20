#pragma once

//ns
#include "Window.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Object3d.h"
#include "Scene.h"
#include "SkyMapRenderer.h"
#include "PostProcessingLayer.h"
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
			bloomIteration = 5;
		}

		unsigned int directionalLightsMax;
		unsigned int pointLightsMax;
		unsigned int spotLightsMax;
		std::string environmentMap;
		bool FXAA;
		int bloomIteration;
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

		void importFromYAML(const std::string filename);
		void exportIntoYAML(const std::string filename);

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
		unsigned int cube_;
		unsigned int cubeBuffer_;
		void createCube();

		//plane VAO
		unsigned int plane_;
		unsigned int planeBuffer_;
		void createPlaneMesh();

		void initPhysicallyBasedRenderingSystem(const std::string& envHdrMapPath);

		//rendering system
		std::unique_ptr<ns::Shader> pbr_;
		Camera& cam_;
		Window& win_;
		Scene& scene_;

		std::shared_ptr<std::thread> tickThread_;
		std::shared_ptr<std::recursive_mutex> guardian_;
		void launchTickThread();
		static void tickThread(ns::Renderer3d* renderer);
		bool runTicks;

		void setDynamicUniforms() const;

		//post postprocessing
		std::unique_ptr<PostProcessingLayer> postProcess;
		std::unique_ptr<Shader> gaussianBlur;


		SkyMapRenderer skyBox;

		friend class Debug;
	};
}
