#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace ns {
	class Window
	{
	public:
		Window(uint32_t width = 800, 
			uint32_t height = 600, 
			const char* title = "Opengl 4.3 window", 
			int samplesCount = 4,
			bool transparentFramebuffer = false);

		Window(Window&) = delete;

		~Window();

		const int width() const;
		const int height() const;
		bool shouldNotClose() const;
		bool key(int GLFW_KEY) const;
		glm::vec<2, double> getCursorPos() const;
		glm::ivec2 position() const;
		glm::ivec2 size() const;
		bool isFocused() const;
		GLFWmonitor* monitor() const;

		void setWidth(const int width);
		void setHeight(const int height);
		void setSize(const int width, const int height);
		void setShouldClose(bool newValue);
		void setCursorPos(double x, double y);
		void hideCursor();
		void showCursor();
		void setTitle(const char* newTitle);
		void maximise();
		void setFullscreen(bool fullscreen);
		void setPosition(int x, int y);

		void beginFrame();
		void endFrame();
		void swapBuffers() const;

		void inputFullscreen(int GLFW_KEY);
		void recordFrameTiming();
		double deltaTime() const;
		uint32_t framerate() const;
	protected:
		GLFWwindow* window_;
		int width_;
		int height_;
		double deltaTime_ = .16;
		uint32_t fps_ = 60;
		bool fullscreen_;
		bool fullscreenKeyState_;

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
	};
};
