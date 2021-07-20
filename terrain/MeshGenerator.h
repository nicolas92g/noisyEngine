#pragma once


//noisy
#include <Rendering/Mesh.h>
#include <configNoisy.hpp>
#include "HeightmapGenerator.h"

//stl
#include <array>
#include <memory>
#include <iostream>

namespace ns {

	class MeshGenerator
	{
	public:
		struct Result {
			std::vector<Vertex> vertices;
			std::vector<unsigned> indices;
			GLint primitiveType = GL_TRIANGLES;
			bool indexed = true;
		};

		struct Settings {
			enum class Normals {
				none,
				smooth,
				flat
			} normals = Normals::smooth;
		};

		using Input = HeightmapGenerator::Result;

	public:

		MeshGenerator(const HeightmapGenerator& heightmapGeneratorUsed, const MeshGenerator::Settings& settings = MeshGenerator::Settings());
		void operator()(const Input& heightmap, Result& result);

		//store a triangle and it's normal
		struct Triangle {
			Triangle(
				const glm::vec3 A = glm::vec3(0),
				const glm::vec3 B = glm::vec3(0),
				const glm::vec3 C = glm::vec3(0),
				const glm::vec3 normal = { 0, 1, 0 })
				:
				a(A),
				b(B),
				c(C),
				normal(normal)
			{}

			void genNormal() {
				const glm::vec3 U = b - a;
				const glm::vec3 V = c - a;

				normal.x = (U.y * V.z) - (U.z * V.y);
				normal.y = (U.z * V.x) - (U.x * V.z);
				normal.z = (U.x * V.y) - (U.y * V.x);
			}

			glm::vec3 a;
			glm::vec3 b;
			glm::vec3 c;
			glm::vec3 normal;
		};

		//store a triangle under the shape of indices and store a normal vector
		struct IndexedTriangle
		{
			IndexedTriangle(
				unsigned pAIndex = NullIndex,
				unsigned pBIndex = NullIndex,
				unsigned pCIndex = NullIndex,
				const glm::vec3 normal = { 0, 1, 0 })
				:
				a(pAIndex),
				b(pBIndex),
				c(pCIndex),
				normal(normal)
			{}

			void genNormal(const glm::vec3* arrayOfVertices) {
				const glm::vec3 U = arrayOfVertices[b] - arrayOfVertices[a];
				const glm::vec3 V = arrayOfVertices[c] - arrayOfVertices[a];

				normal.x = (U.y * V.z) - (U.z * V.y);
				normal.y = (U.z * V.x) - (U.x * V.z);
				normal.z = (U.x * V.y) - (U.y * V.x);
			}

			static constexpr unsigned NullIndex = UINT_MAX;

			unsigned a;
			unsigned b;
			unsigned c;
			glm::vec3 normal;
		};

		struct Quad {
			Quad() :
				triangle_a(),
				triangle_b()
			{}
			Quad(unsigned a, unsigned b, unsigned c, unsigned d) :
				triangle_a(a, b, c),
				triangle_b(b, d, c)
			{}
			IndexedTriangle triangle_a, triangle_b;
		};

	protected:
		const Settings settings_;
		const HeightmapGenerator::Settings heightMapSettings_;
		const HeightmapGenerator::preComputedValues data_;

	protected:
		void genFlatNormalsMesh(const MeshGenerator::Input& heightmap, Result& result, const MapSizeType& chunkPosition);
		void genSmoothNormalsMesh(const MeshGenerator::Input& heightmap, Result& result, const MapSizeType& chunkPosition);

		glm::vec3 VertexWorldPosition(const MapSizeType& chunkWorldPos, float height, int PrimitiveOffsetX, int PrimitiveOffsetY);

		void fillPositions(ns::BiArray<glm::vec3>& positions, const MeshGenerator::Input& heightmap, const MapSizeType chunkWorldPosition);
		void fillTriangles(BiArray<MeshGenerator::Quad>& quads, const ns::BiArray<glm::vec3>& positions);

		void checkNeighborsLinesXandZ(const MeshGenerator::Input& heightmap, const MapSizeType chunkWorldPosition);
	};
}





