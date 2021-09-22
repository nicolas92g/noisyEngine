#include "FlatTerrainScene.h"
#include "configNoisy.hpp"

std::array<std::vector<ns::GridPositionType>, ns::maximunRenderDistance> ns::Plane::FlatTerrainScene::searchingOrder = ns::Plane::FlatTerrainScene::getOrder();

ns::Plane::FlatTerrainScene::FlatTerrainScene(const Settings& settings, const HeightMapGenerator& function)
	:
	settings_(settings),
	heightStorage_(HeightmapStorage::Settings(function, settings.chunkPhysicalSize, settings.numberOfPartitions)),
	meshGen_(heightStorage_),
	numberOfChunks_(0),
	renderDistance_(8),
	maxChunksLoadingThreads_(std::max(std::thread::hardware_concurrency(), 1U)),
	scene_(DirectionalLight::nullLight())
{
	chunks_ = std::make_unique<BiArray<Chunk>>(terrainArraySizeNeeded(renderDistance_));

	//initialize biarray
	centralChunk_ = GridPositionType(0);
	originChunk_ = GridPositionType(renderDistance_ * -1);

	//start searching thread
	stopAndRestartSearchingThread();

	importFromYAML();
}

ns::Plane::FlatTerrainScene::~FlatTerrainScene()
{
	//Stop searchingThread if it is running
	stopSearchingThread_ = true;	//request the thread to stop

	exportIntoYAML();

	searchingThread_->join();		//wait for it to stop
}

void ns::Plane::FlatTerrainScene::update(const GridPositionType& centerPosition)
{
	//check if the chunk loading condition has changed
	if (centerPosition != centralChunk_.load()) {
		stopAndRestartSearchingThread();
	}

	std::scoped_lock chunksDataLock(chunksDataMutex_);

	if(chunksData_.size())
		dout << "adding " << chunksData_.size() << " meshes !\n";

	for (const auto& data : chunksData_)
	{
		auto& chunk = getChunk(data.position);

		if (chunk.wasProcessed) continue;
		chunk.position = data.position;

		MeshConfigInfo info;
		info.primitive = data.meshData.primitiveType;
		info.indexedVertices = data.meshData.indexed;

		chunk.mesh = std::make_shared<ns::Mesh>(data.meshData.vertices, data.meshData.indices, Material::getDefault(), info);
		chunk.object = std::make_shared<ns::DrawableObject3d>(*chunk.mesh);

		scene_.addStatic(*chunk.object);
		chunk.wasProcessed = true;
	}

	chunksData_.clear();
}

ns::Scene& ns::Plane::FlatTerrainScene::lockScene()
{
	sceneMutex_.lock();
	return scene_;
}

void ns::Plane::FlatTerrainScene::unlockScene()
{
	sceneMutex_.unlock();
}

void ns::Plane::FlatTerrainScene::setRenderDistance(uint16_t renderDistance)
{
	renderDistance_ = renderDistance;
}

void ns::Plane::FlatTerrainScene::setMaxOfLoadingThreads(uint16_t maxThreads)
{
	maxChunksLoadingThreads_ = maxThreads;
}

uint16_t ns::Plane::FlatTerrainScene::renderDistance() const
{
	return renderDistance_.load();
}

uint16_t ns::Plane::FlatTerrainScene::maxLoadingThreads() const
{
	return maxChunksLoadingThreads_.load();
}

void ns::Plane::FlatTerrainScene::importFromYAML()
{
	try {
		renderDistance_ = conf["planeTerrain"]["renderdistance"].as<int>();
		maxChunksLoadingThreads_ = conf["planeTerrain"]["maxThreads"].as<int>();
	}
	catch (...) {

	}
}

void ns::Plane::FlatTerrainScene::exportIntoYAML()
{
	conf["planeTerrain"]["renderdistance"] = renderDistance_.load();
	conf["planeTerrain"]["maxThreads"] = maxChunksLoadingThreads_.load();
}

void ns::Plane::FlatTerrainScene::checkRenderDistanceCapacity()
{
	const glm::ivec2 minSize = terrainArraySizeNeeded(renderDistance_);
	
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
	dout << "started searching...\n";
	FlatTerrainScene& object = *ptr;
	for (unsigned dst = 0; dst <= object.renderDistance_; ++dst)
	{
		for (size_t i = 0; i < searchingOrder[dst].size(); i++)
		{
			
			//check if this thread receive a close request
			if (object.stopSearchingThread_.load()) return;

			{
				std::scoped_lock lock(object.loadingFuturesMutex_);

				//wait until the number of loading threads in less than the fixed limit
				if (object.loadingFutures_.size() >= object.maxChunksLoadingThreads_) {
					object.loadingFutures_.erase(object.loadingFutures_.begin()); //wait for the oldest thread to finish by calling his future's destructor
				}
			}

			const GridPositionType& chunkPos = searchingOrder[dst][i];
			Chunk& chunk = object.getChunk(chunkPos);

			//if the chunk was already loaded or is currently loading
			if (chunk.wasProcessed) continue;

			std::scoped_lock lock(object.loadingFuturesMutex_);

			//launch loading thread
			object.loadingFutures_.emplace_back(std::async(std::launch::async, &loadingThreadFunction, &object, chunkPos));
		}
	}
}

void ns::Plane::FlatTerrainScene::loadingThreadFunction(FlatTerrainScene* object, ns::GridPositionType chunk)
{
	dout << "loading chunk " << to_string(chunk) << '\n';
	ChunkToCreate ret;
	ret.position = chunk;
	auto heightmap = object->heightStorage_(chunk);

	object->meshGen_(*heightmap, ret.meshData);

	std::scoped_lock chunkDataProtection(object->chunksDataMutex_);
	object->chunksData_.emplace_back(ret);
}

void ns::Plane::FlatTerrainScene::moveChunkArray(const GridPositionType& newCentralChunk)
{
	ns::BiArray<Chunk> copy(*chunks_);


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
