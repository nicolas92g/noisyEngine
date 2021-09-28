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

			uint8_t face_ = -1;

			if (position.z <= position.x and position.z <= position.y) face_ = 0;
			else if (position.x >= position.y and position.x >= position.z) face_ = 1;
			else if (position.z >= position.x and position.z >= position.y) face_ = 2;
			else if (position.x <= position.y and position.x <= position.z) face_ = 3;
			else if (position.y >= position.x and position.y >= position.z) face_ = 4;
			else if (position.y <= position.x and position.y <= position.z) face_ = 5;
			else {
				vert.emplace_back(ns::Vertex(glm::vec3(10, 0, 0)));
				vert.emplace_back(ns::Vertex(glm::vec3(0, 0, 10)));
				vert.emplace_back(ns::Vertex(glm::vec3(-10, 0, 0)));
			}
			for (size_t face = 0; face < 6; face++)
			{
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
							//std::cout << "face = " << face << "founded face = " << (int)face_ << newl;

						}
					}
				}
			}

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
		struct Vertex {
			glm::vec3 position;
		};

		struct Index {
			uint8_t face;
			uint16_t i;
			uint16_t j;
		};

		struct ChunkCoords {
			Index a;
			Index b;
			Index c;
			Index d;
		};

		struct Chunk {
			std::shared_ptr<SphereChunk> mesh;
			ChunkCoords coords;
			
			//sorted arrays of the chunks coordonates
			std::array<float, 4> Xs;
			std::array<float, 4> Ys;
			std::array<float, 4> Zs;
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

		const Vertex& vertex(const Index& index) const { return vertices_[index.face].value(index.i, index.j); }
		Vertex& vertex(const Index& index) { return vertices_[index.face].value(index.i, index.j); }
		const Chunk& chunk(const Index& index) const { return terrain_[index.face].value(index.i, index.j); }
		Chunk& chunk(const Index& index) { return terrain_[index.face].value(index.i, index.j); }

		static void genSphereVertices(SphereContainer* object);
	};
}
