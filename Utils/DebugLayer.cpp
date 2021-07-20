#include "DebugLayer.h"

#include <vector>
#include <deque>
#include <yaml-cpp/yaml.h>
#include <Utils/utils.h>

#ifdef USE_IMGUI
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

ns::Debug ns::Debug::debugObject;

void ns::Debug::render()
{
#	ifdef USE_IMGUI
	using namespace ImGui;
	freeId = 0;

	//ShowDemoWindow();

	//open the debug layer
	Begin("debug layer");
	fpsMenu();
	Separator();

	windowMenu();
	rendererMenu();
	ShellMenu();
	heapMenu();
	cameraMenu();
	scenesMenu();
	End();

	//open the shell
	shell();

	//finally render imgui stuff 
	Render();
	debugObject.bench.recordFrame();
#else
	std::cout << str();
	str("");
#	endif // !NDEBUG
}

void ns::Debug::log()
{
	static size_t pos = 0;
	static std::string copy;
	copy = str();
	std::cout << copy.substr(pos);

	if(str().size() > 0)
		pos = str().size() - 1;
}

ns::Debug& ns::Debug::get()
{
#	ifdef USE_IMGUI
	assert(&debugObject);
#	endif
	return debugObject;
}

ns::Debug::Debug()
	:
	win_(nullptr),
	cam_(nullptr),
	freeId(0),
	shellMaxChars_(10000),
	renderer_(nullptr)
{
}

void ns::Debug::setCamera(Camera& cam)
{
	debugObject.cam_ = &cam;
}

void ns::Debug::setWindow(Window& win)
{
	debugObject.win_ = &win;
}

void ns::Debug::setRenderer3d(Renderer3d& renderer)
{
	debugObject.renderer_ = &renderer;
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
#ifdef USE_IMGUI

	using namespace ImGui;
	static bool slowFps = false;
	static size_t i_ = 0;
	i_++;
	static uint32_t fps;
	fps = (!(i_ % 30) or !slowFps) ? win_->framerate() : fps;

	Text((std::to_string(fps) + " fps   ").c_str()); SameLine();

	Checkbox("slow fps     ", &slowFps);

	SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(.9f, .4f, .4f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(1.f, .2f, .2f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(.8f, .4f, .4f));

	if (Button("close application")) win_->setShouldClose(true);
	ImGui::PopStyleColor(3);
	

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

#endif // !NDEBUG

	
}

void ns::Debug::heapMenu()
{
#ifdef USE_IMGUI 
	using namespace ImGui;
	if (CollapsingHeader("heap allocation")) {
		const size_t memoryAllocated = Debug::memoryHeapAllocated - Debug::memoryHeapFreed;

		Text(("heap allocation : " + std::to_string(memoryAllocated * 1e-6) + " MegaBytes").c_str());
		static size_t previousFrameMemoryAllocated = 0;

		static std::vector<float> allocations(100, 0);
		allocations.erase(allocations.begin());
		allocations.push_back(memoryAllocated - previousFrameMemoryAllocated);

		float max = 0.f;
		for (const float value : allocations)
			max = std::max(max, value);

		Text("memory allocated per frame :");
		ImGui::PlotLines(" bytes", allocations.data(), allocations.size(), 10, "", 0.f, max, ImVec2(0, 80.0f));
		previousFrameMemoryAllocated = memoryAllocated;
	}
#endif // !NDEBUG 
}

void ns::Debug::cameraMenu()
{
#ifdef USE_IMGUI 
	using namespace ImGui;
	if (CollapsingHeader("camera")) {

		inputDirectionalObject3d(*cam_);
		

		
		float y = cam_->yaw();

		while (y > PI)
			y -= 2 * PI;
		while (y < -PI)
			y += 2 * PI;

		cam_->setYaw(y);

		Text("yaw :   "); SameLine();
		SliderFloat("##yaw", &y, -PI, PI);
		bool wasEdited = y != cam_->yaw();
		
		float p = cam_->pitch();
		Text("pitch : "); SameLine();
		SliderFloat("##pitch", &p, -PI * .5f, PI * .5f);
		if (p != cam_->pitch()) wasEdited = true;
		
		if (wasEdited) {
			cam_->setPitch(p);
			cam_->setYaw(y);
			cam_->updateLookWithYawAndPitch();
		}

		Text("fov");
		SameLine();
		SliderFloat("##fov", &cam_->fov_, PI * .1, PI * .6);

		Text("zNear");
		SameLine();
		SliderFloat("##zNear", &cam_->zNear_, .1f, 10.f);

		static float maxZfar = 5000.0f;
		Text("zFar");
		SameLine();
		SliderFloat("##zFar", &cam_->zFar_, 10.0f, maxZfar);
		Text("max_zFar");
		SameLine();
		SliderFloat("##max_zFar", &maxZfar, 100.0f, 50000.f);
	}
#endif
}

void ns::Debug::ShellMenu()
{
#ifdef USE_IMGUI 
	using namespace ImGui;
	if (CollapsingHeader("shell settings")) {
		SliderInt(" chars", &shellMaxChars_, 100, 10000);
	}
#endif
}

void ns::Debug::shell()
{
#ifdef USE_IMGUI 
	using namespace ImGui;
	Begin("shell");

	static std::string buf;
	buf = str();

	if (buf.size() > shellMaxChars_) {
		buf.erase(0, static_cast<size_t>(shellMaxChars_) - 1);
		str(buf);
	}

	Text(buf.c_str());
	End();
#endif
}

void ns::Debug::windowMenu()
{
#ifdef USE_IMGUI 
	using namespace ImGui;
	if (CollapsingHeader("window")) {

		Text("resolution : "); SameLine();
		glm::ivec2 res = win_->size();
		SliderInt2(" px##resolution", &res.x, 0, 5000);
		win_->setSize(res.x, res.y);

		Text("position : "); SameLine();
		glm::ivec2 pos = win_->position();
		SliderInt2(" px##position", &pos.x, 0, 5000);
		win_->setPosition(pos.x, pos.y);


		if (win_->monitor() and TreeNode("window's monitor")) {
			int width, height;
			glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &width, &height);
			Text(("size = " + std::to_string(width) + " mm / " + (std::to_string(height) + " mm")).c_str());

			int x, y;
			glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), nullptr, nullptr, &x, &y);
			Text(("usable resolution = " + std::to_string(x) + " px / " + (std::to_string(y) + " px")).c_str());
			TreePop();
		}

	}
