#pragma once
#include <Rendering/Scene.h>
#include <Rendering/Window.h>
#include <Rendering/Camera.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <Utils/Timer.h>

namespace ns {
	class Debug : public std::stringstream
	{
	public:
		static void create(Window& win, Camera& cam);
		void destroy();
		
		void setCamera(Camera& cam);
		void setWindow(Window& win);
		void setScenes(const std::vector<ns::Scene*>& scenes);
		void addScene(Scene& scene);
		void removeScene(Scene& scene);
		void clearScenes();


		void render();
		static Debug& get();

#		ifndef NDEBUG
		static size_t memoryHeapAllocated;
		static size_t memoryHeapFreed;
#		endif
	protected:
		Debug(Window& win, Camera& cam);
		performanceBench bench;

		static Debug* debug;
		std::stringstream couts_;

		Window& win_;
		Camera& cam_;
		std::vector<ns::Scene*> scenes_;

		int shellMaxChars_;
		int freeId;
		const char* createNewFreeId();
	protected:
		void fpsMenu();
		void windowMenu();
		void ShellMenu();
		void cameraMenu();
		void heapMenu();
		void scenesMenu();
		
		void shell();

		void inputObject3d(Object3d& obj);
		void inputDirectionalObject3d(DirectionalObject3d& obj);
		void input
	};
}


