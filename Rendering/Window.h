#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <configNoisy.hpp>
#include <string>

namespace ns {
	/**
	 * @brief allow to create a window with an opengl context
	 */
	class Window
	{
	public:
		/**
		 * @brief create a window
		 * \param width
		 * \param height
		 * \param title
		 * \param samplesCount
		 * \param transparentFramebuffer
		 */
		Window(uint32_t width = 800, 
			uint32_t height = 600, 
			const char* title = "Opengl 4.3 window",
			int samplesCount = 1,
			bool transparentFramebuffer = false);

		Window(Window&) = delete;
		/**
		 * @brief destroy the opengl context and the window
		 */
		~Window();
		/**
		 * @brief return the width of the window
		 * \return 
		 */
		const int width() const;
		/**
		 * @brief return the height of the window
		 * \return
		 */
		const int height() const;
		/**
		 * @brief return false if the window need to be closed (usually when alt-F4 is pressed or the red cross is clicked)
		 * \return 
		 */
		bool shouldNotClose() const;
		/**
		 * @brief allow to know if a key is pressed
		 * the list of the keys is here :
		 * https://www.glfw.org/docs/3.3/group__keys.html
		 * \param GLFW_KEY
		 * \return 
		 */
		bool key(int GLFW_KEY) const;
		/**
		 * @brief return the cursor pos relative to the window
		 * \return 
		 */
		glm::vec<2, double> getCursorPos() const;
		/**
		 * @brief return the position of the window relative to the monitors
		 * \return 
		 */
		glm::ivec2 position() const;
		/**
		 * @brief return the size of the window in pixel
		 * \return 
		 */
		glm::ivec2 size() const;
		/**
		 * @brief allow to know if the window is focused
		 * \return 
		 */
		bool isFocused() const;
		/**
		 * @brief return a monitor if the window is fullscreened on this monitor
		 * \return 
		 */
		GLFWmonitor* monitor() const;
		/**
		 * @brief return the monitor where the window is currently located
		 * if the window is on 2 monitors at the same time, the monitor which has the biggest area of the
		 * window will be chose
		 * \return 
		 */
		GLFWmonitor* getUsedMonitor() const;
		/**
		 * @brief edit the width of the window
		 * \param width
		 */
		void setWidth(const int width);
		/**
		 * @brief edit the height of the window
		 * \param height
		 */
		void setHeight(const int height);
		/**
		 * @brief set the size of the window
		 * \param width
		 * \param height
		 */
		void setSize(const int width, const int height);
		/**
		 * @brief allow to simulate the alt-F4 effect or to remove it
		 * \param newValue
		 */
		void setShouldClose(bool newValue);
		/**
		 * @brief edit the cursor position on the window
		 * \param x
		 * \param y
		 */
		void setCursorPos(double x, double y);
		/**
		 * @brief this hide the cursor that will no longer appear when hover the window
		 */
		void hideCursor();
		/**
		 * @brief this unhide the cursor
		 */
		void showCursor();
		/**
		 * @brief change the title of the window
		 * \param newTitle
		 */
		void setTitle(const char* newTitle);
		/**
		 * @brief maximise the window
		 */
		void maximise();
		/**
		 * @brief iconify the window
		 */
		void iconify();
		/**
		 * @brief set the window in fullscreen or back in window
		 * \param fullscreen
		 */
		void setFullscreen(bool fullscreen);
		/**
		 * @brief set the position of the window relative to the monitors
		 * \param x
		 * \param y
		 */
		void setPosition(int x, int y);
		/**
		 * @brief allow to use the IMGUI debug interface and other debug purposes
		 */
		void beginFrame();
		/**
		 * @brief to call at the end of a frame !
		 * swapBuffers is called in this method
		 */
		void endFrame();
		/**
		 * @brief swap the buffers in the framebuffer
		 */
		void swapBuffers() const;
		/**
		 * @brief allow to input fullscreen with a key when called in each frame
		 * \param GLFW_KEY
		 */
		void inputFullscreen(int GLFW_KEY);
		/**
		 * @brief already called in endFrame()
		 * allow to record frame timings
		 */
		void recordFrameTiming();
		/**
		 * @brief allow to know the time that the last frame took in s
		 * \return 
		 */
		double deltaTime() const;
		/**
		 * @brief allow to know the framerate in frame per second
		 * \return 
		 */
		uint32_t framerate() const;

	protected:
		GLFWwindow* window_;
		int width_;
		int height_;
		double deltaTime_ = .16;
		uint32_t fps_ = 60;
		bool fullscreen_;
		bool fullscreenKeyState_;
		bool maximised_;

		glm::ivec4 previousWindowPosAndSize_;
		void importFromYAML();
		void exportIntoYAML();

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
	};
};
