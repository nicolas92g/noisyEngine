#pragma once
#include <Rendering/Camera.h>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include "HeightmapStorage.h"
#include "MeshGenerator.h"
#include <Utils/BiArray.h>

namespace ns::Plane{
	/**
	 * @brief organize a collection of meshes that create a flat terrain
	 * meshes are created by another class, rendering is made by another class 
	 * plan : 
	 * run all on a secondary thread and launch some threads to load chunks until number of loading threads hit a fixed limit
	 * if all the chunks are loaded, end the thread, then the update function check that the central chunk is unedited and that the loading 
	 * settings are still the same, else, the update function launch the searching thread.
	 * the searching thread work by using some precomputed values that turn around the central chunk to search a new chunk to load 
	 * we need to know for each chunk grid pos if the chunk is loaded or in loading or if it is not loaded at all 
	 * 
	 */
	class FlatTerrainScene
	{
	public:
		struct Settings {
			Settings(MapLengthType size, ChunkPartitionType parts) : chunkPhysicalSize(size), numberOfPartitions(parts) {}
			Settings(const HeightmapStorage::Settings& settings) : 
				Settings(settings.chunkPhysicalSize, settings.numberOfPartitions)
			{}
			MapLengthType chunkPhysicalSize = MapLengthType(16.0);
			ChunkPartitionType numberOfPartitions = ns::defaultSize;
		};
		FlatTerrainScene(const Settings& settings, const HeightMapGenerator& function);
		~FlatTerrainScene();

		void update(const GridPositionType& centralChunk);

		Scene& lockScene();
		void unlockScene();

		void setRenderDistance(uint16_t renderDistance);
		void setMaxOfLoadingThreads(uint16_t maxThreads);

		uint16_t renderDistance() const;
		uint16_t maxLoadingThreads() const;

		void importFromYAML();
		void exportIntoYAML();

	protected:

		//dynamic settings
		std::atomic_uint16_t renderDistance_;
		std::atomic_uint16_t maxChunksLoadingThreads_;

		struct Chunk {
			std::shared_ptr<Mesh> mesh;					//chunk mesh
			std::shared_ptr<DrawableObject3d> object;	//chunk object
			GridPositionType position;					//position on a grid plane 
			bool wasProcessed = false;					//indicate if the chunk is loaded or currently in loading
		};

		struct ChunkToCreate {
			ns::GridPositionType position;
			MeshGenerator::Result meshData;
		};

		std::atomic<Settings> settings_;

		//generators
		HeightmapStorage heightStorage_;
		MeshGenerator meshGen_;

		//data storage
		Scene scene_;
		std::mutex sceneMutex_;

		std::vector<ChunkToCreate> chunksData_;
		std::mutex chunksDataMutex_;

		std::unique_ptr<BiArray<Chunk>> chunks_;
		std::atomic_uint32_t numberOfChunks_;

		//chunks loading
		std::atomic<GridPositionType> centralChunk_;//store the central chunk grid position
		std::atomic<GridPositionType> originChunk_;//store the first chunk position of the array of chunks

		std::unique_ptr<std::thread> searchingThread_; //thread that search for new chunk to load
		std::atomic_bool stopSearchingThread_;

		std::vector<std::future<void>> loadingFutures_;
		std::mutex loadingFuturesMutex_;

	protected:
		void checkRenderDistanceCapacity();
		static glm::ivec2 terrainArraySizeNeeded(unsigned renderDistance);

		static void searchingThreadFunction(FlatTerrainScene* object);
		static void loadingThreadFunction(FlatTerrainScene* object, ns::GridPositionType chunk);


		void moveChunkArray(const GridPositionType& newCentralChunk);
		//this function check if the searching is already running and stop it to restart it from the beginning
		void stopAndRestartSearchingThread();

		Chunk& getChunk(const GridPositionType& gridPos);

	public:
		//store a list of grid position that allow to check all the chunks near one central chunk one by one by iterating through those arrays
		static std::array<std::vector<ns::GridPositionType>, maximunRenderDistance> getOrder();			//create the arrays
		static std::array<std::vector<ns::GridPositionType>, maximunRenderDistance> searchingOrder;		//store the arrays
	};
}


