#include "DebugLayer.h"

#include <vector>
#include <deque>

#ifndef NDEBUG
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void* operator new(size_t size) {
	ns::Debug::memoryHeapAllocated += size;

	void* const ptr = malloc(size);
	assert(ptr);
	return ptr;
}

void operator delete(void* ptr, size_t size) {
	ns::Debug::memoryHeapFreed += size;
	return free(ptr);
}

size_t ns::Debug::memoryHeapAllocated = 0;
size_t ns::Debug::memoryHeapFreed = 0;

#endif

ns::Debug* ns::Debug::debug;

void ns::Debug::render()
{
#	ifndef NDEBUG
	using namespace ImGui;
	freeId = 0;

	//ShowDemoWindow();

	//open the debug layer
	Begin("debug layer");
	fpsMenu();
	Separator();

	windowMenu();
	ShellMenu();
	cameraMenu();
	scenesMenu();
	End();

	//open the shell
	shell();

	//finally render imgui stuff 
	Render();
	debug->bench.recordFrame();
#	endif // !NDEBUG
}

ns::Debug& ns::Debug::get()
{
#	ifndef NDEBUG
	assert(debug);
#	endif
	return *debug;
}

ns::Debug::Debug(Window& win, Camera& cam)
	:
	win_(win),
	cam_(cam),
	freeId(0),
	shellMaxChars_(200)
{
	str("Hello in the Noisy Engine !\n----------\n");
}

void ns::Debug::create(Window& win, Camera& cam)
{
	debug = new Debug(win, cam);

	Debug::get() << glGetString(GL_VENDOR) << ' ' << glGetString(GL_RENDERER) << '\n'
		<< "OpenGL version : " << glGetString(GL_VERSION) << "\n----\n";
}

void ns::Debug::destroy()
{
	delete Debug::debug;
}

void ns::Debug::setCamera(Camera& cam)
{
	debug->cam_ = cam;
}

void ns::Debug::setWindow(Window& win)
{
	debug->win_ = win;
}

void ns::Debug::setScenes(const std::vector<ns::Scene*>& scenes)
{
	scenes_ = scenes;
}

void ns::Debug::addScene(Scene& scene)
{
	scenes_.push_back(&scene);
}

void ns::Debug::removeScene(Scene& scene)
{
	for (auto it = scenes_.begin(); it != scenes_.end(); ++it) {
		if (*it == &scene) {
			scenes_.erase(it);
		}
	}
}

void ns::Debug::clearScenes()
{
	scenes_.clear();
}

