#include "HeightmapStorage.h"

#define WIDTH (settings_.numberOfPartitions.x + (size_t)1U)
#define HEIGHT (settings_.numberOfPartitions.y + (size_t)1U) 

#define WIDTH_F (float)(settings_.numberOfPartitions.x + 1.f)
#define HEIGHT_F (float)(settings_.numberOfPartitions.y + 1.f)

ns::Plane::HeightmapStorage::HeightmapStorage(const ns::Plane::HeightmapStorage::Settings& settings)
	:
	settings_(settings),
	data_({ 
		(MapLengthType)settings_.chunkPhysicalSize / (MapLengthType)settings_.numberOfPartitions
		})
{}

std::shared_ptr<ns::Plane::HeightmapStorage::Result> ns::Plane::HeightmapStorage::operator()(const Input& input)
{
	const MapLengthType chunkPosition = settings_.chunkPhysicalSize * (MapLengthType)input;

	//create the array of heights
	std::shared_ptr<ns::Plane::HeightmapStorage::Result> result = std::make_shared<Result>((glm::ivec2)settings_.numberOfPartitions + glm::ivec2(1));
	result->chunk = input;

	for (size_t i = 0; i < result->values.x(); i++)
	{
		for (size_t j = 0; j < result->values.y(); j++)
		{
			result->values.value(i, j) = settings_.generator(MapLengthType(
				settings_.chunkPhysicalSize.x * input.x + i * data_.primitiveSize.x,
				settings_.chunkPhysicalSize.y * input.y + j * data_.primitiveSize.y
			));
		}
	}

	//calculate proximity vertices heights
	
	NeighborChunkLine left;
	left.chunkPos = input + GridPositionType(-1, 0);
	left.neighborChunk = input;
	left.positions.resize(HEIGHT);
	for (size_t i = 0; i < HEIGHT; i++)
	{
		left.positions[i].y = settings_.generator(MapLengthType(
			settings_.chunkPhysicalSize.x * input.x - data_.primitiveSize.x,
			settings_.chunkPhysicalSize.y * input.y + i * data_.primitiveSize.y
		));
	}
	
	NeighborChunkLine right;
	right.chunkPos = input + GridPositionType(1, 0);
	right.neighborChunk = input;
	right.positions.resize(HEIGHT);
	for (size_t i = 0; i < HEIGHT; i++)
	{
		right.positions[i].y = settings_.generator(MapLengthType(
			settings_.chunkPhysicalSize.x * input.x + (WIDTH) *  data_.primitiveSize.x,
			settings_.chunkPhysicalSize.y * input.y + i * data_.primitiveSize.y
		));
	}

	NeighborChunkLine bottom;
	bottom.chunkPos = input + GridPositionType(0, -1);
	bottom.neighborChunk = input;
	bottom.positions.resize(WIDTH);
	for (size_t i = 0; i < WIDTH; i++)
	{
		bottom.positions[i].y = settings_.generator(MapLengthType(
			settings_.chunkPhysicalSize.x * input.x + i * data_.primitiveSize.x,
			settings_.chunkPhysicalSize.y * input.y - data_.primitiveSize.y
		));
	}

	NeighborChunkLine top;
	top.chunkPos = input + GridPositionType(0, 1);
	top.neighborChunk = input;
	top.positions.resize(WIDTH);
	for (size_t i = 0; i < WIDTH; i++)
	{
		top.positions[i].y = settings_.generator(MapLengthType(
			settings_.chunkPhysicalSize.x * input.x + i * data_.primitiveSize.x,
			settings_.chunkPhysicalSize.y * input.y + (HEIGHT) * data_.primitiveSize.y
		));
	}

	partiallyComputedChunks_.reserve(partiallyComputedChunks_.size() + 4);

	partiallyComputedChunks_.emplace_back(right);
	result->neighbors[Result::right] = &partiallyComputedChunks_.back();

	partiallyComputedChunks_.emplace_back(left); 
	result->neighbors[Result::left] = &partiallyComputedChunks_.back();

	partiallyComputedChunks_.emplace_back(top); 
	result->neighbors[Result::top] = &partiallyComputedChunks_.back();

	partiallyComputedChunks_.emplace_back(bottom); 
	result->neighbors[Result::bottom] = &partiallyComputedChunks_.back();

	return result;
}
