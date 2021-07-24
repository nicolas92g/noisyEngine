#pragma once

//noisy
#include <configNoisy.hpp>
#include <Rendering/Mesh.h>
#include <Utils/BiArray.h>

//stl
#include <vector>
#include <array>
#include <memory>

//glm
#include <glm/glm.hpp>

namespace ns::Plane {
	class HeightmapStorage
	{
	public:
		struct Settings {
			MapLengthType chunkPhysicalSize = MapLengthType(32);
			ChunkPartitionType numberOfPartitions = ns::defaultSize;

			HeightType(*generator_)(const MapLengthType&);
		};

		struct NeighborChunkLine {
			std::vector<glm::vec<3, LengthType>> positions;		//lines of vertices, the heights are stored in y and the rest is computed by the meshGenerator 
			GridPositionType chunkPos;			//chunk where the vertices are
			GridPositionType neighborChunk;		//chunk that is really close to these vertices
			bool xzComputed = false;
		};

		//need a chunk pos to work
		using Input = GridPositionType;

		//output an array of heights but also the heights really close to the the chunk
		struct Result {
			Result(const glm::ivec2 biarraySize) :
				values(biarraySize),
				chunk(0, 0),
				neighbors()
			{}

			BiArray<HeightType> values;
			std::array<NeighborChunkLine*, 4> neighbors;
			ns::GridPositionType chunk;

			static constexpr unsigned int bottom = 0;	//neighbors are stored in the array using those values
			static constexpr unsigned int top = 1;
			static constexpr unsigned int left = 2;
			static constexpr unsigned int right = 3;
		};

	public:
		HeightmapStorage(const HeightmapStorage::Settings& settings);

		std::shared_ptr<Result> operator()(const Input& input);

	protected:
		const Settings settings_;

		const struct preComputedValues
		{
			MapLengthType primitiveSize;
		} data_;

		std::vector<NeighborChunkLine> partiallyComputedChunks_;

		friend class MeshGenerator;		
	};
}
