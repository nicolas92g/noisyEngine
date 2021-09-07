#include "GeneratorInterface.h"
#include <Utils/DebugLayer.h>

//noisy terrain
#include <terrain/Plane/HeightMapGenerator.h>
#include <terrain/Plane/HeightmapStorage.h>
#include <terrain/Plane/MeshGenerator.h>
#include <terrain/Plane/FlatTerrainScene.h>

//imgui
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

std::vector<ns::MapLengthType> ns::GeneratorInterface::heightComputationPos;

ns::GeneratorInterface::GeneratorInterface()
	:
	window_(1920, 1080, "noisy terrain generator v0 alpha", 2),
	cam_(glm::vec3(9, 6, -8), -.6, 1.5, 1.8),
	scene_(DirectionalLight::nullLight()),
	renderer_(window_, cam_, scene_, Renderer3dCreateInfo()),
	chunkSize_(ns::defaultSize),
	chunkRes_(32)
{
	Debug::get().setWindow(window_);
	Debug::get().setCamera(cam_);
	Debug::get().setRenderer3d(renderer_);
	Debug::get().setScenes({&scene_});
}

int ns::GeneratorInterface::run()
{
	Timer t("this programm");
	using namespace ns::Plane;

	//height map function
	generation_.octaves = {
		{.05,  5, 5.5, false},
		{.2,  20, -.6, false},
		//{.4,  .25, 1.5, false},
		//{2, .5, 2.5, false}
	};
	generation_.exponent = 1;


	plane_ = std::make_unique<PlaneGeneration>(generation_, chunkSize_, chunkRes_);
	plane_->renderer.setRenderDistance(8);

	DirectionalLight sun;
	Scene initialScene(sun);

	glEnable(GL_CULL_FACE);

	while (window_.shouldNotClose()) {
		window_.beginFrame();

		ImGui::Begin("terrain");
		mainMenu();
		generationMenu();

		ImGui::End();

		plane_->renderer.update(ns::GridPositionType(0));

		cam_.classicKeyboardControls(window_, settings_.cameraSpeed);
		cam_.classicMouseControls(window_, settings_.mouseSensivity);

		scene_ = initialScene + plane_->renderer.lockScene();

		renderer_.startRendering();
		renderer_.finishRendering();

		plane_->renderer.unlockScene();

		Debug::get().render();

		window_.inputFullscreen(GLFW_KEY_F11);
		window_.endFrame();

		if (window_.key(GLFW_KEY_ESCAPE))
			window_.setShouldClose(true);
	}


	return EXIT_SUCCESS;
}

void ns::GeneratorInterface::mainMenu()
{
	using namespace ImGui;
	
	Separator();
	Text("camera speed"); ImGui::SameLine();
	SliderFloat("##camera speed", &settings_.cameraSpeed, .1f, 100.f);
	Text("mouse sensivity"); ImGui::SameLine();
	SliderFloat("##mouse sensivity", &settings_.mouseSensivity, .001f, .02f);

	//Text(("number of vertices : " + std::to_string((settings.numberOfPartitions.x + 1) * (settings.numberOfPartitions.y + 1))).c_str());
	//Text(("number of computed heights : " + std::to_string(heightComputationCount)).c_str());
}

void ns::GeneratorInterface::generationMenu()
{
	using namespace ImGui;
	if (CollapsingHeader("generation##terrain")) {

		int buf = plane_->renderer.renderDistance();
		Text("renderDistance"); SameLine(); SliderInt("##renderDistance", &buf, 0, 100);
		plane_->renderer.setRenderDistance(buf);
		Separator();

		for (size_t i = 0; i < generation_.octaves.size(); i++)
		{
			inputOctave(generation_.octaves[i], i);
		}

		Separator();

		if (Button("regenerate")) {
			plane_ = std::make_unique<PlaneGeneration>(generation_, chunkSize_, chunkRes_);
			plane_->renderer.setRenderDistance(8);
			plane_->renderer.setMaxOfLoadingThreads(1);
		}
	}
}

void ns::GeneratorInterface::inputOctave(ns::Plane::HeightMapGenerator::Octave& octave, int index)
{
	using namespace ImGui;
	if (TreeNode(("Octave " + std::to_string(index)).c_str())) {
		Separator();
		Text("Amplitude"); SameLine();
		DragFloat(("##amplitudeInput" + std::to_string(index)).c_str(), &octave.amplitude, .001f);

		Text("Frequency"); SameLine();
		DragFloat(("##frequencyInput" + std::to_string(index)).c_str(), &octave.frequency, .001f);

		Text("Offset"); SameLine();
		DragFloat(("##offsetInput" + std::to_string(index)).c_str(), &octave.offset, .001f);

		Text("Ridged"); SameLine();
		Checkbox(("##ridgedInput" + std::to_string(index)).c_str(), &octave.ridged);
		Separator();

		TreePop();  
	}
}

void ns::GeneratorInterface::debugOptimisationHeightsComputations()
{
	std::unordered_map<LengthType, std::unordered_map<LengthType, std::optional<unsigned>>> count;

	for (const auto& pos : heightComputationPos)
	{
		if (!count[pos.x][pos.y].has_value())
			count[pos.x][pos.y] = 1;
		else
			count[pos.x][pos.y].value()++;
	}

	std::array<unsigned, 200> numbersOfcomputations;
	unsigned max = 0;

	for (const auto arr : count) {
		for (const auto value : arr.second) {
			if (value.second.value() < 200) {
				numbersOfcomputations[value.second.value()]++;
				max = std::max(value.second.value(), max);
			}
		}
	}
	for (size_t i = 1; i <= max; i++)
	{
		if (numbersOfcomputations[i] == 0) continue;
		Debug::get() << numbersOfcomputations[i] << " heights has been computed " << i << " times !\n";
	}
}

ns::GeneratorInterface::~GeneratorInterface()
{
	Material::clearTextures();
}
