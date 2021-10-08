#pragma once

//ns
#include <Utils/BiArray.h>
#include <Utils/DebugLayer.h>
#include "SphereChunk.h"
#include "Sphere.h"
#include <Rendering/Mesh.h>

//stl
#include <array>

namespace ns::Sphere {
	class SphereContainer
	{
	public:
		SphereContainer(uint32_t resolution, float sphereRadius);

		double sphereProgressionPercentage() const;
		std::shared_ptr<Mesh> getDebugSphere() const;
		/**
		 * @brief input a position relative to the sphere and normalized
		 * \param normalizedVector
		 * \return 
		 */
		std::shared_ptr<SphereChunk> findChunk(const glm::vec3& normalizedVector);

		std::shared_ptr<Mesh> test(const glm::vec3& position) {
			std::vector<ns::Vertex> vert;
			std::vector<unsigned> ind;
			std::vector<uint8_t> faces;

			if (position.z <= position.x and position.z <= position.y and position.z < 0) faces.push_back(0);
			if (position.x >= position.y and position.x >= position.z and position.x > 0) faces.push_back(1);
			if (position.z >= position.x and position.z >= position.y and position.z > 0) faces.push_back(2);
			if (position.x <= position.y and position.x <= position.z and position.x < 0) faces.push_back(3);
			if (position.y >= position.x and position.y >= position.z and position.y > 0) faces.push_back(4);
			if (position.y <= position.x and position.y <= position.z and position.y < 0) faces.push_back(5);

			for (uint8_t f = 0; f < faces.size(); f++)
			{
				const auto face = faces[f];
				const auto& region = findRegion(subRegions[face], position);
				//dout << "founded region : \n";
				//logRegion(region);
				
				for (uint32_t i = region.firstChunkIndex.x; i < static_cast<uint32_t>(region.lastChunkIndex.x + 1); i++)
				{
					for (uint32_t j = region.firstChunkIndex.y; j < static_cast<uint32_t>(region.lastChunkIndex.y + 1); j++)
					{
						if (checkCoordIsInLimit(position, terrain_[face].value(i, j).limit)) {
							const auto& coo = terrain_[face].value(i, j).coords;
				
							vert.emplace_back(vertex(coo.a).position);
							vert.emplace_back(vertex(coo.b).position);
							vert.emplace_back(vertex(coo.c).position);
							vert.emplace_back(vertex(coo.c).position);
							vert.emplace_back(vertex(coo.b).position);
							vert.emplace_back(vertex(coo.d).position);
				
						}
					}
				}
			}

			if (vert.empty()) {
				vert.emplace_back(glm::vec3(10, 0, 0));
				vert.emplace_back(glm::vec3(0, 0, 10));
				vert.emplace_back(glm::vec3(-10, 0, 0));
				vert.emplace_back(glm::vec3(0, 0, 10));
				vert.emplace_back(glm::vec3(-10, 0, 0));
				vert.emplace_back(glm::vec3(-10, 0, -10));
			}
		
			MeshConfigInfo info;
			info.indexedVertices = false;
			return std::make_shared<ns::Mesh>(vert, ind, Material(glm::vec3(0), .1f, 1, glm::vec3(1, 0, 0)), info);
		}

		float radius() const;
		
	protected:
		//define a vertex on the spheric grid
		struct Vertex {
			glm::vec3 position;
		};

		//define the value that allow to access memory
		struct Index {
			Index(uint8_t face = NULL_FACE_INDEX, uint16_t i = 0, uint16_t j = 0) : face(face), i(i), j(j){}
			uint8_t face;	//index between 0 and 5, it is the face of the cube
			uint16_t i;		//x on the square face
			uint16_t j;		//y on the square face
			bool isNull() const { return (face == NULL_FACE_INDEX); }
			static const Index null;
		};

		//define some chunk coordinates (a square with four vertices but vertices are not copied, they are indexed)
		struct ChunkCoords {
			Index a;
			Index b;
			Index c;
			Index d;
		};

		//store the min and max of each position components
		struct ChunkLimits {
			float minX;
			float maxX;
			float minY;
			float maxY;
			float minZ;
			float maxZ;
		};

		//describe a zone of chunks to allow fast search of a chunk
		struct ChunksRegion {
			ChunksRegion(glm::u16vec2 first = { 0,0 }, glm::u16vec2 last = { 0,0 }) : firstChunkIndex(first), lastChunkIndex(last) {}
			ChunkLimits limit{};//limits of the region
			glm::u16vec2 firstChunkIndex;	//smallest chunk index
			glm::u16vec2 lastChunkIndex;	//biggest chunk index
			std::shared_ptr<ns::BiArray<ChunksRegion>> innerRegions;//array of four regions or nothing if it is too small
		};

		//describe a chunk
		struct Chunk {
			std::shared_ptr<SphereChunk> mesh;//mesh of the chunk
			ChunkCoords coords{};//position of the chunk
			ChunkLimits limit{};//limits of the chunk
		};

	protected:
		//parameters
		const uint32_t resolution_;
		const uint32_t resolutionPlusOne_;
		const float radius_;

		//data
		std::array<ns::BiArray<Chunk>, NUMBER_OF_FACES_IN_A_CUBE> terrain_;
		std::array<ns::BiArray<Vertex>, NUMBER_OF_FACES_IN_A_CUBE> vertices_;
		std::array<ChunksRegion, NUMBER_OF_FACES_IN_A_CUBE> subRegions;	//sub regions of the chunk that allow to quickly search for a chunk
		
		//multi-threading
		std::thread sphereThread_;
		std::atomic_uint32_t sphereProgression_;

	protected:
		bool checkCoordIsInLimit(const glm::vec3& pos, const ChunkLimits& limit) const;//allow to know if a position is in a chunk
		void fillChunkLimits(ChunkLimits& limit, const ChunkCoords& chunkPos);//compute the limits of a chunk
		void fillChunkSubRegions(ChunksRegion& chunk, uint8_t face);		//compute the sub-regions of a face

		const ChunksRegion& findRegion(const ChunksRegion& region, const glm::vec3& pos) const;//recursive function that find the smallest sub region that contain the chunk

		const Vertex& vertex(const Index& index) const;//get a vertex of the spheric grid
		Vertex& vertex(const Index& index);	//get a vertex of the spheric grid

		const Chunk& chunk(const Index& index) const;//get a chunk of the terrain
		Chunk& chunk(const Index& index);//get a chunk of the terrain
		
		Index find(const glm::vec3& normalizedVector) const;//find a chunk index with the normalized position relative to the sphere
		Index find(const Index& previousIndex) const;//find a chunk index by searching around the previous chunk

		static void genSphereVertices(SphereContainer* object);//create the grid in the object (multi-threadable function)

		static void logRegion(const ChunksRegion& region){ 
			dout << "\nstart :\nfirst = " << to_string((glm::ivec2)region.firstChunkIndex) <<
				"\nlast = " << to_string((glm::ivec2)region.lastChunkIndex) << "\n-------->\n\n";

			if (region.innerRegions) {
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[0].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[0].lastChunkIndex) << "\n--\n";
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[1].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[1].lastChunkIndex) << "\n--\n";
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[2].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[2].lastChunkIndex) << "\n--\n";
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[3].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[3].lastChunkIndex) << "\n\n";
			}
		}
	};
}
