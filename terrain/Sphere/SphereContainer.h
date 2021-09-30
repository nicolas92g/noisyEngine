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

		std::shared_ptr<SphereChunk> chunk(glm::vec2 angles);

		std::shared_ptr<Mesh> test(const glm::vec3& position) {
			std::vector<ns::Vertex> vert;
			std::vector<unsigned> ind;

			static const glm::vec3 normals[NUMBER_OF_FACES_IN_A_CUBE]
			{
				glm::vec3(0.0, 0.0, -1.0),
				glm::vec3(1.0, 0.0 ,0.0),
				glm::vec3(0.0, 0.0, 1.0),
				glm::vec3(-1.0, 0.0, 0.0),
				glm::vec3(0.0, 1.0 ,0.0),
				glm::vec3(0.0, -1.0, 0.0)
			};

			std::vector<uint8_t> faces;

			if (position.z <= position.x and position.z <= position.y and position.z < 0) faces.push_back(0);
			if (position.x >= position.y and position.x >= position.z and position.x > 0) faces.push_back(1);
			if (position.z >= position.x and position.z >= position.y and position.z > 0) faces.push_back(2);
			if (position.x <= position.y and position.x <= position.z and position.x < 0) faces.push_back(3);
			if (position.y >= position.x and position.y >= position.z and position.y > 0) faces.push_back(4);
			if (position.y <= position.x and position.y <= position.z and position.y < 0) faces.push_back(5);

			for (size_t f = 0; f < faces.size(); f++)
			{
				auto face = faces[f];
				for (size_t i = 0; i < terrain_[face].x(); i++)
				{
					for (size_t j = 0; j < terrain_[face].y(); j++)
					{
						if (checkCoordIsInChunk(position, terrain_[face].value(i, j))) {
							const auto& coo = terrain_[face].value(i, j).coords;

							vert.emplace_back(ns::Vertex(vertex(coo.a).position));
							vert.emplace_back(ns::Vertex(vertex(coo.b).position));
							vert.emplace_back(ns::Vertex(vertex(coo.c).position));
							vert.emplace_back(ns::Vertex(vertex(coo.c).position));
							vert.emplace_back(ns::Vertex(vertex(coo.b).position));
							vert.emplace_back(ns::Vertex(vertex(coo.d).position));

							//std::cout << "real face = " << (int)face;
						}
					}
				}
			}

			
			//for (size_t k = 0; k < faces.size(); k++)
			//{
			//	std::cout << " founded faces = " << (int)faces[k];
			//}
			//std::cout << ns::to_string(position) << newl;

			if (vert.empty()) {
				vert.emplace_back(ns::Vertex(glm::vec3(10, 0, 0)));
				vert.emplace_back(ns::Vertex(glm::vec3(0, 0, 10)));
				vert.emplace_back(ns::Vertex(glm::vec3(-10, 0, 0)));
				vert.emplace_back(ns::Vertex(glm::vec3(0, 0, 10)));
				vert.emplace_back(ns::Vertex(glm::vec3(-10, 0, 0)));
				vert.emplace_back(ns::Vertex(glm::vec3(-10, 0, -10)));
			}
		
			MeshConfigInfo info;
			info.indexedVertices = false;
			return std::make_shared<ns::Mesh>(vert, ind, Material(glm::vec3(0), .1f, 1, glm::vec3(1, 0, 0)), info);
		}
		
	protected:
		//define a vertex on the spheric grid
		struct Vertex {
			glm::vec3 position;
		};

		//define the value that allow to access memory
		struct Index {
			Index(uint8_t face = 0, uint16_t i = 0, uint16_t j = 0) : face(face), i(i), j(j){}
			uint8_t face;	//index between 0 and 5, it is the face of the cube
			uint16_t i;		//x on the square face
			uint16_t j;		//y on the square face
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
			ChunkLimits limit{};
			glm::u16vec2 firstChunkIndex;
			glm::u16vec2 lastChunkIndex;
			std::optional<ns::BiArray<ChunksRegion>> innerRegions;//array of four regions or nothing
		};

		//describe a chunk
		struct Chunk {
			std::shared_ptr<SphereChunk> mesh;//mesh of the chunk
			ChunkCoords coords{};//position of the chunk
			ChunkLimits limit{};//limits of the chunk
			ChunksRegion subRegions;//sub regions of the chunk that allow to quickly search for a chunk
		};

	protected:

		const uint32_t resolution_;
		const uint32_t resolutionPlusOne_;
		const float radius_;

		std::array<ns::BiArray<Chunk>, NUMBER_OF_FACES_IN_A_CUBE> terrain_;
		std::array<ns::BiArray<Vertex>, NUMBER_OF_FACES_IN_A_CUBE> vertices_;
		

		std::thread sphereThread_;
		std::atomic_uint32_t sphereProgression_;

	protected:
		bool checkCoordIsInChunk(glm::vec3 pos, const Chunk& chunk);
		void fillChunkLimits(ChunkLimits& limit, const ChunkCoords& chunkPos);
		void fillChunkSubRegions(ChunksRegion& chunk, uint8_t face);

		const Vertex& vertex(const Index& index) const { return vertices_[index.face].value(index.i, index.j); }
		Vertex& vertex(const Index& index) { return vertices_[index.face].value(index.i, index.j); }
		const Chunk& chunk(const Index& index) const { return terrain_[index.face].value(index.i, index.j); }
		Chunk& chunk(const Index& index) { return terrain_[index.face].value(index.i, index.j); }

		static void genSphereVertices(SphereContainer* object);
	};
}
