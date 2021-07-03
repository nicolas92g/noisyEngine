#include <glad/glad.h>

#include "Window.h"

#include <iostream>
#include <chrono>

#ifndef NDEBUG
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif

ns::Window::Window(uint32_t width, uint32_t height, const char* title, int samplesCount, bool transparentFB)
    : 
    width_(width), 
    height_(height),    
    fullscreen_(false),
    fullscreenKeyState_(false)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, samplesCount);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, transparentFB);

	window_ = glfwCreateWindow(
		static_cast<int>(width), static_cast<int>(height),
		title, nullptr, nullptr);

	if (!window_) {
        std::cerr << "failed to create a window with glfw !\n";
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window_);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
        std::cerr << "Failed to initialize GLAD" << std::endl;
		exit(EXIT_FAILURE);
	}

	glViewport(0, 0, width, height);

	glfwSetWindowUserPointer(window_, this);

	glfwSetFramebufferSizeCallback(window_, &framebufferResizeCallback);

    glfwSwapInterval(0);

	//debug callbacks
#	ifndef NDEBUG

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
    else {
        std::cerr << "info : not able to use opengl debug mode !\n";
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init();

#	endif //NDEBUG
}

ns::Window::~Window()
{
#	ifndef NDEBUG
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#   endif

	glfwTerminate();
}

const int ns::Window::width() const
{
	return width_;
}

const int ns::Window::height() const
{
	return height_;
}

bool ns::Window::shouldNotClose() const
{
    return !glfwWindowShouldClose(window_);
}

bool ns::Window::key(int GLFW_KEY) const
{
    return glfwGetKey(window_, GLFW_KEY);
}

glm::vec<2, double> ns::Window::getCursorPos() const
{
    double x, y;
    glfwGetCursorPos(window_, &x, &y);
    return {x, y};
}

glm::ivec2 ns::Window::position() const
{
    glm::ivec2 ret;
    glfwGetWindowPos(window_, &ret.x, &ret.y);
    return ret;
}

glm::ivec2 ns::Window::size() const
{
    glm::ivec2 ret;
    glfwGetWindowSize(window_, &ret.x, &ret.y);
    return ret;
}

bool ns::Window::isFocused() const
{
    return glfwGetWindowAttrib(window_, GLFW_FOCUSED);
}

GLFWmonitor* ns::Window::monitor() const
{
    return glfwGetWindowMonitor(window_);
}

void ns::Window::setWidth(const int width)
{
    glfwSetWindowSize(window_, width, height_);
}

void ns::Window::setHeight(const int height)
{
    glfwSetWindowSize(window_, width_, height);
}

void ns::Window::setSize(const int width, const int height)
{
    glfwSetWindowSize(window_, width, height);
}

void ns::Window::setShouldClose(bool newValue)
{
    glfwSetWindowShouldClose(window_, newValue);
}

void ns::Window::setCursorPos(double x, double y)
{
    glfwSetCursorPos(window_, x, y);
}

void ns::Window::hideCursor()
{
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#   ifndef NDEBUG
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
#   endif
}

void ns::Window::showCursor()
{
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#   ifndef NDEBUG
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
#   endif
}

void ns::Window::setTitle(const char* newTitle)
{
    if(!fullscreen_) //renaming window while beeing fulscreen is laggy for some reason
        glfwSetWindowTitle(window_, newTitle);
}

void ns::Window::maximise()
{
    glfwMaximizeWindow(window_);
}

void ns::Window::setFullscreen(bool fullscreen)
{
    if (fullscreen and !fullscreen_) {
        fullscreen_ = fullscreen;
        GLFWmonitor* monitorToUse = glfwGetPrimaryMonitor();
        int x, y;
        glfwGetMonitorWorkarea(monitorToUse, &x, &y, &width_, &height_);

        glfwSetWindowMonitor(window_, monitorToUse, x, y, width_, height_, GLFW_DONT_CARE);
    }

    if (!fullscreen and fullscreen_){
        fullscreen_ = fullscreen;
        glfwSetWindowMonitor(window_, nullptr, 100, 100, 800, 600, GLFW_DONT_CARE);
        glfwRestoreWindow(window_);
        maximise();

    }
}

void ns::Window::setPosition(int x, int y)
{
    glfwSetWindowPos(window_, x, y);
}

void ns::Window::beginFrame()
{
#   ifndef NDEBUG
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
#   endif
}

void ns::Window::endFrame()
{
    glfwPollEvents();
#   ifndef NDEBUG
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#   endif
    swapBuffers();
    recordFrameTiming();
}

void ns::Window::swapBuffers() const
{
    glfwSwapBuffers(window_);
}

void ns::Window::inputFullscreen(int GLFW_KEY)
{
    if (key(GLFW_KEY_F11) and !fullscreenKeyState_) {
        setFullscreen(!fullscreen_);
    }
    fullscreenKeyState_ = key(GLFW_KEY_F11);
}

void ns::Window::recordFrameTiming()
{
    using namespace std::chrono;
    static auto previousTime = high_resolution_clock::now();

    auto deltaTimeDuration = duration_cast<nanoseconds>(high_resolution_clock::now() - previousTime);

    deltaTime_ = (double)deltaTimeDuration.count() * 1e-9;
    fps_ = 1.0 / deltaTime_;

    previousTime = high_resolution_clock::now();
}

double ns::Window::deltaTime() const
{
    return deltaTime_;
}

uint32_t ns::Window::framerate() const
{
    return fps_;
}

void ns::Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	ns::Window* ptr_ = reinterpret_cast<ns::Window*>(glfwGetWindowUserPointer(window));
	ptr_->width_ = width;
	ptr_->height_ = height;
}

void APIENTRY ns::Window::glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cerr << "---------------" << std::endl;
    std::cerr << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
    } std::cerr << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
    } std::cerr << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
    } std::cerr << std::endl;
    std::cerr << std::endl;
}