#endif
}

void ns::Debug::scenesMenu()
{
#ifdef USE_IMGUI 
	using namespace ImGui;
	if (CollapsingHeader("Scenes")) {

		for (size_t i = 0; i < scenes_.size(); ++i) {
			static std::string index;
			index = std::to_string(i + 1);

			if (TreeNode(("scene " + index).c_str())) {

				if (TreeNode(("entities ##" + index).c_str())) {
					
					for (DrawableObject3d* const obj : scenes_[i]->entities_)
					{
						Separator();
						if (TreeNode(obj->name().c_str())) {
							Text("3d values : ");
							inputDrawableObject3d(*obj);

							Model& model = *dynamic_cast<Model*>(obj);
							if (&model) {
								Text("mesh materials :");
								
								for (auto& mesh : model.meshes_) {
									BulletText(("mesh " + mesh->info_.name).c_str());
									inputMaterial(mesh->material_);
								}
									
							}
							
							
							TreePop();
						}
					}
					Separator();

					TreePop();
				}

				if (TreeNode(("stationaries ##" + index).c_str())) {
					Text("coucou");



					TreePop();
				}

				if (TreeNode(("lights ##" + index).c_str())) {
					Text("coucou");



					TreePop();
				}


				TreePop();
			}
		}

	}
#endif
}

void ns::Debug::rendererMenu()
{
#ifdef USE_IMGUI 
	if (renderer_ == nullptr) return;

	using namespace ImGui;

	if (CollapsingHeader("Renderer Settings")) {
		Separator();
		Text("enable FXAA");
		SameLine();
		Checkbox("##FXAA", &renderer_->info_.FXAA);
		Separator();
		Text("Bloom iterations");
		SameLine();
		SliderInt("##bloom it", &renderer_->info_.bloomIteration, 0, 20);
		Separator();
		static int lineWidth = 4;
		Text("line width"); 
		SameLine();
		SliderInt("##line width", &lineWidth, 1, 10);
		glLineWidth(lineWidth);
		Separator();
		static glm::vec4 clearColor = ns::getClearColor();
		Text("clear color"); SameLine();
		ColorPicker4("##clear color", &clearColor.x);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	}
#endif
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
#ifdef USE_IMGUI 
	ImGui::Text("position : ");
	ImGui::SameLine();
	ImGui::DragFloat3(createNewFreeId(), &obj.position_.x, .01f);
#endif
}

void ns::Debug::inputDirectionalObject3d(DirectionalObject3d& obj)
{
#ifdef USE_IMGUI 
	inputObject3d(obj);

	ImGui::Text("direction : ");
	ImGui::SameLine();
	ImGui::SliderFloat3(createNewFreeId(), &obj.direction_.x, -1.f, 1.f);
#endif
}

void ns::Debug::inputGeometricObject3d(GeometricObject3d& obj)
{
#ifdef USE_IMGUI 
	inputObject3d(obj);
	ImGui::Text("scale : ");
	ImGui::SameLine();
	ImGui::InputFloat3(createNewFreeId(), &obj.scale_.x);
	ImGui::Text("Rotation");
	ImGui::Text("axis : ");
	ImGui::SameLine();
	ImGui::SliderFloat3(createNewFreeId(), &obj.axis_.x, -1, 1);
	ImGui::Text("angle : ");
	ImGui::SameLine();
	ImGui::SliderFloat(createNewFreeId(), &obj.angle_, 0, 2 * PI);
#endif
}

void ns::Debug::inputDrawableObject3d(DrawableObject3d& obj)
{
#ifdef USE_IMGUI 
	inputGeometricObject3d(obj);
#endif
}

void ns::Debug::MaterialComponentInput(std::optional<ns::TextureView>& texture, float& value) {
#ifdef USE_IMGUI
	using namespace ImGui;
	if (texture.has_value()) {
		Image((void*)texture.value().textureId_, ImVec2(80, 80));
	}

#endif // !NDEBUG
}

void ns::Debug::inputMaterial(Material& mat)
{
#ifdef USE_IMGUI
	using namespace ImGui;
	BulletText(("Material " + mat.name_).c_str());

	MaterialComponentInput(mat.albedoMap_, mat.roughness_);
	MaterialComponentInput(mat.roughnessMap_, mat.roughness_);
	MaterialComponentInput(mat.metallicMap_, mat.roughness_);
	MaterialComponentInput(mat.normalMap_, mat.roughness_);
	MaterialComponentInput(mat.ambientOcclusionMap_, mat.roughness_);

#endif // !NDEBUG
}