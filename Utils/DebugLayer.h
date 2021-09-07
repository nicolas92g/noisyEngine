#pragma once

#include <Rendering/Scene.h>
#include <Rendering/Window.h>
#include <Rendering/Camera.h>
#include <Rendering/Renderer3d.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <Utils/Timer.h>
#include <Utils/utils.h>

namespace ns {
	/**
	 * @brief Debug 2d interface made with imgui
	 */
	class Debug : public std::stringstream
	{
	public:
		struct MeshDescription {
			std::vector<Vertex> vertices;
			bool indexed;
			std::vector<unsigned> indices;
		};
		
		void setCamera(Camera& cam);
		void setWindow(Window& win);
		void setRenderer3d(Renderer3d& renderer);
		void setScenes(const std::vector<ns::Scene*>& scenes);
		void addScene(Scene& scene);
		void removeScene(Scene& scene);
		void clearScenes();
		void setMeshDescription(const MeshDescription& mesh);


		void render();
		void log();
		static Debug& get();

#		ifdef USE_IMGUI
		static size_t memoryHeapAllocated;
		static size_t memoryHeapFreed;
#		endif
	protected:
		Debug();
		performanceBench bench;

		static Debug debugObject;
		std::stringstream couts_;

		Window* win_;
		Camera* cam_;
		Renderer3d* renderer_;
		MeshDescription mesh_;
		std::vector<ns::Scene*> scenes_;

		int shellMaxChars_;
		float texturesSize_;
		int freeId;
		char textureFileNameBuffer[100];
		float maxEmission_;
		const char* createNewFreeId();

		int maxTextureSize_;
	protected:
		void fpsMenu();
		void windowMenu();
		void ShellMenu();
		void cameraMenu();
		void heapMenu();
		void scenesMenu();
		void rendererMenu();
		void meshDecompositionMenu();
		
		void shell();

		void inputObject3d(Object3d& obj);
		void inputDirectionalObject3d(DirectionalObject3d& obj);
		void inputGeometricObject3d(GeometricObject3d& obj);
		void inputDrawableObject3d(DrawableObject3d& obj);

		void buttonAddTexture(std::optional<ns::TextureView>& tex, ns::Material& mat);

		void inputMaterial(Material& mat);
	};
}


