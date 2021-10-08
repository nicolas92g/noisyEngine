#include "SphereChunk.h"
#include <Utils/DebugLayer.h>

ns::Sphere::SphereChunk::SphereChunk(const std::array<glm::vec3, 4>& chunkLocation, float sphereRadius, uint16_t resolution) :
	sphereRadius_(sphereRadius),
	resolution_(resolution),
	grid_(glm::ivec2(resolution_ + 1))
{
	using namespace glm;

	auto location_ = chunkLocation;
	//normalize inputs (should be removable)
	for (size_t i = 0; i < chunkLocation.size(); i++)
		location_[i] = normalize(chunkLocation[i]);

	//calculate the rotation axis to find all the points in one direction that goes toward location_[1]
	const vec3 rotAxisA = cross(location_[0], location_[1]);
	const float angleStepA = dot(location_[0], location_[1]) / resolution_;

	//calculate the rotation axis to find all the points in one direction that goes toward location_[2]
	const vec3 rotAxisB = cross(location_[0], location_[2]);
	const float angleStepB = dot(location_[0], location_[2]) / resolution_;



	for (uint16_t i = 0; i < resolution_ + 1; i++)
	{
		for (uint16_t j = 0; j < resolution_ + 1; j++)
		{
			grid_.value(i, j) = vec4(location_.front(), 1.f) * rotate(angleStepA * i, rotAxisA) * rotate(angleStepB * i, rotAxisB);
		}
	}

	std::vector<Vertex> vertices; vertices.reserve((size_t)resolution_ * resolution_ * 6U);
	std::vector<unsigned> indices;

	for (uint16_t i = 0; i < resolution_; i++)
	{
		for (uint16_t j = 0; j < resolution_; j++)
		{
			vertices.emplace_back(grid_.value(i + 0, j + 0));
			vertices.emplace_back(grid_.value(i + 1, j + 0));
			vertices.emplace_back(grid_.value(i + 0, j + 1));
			vertices.emplace_back(grid_.value(i + 1, j + 0));
			vertices.emplace_back(grid_.value(i + 0, j + 1));
			vertices.emplace_back(grid_.value(i + 1, j + 1));
		}
	}

	MeshConfigInfo info;
	info.indexedVertices = false;
	mesh_ = std::make_unique<Mesh>(vertices, indices);
}

void ns::Sphere::SphereChunk::draw(const ns::Shader& shader) const
{
	mesh_->draw(shader);
}