void ns::Debug::fpsMenu()
{
	using namespace ImGui;
	static bool slowFps = true;
	static size_t i_ = 0;
	i_++;
	static uint32_t fps;
	fps = (!(i_ % 30) or !slowFps) ? win_.framerate() : fps;

	Text((std::to_string(fps) + " fps   ").c_str()); SameLine();
	
	Checkbox("slow fps     ", &slowFps); 

	if (win_.monitor()) {
		SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(.9f, .4f, .4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(1.f, .2f, .2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(.8f, .4f, .4f));
		
		if (Button("close application")) win_.setShouldClose(true);
		ImGui::PopStyleColor(3);
	}

	static std::vector<float> fpss;

	if (fpss.size() < 200) {
		fpss.push_back(fps);
	}
	else {
		fpss.erase(fpss.begin());
	}

	uint32_t max = 0;
	uint32_t min = UINT32_MAX;
	for (const uint32_t value : fpss) {
		max = std::max(max, value);
		min = std::min(min, value);
	}
		

	ImGui::PlotLines(("fps (max = " + std::to_string(max) + ")").c_str(), fpss.data(), fpss.size(), 10, "", (float)min * .2, (float)max, ImVec2(0, 40.0f));
}

void ns::Debug::heapMenu()
{
	using namespace ImGui;
	if (CollapsingHeader("heap allocation")) {
		
	}
}

void ns::Debug::cameraMenu()
{
	using namespace ImGui;
	if (CollapsingHeader("camera")) {

		inputDirectionalObject3d(cam_);
		

		
		float y = cam_.yaw();

		while (y > PI)
			y -= 2 * PI;
		while (y < -PI)
			y += 2 * PI;

		cam_.setYaw(y);

		Text("yaw :   "); SameLine();
		SliderFloat("##yaw", &y, -PI, PI);
		bool wasEdited = y != cam_.yaw();
		
		float p = cam_.pitch();
		Text("pitch : "); SameLine();
		SliderFloat("##pitch", &p, -PI * .5f, PI * .5f);
		if (p != cam_.pitch()) wasEdited = true;
		
		if (wasEdited) {
			cam_.setPitch(p);
			cam_.setYaw(y);
			cam_.updateLookWithYawAndPitch();
		}
	}
}

void ns::Debug::ShellMenu()
{
	using namespace ImGui;
	if (CollapsingHeader("shell settings")) {
		SliderInt(" chars", &shellMaxChars_, 100, 1500);
	}
}

void ns::Debug::shell()
{
	using namespace ImGui;
	Begin("shell");

	static std::string buf;
	buf = str();

	if (buf.size() > shellMaxChars_) {
		buf.erase(0, static_cast<size_t>(shellMaxChars_) - 1);
		str(buf);
	}

	//invert lines
	//std::vector<std::string> lines(2);
	//
	//for (size_t i = 0; i < buf.size(); i++)
	//{
	//	if (buf[i] == '\n') {
	//		lines.push_back("");
	//		continue;
	//	}
	//	lines[lines.size() - 1] += buf[i];
	//}
	//
	//for (size_t i = lines.size() - 1; i > 0; i--)
	//{
	//	Text(lines[i].c_str());
	//}

	Text(buf.c_str());
	End();
}

void ns::Debug::windowMenu()
{
	using namespace ImGui;
	if (CollapsingHeader("window")) {

		Text("resolution : "); SameLine();
		glm::ivec2 res = win_.size();
		SliderInt2(" px##resolution", &res.x, 0, 5000);
		win_.setSize(res.x, res.y);

		Text("position : "); SameLine();
		glm::ivec2 pos = win_.position();
		SliderInt2(" px##position", &pos.x, 0, 5000);
		win_.setPosition(pos.x, pos.y);


		if (win_.monitor() and TreeNode("window's monitor")) {
			int width, height;
			glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &width, &height);
			Text(("size = " + std::to_string(width) + " mm / " + (std::to_string(height) + " mm")).c_str());

			int x, y;
			glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), nullptr, nullptr, &x, &y);
			Text(("usable resolution = " + std::to_string(x) + " px / " + (std::to_string(y) + " px")).c_str());
			TreePop();
		}

	}
}

void ns::Debug::scenesMenu()
{
	using namespace ImGui;
	if (CollapsingHeader("Scenes")) {

		for (size_t i = 0; i < scenes_.size(); ++i) {
			if (TreeNode(("scene " + std::to_string(i + 1)).c_str())) {

				if (TreeNode(("entities ##" + std::to_string(i + 1)).c_str())) {
					Text("coucou");



					TreePop();
				}

				if (TreeNode(("stationaries ##" + std::to_string(i + 1)).c_str())) {
					Text("coucou");



					TreePop();
				}

				if (TreeNode(("lights ##" + std::to_string(i + 1)).c_str())) {
					Text("coucou");



					TreePop();
				}


				TreePop();
			}
		}

	}
}

const char* ns::Debug::createNewFreeId()
{
	static char buf[200];
	sprintf_s(buf, "##%d", freeId);
	freeId++;
	return buf;
}

void ns::Debug::inputObject3d(Object3d& obj)
{
	glm::vec3 vec = obj.position();

	ImGui::Text("position : ");
	ImGui::SameLine();
	ImGui::InputFloat3(createNewFreeId(), &vec.x);
	obj.setPosition(vec);
}

void ns::Debug::inputDirectionalObject3d(DirectionalObject3d& obj)
{
	inputObject3d(obj);
	glm::vec3 dir = obj.direction();

	ImGui::Text("direction : ");
	ImGui::SameLine();
	ImGui::SliderFloat3(createNewFreeId(), &dir.x, -1.f, 1.f);
	obj.setDirection(dir);
}
