#pragma once

//ns
#include "Window.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Object3d.h"
#include "Scene.h"

//stl
#include <memory>
#include <thread>
#include <mutex>

namespace ns {
	class Renderer3d
	{
	public:
		Renderer3d(Window& window, Camera& camera, Scene& scene);
		~Renderer3d();

		void draw();
		void updateStationaries();

		void setCamera(Camera& camera);
		void setScene(Scene& scene);

	protected:
		//physically based rendering system
		unsigned int environmentMap_;
		unsigned int preFilteredEnvironmentMap_;
		unsigned int irradianceMap_;
		unsigned int brdfMap_;

		void loadEnvironmentMap(const char* path, int res);
		void updateBRDFpreComputing();
		void updateIrradianceMap();
		void updatePreFilteredEnvironmentMap();
		void sendFixDataToShader() const;

		unsigned int cube_;
		unsigned int cubeBuffer_;
		void createCube();

		unsigned int plane_;
		unsigned int planeBuffer_;
		void createPlaneMesh();

		void initPhysicallyBasedRenderingSystem();

		//rendering system
		std::shared_ptr<ns::Shader> pbr_;
		Camera& cam_;
		Window& win_;
		Scene& scene_;

		std::shared_ptr<std::thread> tickThread_;
		std::shared_ptr<std::recursive_mutex> guardian_;
		void launchTickThread();
		static void tickThread(ns::Renderer3d* renderer);
		bool runTicks;

		void setDynamicUniforms() const;
	};
}
