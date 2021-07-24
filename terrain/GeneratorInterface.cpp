#include "GeneratorInterface.h"
#include <Utils/DebugLayer.h>

//noisy terrain
#include <terrain/Plane/HeightmapStorage.h>
#include <terrain/HeightMapGenerator.h>
#include <terrain/Plane/MeshGenerator.h>
#include <terrain/Plane/FlatTerrainScene.h>

//imgui
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

size_t ns::GeneratorInterface::heightComputationCount;
std::vector<ns::MapLengthType> ns::GeneratorInterface::heightComputationPos;

ns::GeneratorInterface::GeneratorInterface()
	:
	window_(1920, 1080, "noisy terrain generator v0 alpha", 2),
	cam_(glm::vec3(9, 6, -8), -.6, 1.5, 1.8),
	scene_(),
	renderer_(window_, cam_, scene_, Renderer3dCreateInfo())
{
	Debug::get().setWindow(window_);
	Debug::get().setCamera(cam_);
	Debug::get().setRenderer3d(renderer_);
}

int ns::GeneratorInterface::run()
{
	using namespace ns::Plane;

	constexpr PartitionType factor = 4;
	HeightmapStorage::Settings settings;
	settings.numberOfPartitions = ChunkPartitionType(32) * factor;
	settings.chunkPhysicalSize = MapLengthType(32) * (LengthType)factor;

	settings.generator_ = [](const MapLengthType& pos) {
		heightComputationCount++;
		return 
			abs(simplexNoise(pos * .1lt)) * 1 +
			abs(simplexNoise(pos * .05lt)) * 4 +
			abs(simplexNoise(pos * .01lt)) * 8 
			;
	};

	HeightmapStorage generateHeightMap(settings);

	
	MeshGenerator::Settings meshSettings;
	meshSettings.normals = MeshGenerator::Settings::Normals::smooth;
	MeshGenerator terrainGenerator(generateHeightMap, meshSettings);

	auto heightmap = generateHeightMap(glm::ivec2(0, 0));
	MeshGenerator::Result meshDescription;
	terrainGenerator(*heightmap, meshDescription);

	//auto heightmap2 = generateHeightMap(glm::ivec2(1, 0));
	//MeshGenerator::Result meshDescription2;
	//terrainGenerator(*heightmap2, meshDescription2);

	MeshConfigInfo info;
	info.primitive = meshDescription.primitiveType;
	info.indexedVertices = meshDescription.indexed;
	Mesh mesh(meshDescription.vertices, meshDescription.indices, Material::getDefault(), info);
	//Mesh mesh2(meshDescription2.vertices, meshDescription2.indices, Material::getDefault(), info);

	DrawableObject3d chunk(mesh);
	scene_.addEntity(chunk);

	Model model("C:/Users/nicol/OneDrive/Documents/Graphismes/models/chunk/chunk.fbx");
	DrawableObject3d example(model, glm::vec3(-20, 0, 20), glm::vec3(10));
	scene_.addEntity(example);

	//DrawableObject3d chunk2(mesh2);
	//scene_.addEntity(chunk2);

	DirectionalLight sun;
	scene_.addLight(sun);

	glEnable(GL_CULL_FACE);

	

	while (window_.shouldNotClose()) {
		window_.beginFrame();

		ImGui::Begin("terrain");
		ImGui::Text("camera speed"); ImGui::SameLine();
		ImGui::SliderFloat("##camera speed", &settings_.cameraSpeed, .1f, 100.f);
		ImGui::Text("mouse sensivity"); ImGui::SameLine();
		ImGui::SliderFloat("##mouse sensivity", &settings_.mouseSensivity, .001f, .02f);
		ImGui::Text(("number of vertices : " + std::to_string((settings.numberOfPartitions.x + 1) * (settings.numberOfPartitions.y + 1))).c_str());
		ImGui::Text(("number of computed heights : " + std::to_string(heightComputationCount)).c_str());
		ImGui::End();

		cam_.classicKeyboardControls(window_, settings_.cameraSpeed);
		cam_.classicMouseControls(window_, settings_.mouseSensivity);

		renderer_.startRendering();
		renderer_.finishRendering();
		Debug::get().render();

		window_.inputFullscreen(GLFW_KEY_F11);
		window_.endFrame();
		if (window_.key(GLFW_KEY_ESCAPE))
			window_.setShouldClose(true);
	}
	ns::clearConfigFile();
	return EXIT_SUCCESS;
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
	PostProcessingLayer::deleteScreen();
	Material::clearTextures();
}
