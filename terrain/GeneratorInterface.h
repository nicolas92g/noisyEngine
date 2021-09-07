//renderer
#include <noisy.hpp>
#include <configNoisy.hpp>

#include "Plane/HeightMapGenerator.h"
#include "Plane/FlatTerrainScene.h"

namespace ns {

	struct GeneratorInterfaceSettings {
		float mouseSensivity = .004f;
		float cameraSpeed = 5.f;
	};

	class GeneratorInterface {
	public:
		GeneratorInterface();
		~GeneratorInterface();

		int run();

	protected:
		Window window_;
		Camera cam_;
		Scene scene_;
		Renderer3d renderer_;
		Plane::HeightMapGenerator::Settings generation_;

		GeneratorInterfaceSettings settings_;
		MapLengthType chunkSize_;
		ChunkPartitionType chunkRes_;

		struct PlaneGeneration {
			PlaneGeneration(const ns::Plane::HeightMapGenerator::Settings heightGenerationSettings, const MapLengthType& chunkSize, const ChunkPartitionType& numberOfParts) :
				heightFunction(heightGenerationSettings),
				terrainSettings({ chunkSize , numberOfParts }),
				renderer(terrainSettings, heightFunction)
			{}
			Plane::HeightMapGenerator heightFunction;
			Plane::FlatTerrainScene::Settings terrainSettings;
			Plane::FlatTerrainScene renderer;
		};

		std::unique_ptr<PlaneGeneration> plane_;


		

	protected:
		

		static size_t heightComputationCount;
		static std::vector<MapLengthType> heightComputationPos;

		void mainMenu();
		void generationMenu();

		void inputOctave(ns::Plane::HeightMapGenerator::Octave& octave, int index);
		void debugOptimisationHeightsComputations(); //count if the height function is called multiple times for the same point
	};
}