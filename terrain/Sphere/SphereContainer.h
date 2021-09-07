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

		void test(const glm::vec3& position) {
			for (size_t face = 0; face < 6; face++)
			{
				for (size_t i = 0; i < terrain_[face].x(); i++)
				{
					for (size_t j = 0; j < terrain_[face].y(); j++)
					{
						//checkCoordIsInChunk(, terrain_[face].value(i, j).coords);
						
					}
				}
			}
		}
		
	protected:
		const uint32_t resolution_;
		const uint32_t resolutionPlusOne_;
		const float radius_;

		std::thread sphereThread_;
		std::atomic_uint32_t sphereProgression_;

		struct Vertex {
			glm::vec3 position;
			glm::vec2 angles;
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
		};

	
		std::array<ns::BiArray<Chunk>, NUMBER_OF_FACES_IN_A_CUBE> terrain_;
		std::array<ns::BiArray<Vertex>, NUMBER_OF_FACES_IN_A_CUBE> vertices_;


	protected:
		static glm::vec2 getAngle(const glm::vec3& localSpherePosition);
		bool checkCoordIsInChunk(glm::vec2 coords, const ChunkCoords& chunk);
		const Vertex& vertex(const Index& index);
		static void genSphereVertices(SphereContainer* object);
	};
}
