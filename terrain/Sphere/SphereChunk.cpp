#include "SphereChunk.h"
#include <Utils/DebugLayer.h>
#include <Utils/utils.h>

ns::Sphere::SphereChunk::SphereChunk(const std::array<glm::vec3, 4>& chunkLocation, float sphereRadius, uint16_t resolution) :
	sphereRadius_(sphereRadius),
	resolution_(resolution)
{
	using namespace glm;
	BiArray<glm::vec3> grid(glm::ivec2(resolution_ + 1));

	auto location = chunkLocation;
	//normalize inputs (should be removable)
	for (size_t i = 0; i < chunkLocation.size(); i++)
		location[i] = normalize(chunkLocation[i]);

	//calculate the rotation from location[0] to location[1]
	vec3 rotAxis = cross(location[0], location[1]);
	float angleStep = -acos(dot(location[0], location[1])) / resolution_;

	for (uint16_t i = 1; i < resolution_; i++)
	{
		grid.value(i, 0) = vec4(location[0], 1.f) * rotate(angleStep * i, rotAxis);
	}
	grid.value(0, 0) = location[0];
	grid.value(resolution_, 0) = location[1];

	//calculate the rotation from location[2] to location[3]
	rotAxis = cross(location[2], location[3]);
	angleStep = -acos(dot(location[2], location[3])) / resolution_;

	for (uint16_t i = 1; i < resolution_; i++)
	{
		grid.value(i, resolution_) = vec4(location[2], 1.f) * rotate(angleStep * i, rotAxis);
	}
	grid.value(0, resolution_) = location[2];
	grid.value(resolution_, resolution_) = location[3];

	for (uint16_t i = 0; i < resolution_ + 1; i++)
	{
		const auto& a = grid.value(i, 0);
		const auto& b = grid.value(i, resolution_);

		//calculate the rotation from a to b
		rotAxis = cross(a, b);
		angleStep = -acos(dot(a, b)) / resolution_;
		
		for (uint16_t j = 1; j < resolution_; j++)
		{
			grid.value(i, j) = vec4(a, 1.f) * rotate(angleStep * j, rotAxis);
		}
	}

	//heightmap generation
	for (uint16_t i = 0; i < resolution_ + 1; i++)
	{
		for (uint16_t j = 0; j < resolution_ + 1; j++)
		{
			auto& v = grid.value(i, j);
			v *= sphereRadius_ + noise(v * 1.1f) * .02f * sphereRadius_ + noise(v * 10.f) * .01f * sphereRadius_;
		}
	}

	// mesh generation
	std::vector<Vertex> vertices((size_t)resolution_ * resolution_ * 4U);
	//std::vector<unsigned> indices((size_t)resolution_ * resolution_ * 6U);
	std::vector<unsigned> indices;

	size_t verticesCount = 0U, indicesCount = 0U;
	for (uint16_t i = 0; i < resolution_; i++)
	{
		for (uint16_t j = 0; j < resolution_; j++)
		{
			vec3 a = grid.value(i + 0, j + 0);
			vec3 b = grid.value(i + 0, j + 1);
			vec3 c = grid.value(i + 1, j + 0);
			vec3 d = grid.value(i + 1, j + 1);
			vec3 n1 = normalize(genNormal(a, b, c));
			vec3 n2 = normalize(genNormal(c, b, d));

			vertices.emplace(vertices.begin() + verticesCount + 0, a, n1);
			vertices.emplace(vertices.begin() + verticesCount + 1, b, n1);
			vertices.emplace(vertices.begin() + verticesCount + 2, c, n1);
			vertices.emplace(vertices.begin() + verticesCount + 3, c, n2);
			vertices.emplace(vertices.begin() + verticesCount + 4, b, n2);
			vertices.emplace(vertices.begin() + verticesCount + 5, d, n2);

			//indices.emplace(indices.begin() + indicesCount + 0, verticesCount + 0);
			//indices.emplace(indices.begin() + indicesCount + 1, verticesCount + 1);
			//indices.emplace(indices.begin() + indicesCount + 2, verticesCount + 2);
			//indices.emplace(indices.begin() + indicesCount + 3, verticesCount + 1);
			//indices.emplace(indices.begin() + indicesCount + 4, verticesCount + 3);
			//indices.emplace(indices.begin() + indicesCount + 5, verticesCount + 2);

			//indicesCount += 6;
			verticesCount += 6;
		}
	}

	MeshConfigInfo info;
	info.indexedVertices = false;
	mesh_ = std::make_unique<Mesh>(vertices, indices, Material(vec3(.05), .6, .9), info);
}

void ns::Sphere::SphereChunk::draw(const ns::Shader& shader) const
{
	mesh_->draw(shader);
}
