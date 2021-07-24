#include "FlatTerrainScene.h"
#include "configNoisy.hpp"

std::array<std::vector<ns::GridPositionType>, ns::maximunRenderDistance> ns::Plane::FlatTerrainScene::searchingOrder = ns::Plane::FlatTerrainScene::getOrder();

ns::Plane::FlatTerrainScene::FlatTerrainScene(const Settings& settings, HeightmapStorage& heightMapGenerator)
	:
	settings_(settings),
	heightGen_(heightMapGenerator),
	meshGen_(heightGen_),
	numberOfChunks_(0)
{
	chunks_ = std::make_unique<BiArray<Chunk>>(terrainArraySizeNeeded(settings_.load().renderDistance));
}

void ns::Plane::FlatTerrainScene::update(GridPositionType& centerPosition)
{
	//check if the chunk loading condition has changed
	if (centerPosition != centralChunk_.load()) {
		stopAndRestartSearchingThread();
	}
}

void ns::Plane::FlatTerrainScene::setScene(Scene& scene)
{
	scene_ = &scene;
}

void ns::Plane::FlatTerrainScene::checkRenderDistanceCapacity()
{
	const glm::ivec2 minSize = terrainArraySizeNeeded(settings_.load().renderDistance);

	if (chunks_->x() < minSize.x or chunks_->y() < minSize.y) {
		const BiArray<Chunk> copy(*chunks_);
	
		chunks_ = std::make_unique<BiArray<Chunk>>(minSize);
		*chunks_ = copy;
	}
}

glm::ivec2 ns::Plane::FlatTerrainScene::terrainArraySizeNeeded(unsigned renderDistance)
{
	return glm::ivec2(renderDistance) * 2 + glm::ivec2(1);
}

void ns::Plane::FlatTerrainScene::searchingThreadFunction(FlatTerrainScene* ptr)
{
	FlatTerrainScene& object = *ptr;
	for (unsigned dst = 0; dst <= object.settings_.load().renderDistance; ++dst)
	{
		for (size_t i = 0; i < searchingOrder[dst].size(); i++)
		{
			//check if this thread receive a close request
			if (object.stopSearchingThread_.load()) return;

			//wait until the number of loading threads in less than the fixed limit
			if (object.loadingFutures_.size() >= object.settings_.load().maxChunksLoadingThreads) {
				object.loadingFutures_.erase(object.loadingFutures_.begin()); //wait for the oldest thread to finish by calling his future's destructor
			}

			const GridPositionType& chunkPos = searchingOrder[dst][i];
			Chunk& chunk = object.getChunk(chunkPos);

			//if the chunk was already loaded or is currently loading
			if (chunk.wasProcessed) continue;

			//launch loading thread
			object.loadingFutures_.emplace_back(std::async(std::launch::async, &loadingThreadFunction, &object, chunkPos));
		}
	}
}

void ns::Plane::FlatTerrainScene::stopAndRestartSearchingThread()
{
	//check that the a thread object was created
	if (searchingThread_.get() != nullptr) {

		stopSearchingThread_ = true;	//request the thread to stop
		searchingThread_->join();		//wait for it to stop
		stopSearchingThread_ = false;	//remove the stop request for the next thread
	}

	//create or recreate the thread object
	searchingThread_ = std::make_unique<std::thread>(&searchingThreadFunction, this);
}

ns::Plane::FlatTerrainScene::Chunk& ns::Plane::FlatTerrainScene::getChunk(const GridPositionType& gridPos)
{
	GridPositionType location = gridPos - originChunk_.load();
	return chunks_->value(location.x, location.y);
}

void ns::Plane::FlatTerrainScene::loadingThreadFunction(FlatTerrainScene* object, ns::GridPositionType chunk)
{

	auto heightmap = object->heightGen_(chunk);

	MeshGenerator::Result meshDescription;
	object->meshGen_(*heightmap, meshDescription);

}

std::array<std::vector<ns::GridPositionType>, ns::maximunRenderDistance> ns::Plane::FlatTerrainScene::getOrder()
{
	//(dst, dst) / (-1, 0) * dst * 2 / (0, -1) * dst * 2 / (1, 0) * dst * 2 / (0, 1) * (dst * 2 - 1)
	std::array<std::vector<ns::GridPositionType>, maximunRenderDistance> ret;

	//central chunk offset is of course null
	ret[0].push_back(GridPositionType(0, 0));

	for (size_t dst = 1; dst < maximunRenderDistance; dst++)
	{
		std::vector<ns::GridPositionType>& circle = ret[dst];
		circle.resize(8 * dst);

		//start by the chunk that is at ivec2(dst, dst)
		circle[0] = GridPositionType(static_cast<int>(dst));
		unsigned cursor = 1;

		//then go to the top-left corner
		for (size_t i = 0; i < dst * 2; i++)
		{
			circle[cursor] = circle[cursor - 1] + GridPositionType(-1, 0);
			cursor++;
		}
		//then to the bottom-left corner
		for (size_t i = 0; i < dst * 2; i++)
		{
			circle[cursor] = circle[cursor - 1] + GridPositionType(0, -1);
			cursor++;
		}
		//then to the bottom-right corner
		for (size_t i = 0; i < dst * 2; i++)
		{
			circle[cursor] = circle[cursor - 1] + GridPositionType(1, 0);
			cursor++;
		}
		//then just under the start at ivec2(dst, dst - 1)
		for (size_t i = 0; i < dst * 2 - 1; i++)
		{
			circle[cursor] = circle[cursor - 1] + GridPositionType(0, 1);
			cursor++;
		}
		//and all the chunks in the square that has a size of dst * 2 + 1 are in the array at the index dst
	}
	return ret;
}
