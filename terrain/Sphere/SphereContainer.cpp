#include "SphereContainer.h"

//stl
#include <future>
#include <mutex>

ns::Sphere::SphereContainer::SphereContainer(uint32_t resolution, float sphereRadius)
	:
	resolution_(resolution),
	resolutionPlusOne_(resolution_ + 1),
	radius_(sphereRadius),
	sphereThread_(genSphereVertices, this),
	terrain_({ 
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_))}),
	vertices_({
		BiArray<SphereContainer::Vertex>(glm::ivec2(resolutionPlusOne_)),
		BiArray<SphereContainer::Vertex>(glm::ivec2(resolutionPlusOne_)),
		BiArray<SphereContainer::Vertex>(glm::ivec2(resolutionPlusOne_)),
		BiArray<SphereContainer::Vertex>(glm::ivec2(resolutionPlusOne_)),
		BiArray<SphereContainer::Vertex>(glm::ivec2(resolutionPlusOne_)),
		BiArray<SphereContainer::Vertex>(glm::ivec2(resolutionPlusOne_))
	})
{
	sphereThread_.join();
}

double ns::Sphere::SphereContainer::sphereProgressionPercentage() const
{
	return ((double)sphereProgression_ / ((double)resolutionPlusOne_ * (double)resolutionPlusOne_ * (double)NUMBER_OF_FACES_IN_A_CUBE)) * 100.0;
}

std::shared_ptr<ns::Mesh> ns::Sphere::SphereContainer::getDebugSphere() const
{
	MeshConfigInfo info_;
	info_.indexedVertices = true;
	info_.primitive = GL_LINES;
	std::vector<ns::Vertex> verts((size_t)resolutionPlusOne_ * resolutionPlusOne_ * NUMBER_OF_FACES_IN_A_CUBE);
	std::vector<uint32_t> inds;
	inds.reserve((size_t)NUMBER_OF_FACES_IN_A_CUBE * resolution_ * resolution_);

	const uint32_t k = resolutionPlusOne_;
	for (uint32_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face)
	{
		for (uint32_t j = 0; j < resolution_; ++j)
		{
			for (uint32_t i = 0; i < resolution_; ++i)
			{
				const uint32_t a = (face * k + j) * k + i;
				const uint32_t b = (face * k + j) * k + i + 1;
				const uint32_t c = (face * k + j + 1) * k + i;
				const uint32_t d = (face * k + j + 1) * k + i + 1;

				inds.emplace_back(b);
				inds.emplace_back(a);
				inds.emplace_back(c);
				inds.emplace_back(d);
				inds.emplace_back(d);
				inds.emplace_back(b);

			}
		}
	}

	size_t cursor = 0;
	for (size_t face = 0; face < 6; face++)
	{
		for (size_t i = 0; i < resolutionPlusOne_; i++)
		{
			for (size_t j = 0; j < resolutionPlusOne_; j++)
			{
				verts[cursor].position = vertices_[face].value(i, j).position;
				verts[cursor].normal = vertices_[face].value(i, j).position;
				cursor++;
			}
		}
	}

	return std::make_shared<ns::Mesh>(verts, inds, Material::getDefault(), info_);
}

std::shared_ptr<ns::Sphere::SphereChunk> ns::Sphere::SphereContainer::chunk(glm::vec2 angles)
{
	return std::shared_ptr<ns::Sphere::SphereChunk>();
}


glm::vec2 ns::Sphere::SphereContainer::getAngle(const glm::vec3& localSpherePosition)
{
	return glm::vec2();
}

bool ns::Sphere::SphereContainer::checkCoordIsInChunk(glm::vec2 coords, const ChunkCoords& chunk)
{
	const glm::vec2 min(std::min(vertex(chunk.a).angles.x, vertex(chunk.d).angles.x), std::min(vertex(chunk.a).angles.y, vertex(chunk.d).angles.y));
	const glm::vec2 max(std::max(vertex(chunk.a).angles.x, vertex(chunk.d).angles.x), std::max(vertex(chunk.a).angles.y, vertex(chunk.d).angles.y));
	return coords.x >= min.x and coords.x <= max.x and coords.y >= min.y and coords.y <= max.y;
}

const ns::Sphere::SphereContainer::Vertex& ns::Sphere::SphereContainer::vertex(const Index& index)
{
	return vertices_[index.face].value(index.i, index.j);
}

void ns::Sphere::SphereContainer::genSphereVertices(SphereContainer* object)
{
	Timer t("generate spherified cube");
	static constexpr glm::vec3 origins[NUMBER_OF_FACES_IN_A_CUBE] =
	{
		glm::vec3(-1.0, -1.0, -1.0),
		glm::vec3(1.0, -1.0, -1.0),
		glm::vec3(1.0, -1.0, 1.0),
		glm::vec3(-1.0, -1.0, 1.0),
		glm::vec3(-1.0, 1.0, -1.0),
		glm::vec3(-1.0, -1.0, 1.0)
	};
	static constexpr glm::vec3 rights[NUMBER_OF_FACES_IN_A_CUBE] =
	{
		glm::vec3(2.0, 0.0, 0.0),
		glm::vec3(0.0, 0.0, 2.0),
		glm::vec3(-2.0, 0.0, 0.0),
		glm::vec3(0.0, 0.0, -2.0),
		glm::vec3(2.0, 0.0, 0.0),
		glm::vec3(2.0, 0.0, 0.0)
	};
	static constexpr glm::vec3 ups[NUMBER_OF_FACES_IN_A_CUBE] =
	{
		glm::vec3(0.0, 2.0, 0.0),
		glm::vec3(0.0, 2.0, 0.0),
		glm::vec3(0.0, 2.0, 0.0),
		glm::vec3(0.0, 2.0, 0.0),
		glm::vec3(0.0, 0.0, 2.0),
		glm::vec3(0.0, 0.0, -2.0)
	};

	const float step = 1.f / (float)object->resolution_;
	object->sphereProgression_ = 0;

	for (uint8_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; face++)
	{
		std::cout << "face " << (int)face << std::endl;
		for (uint32_t j = 0; j < object->resolutionPlusOne_; j++)
		{
			const glm::vec3 jup = (float)j * ups[face];

			for (uint32_t i = 0; i < object->resolutionPlusOne_; i++)
			{
				const glm::vec3 p = origins[face] + step * ((float)i * rights[face] + jup);
				const glm::vec3 p2 = p * p;
				const glm::vec3 n(
					p.x * std::sqrt(1.0f - 0.5f * (p2.y + p2.z) + p2.y * p2.z / 3.0f),
					p.y * std::sqrt(1.0f - 0.5f * (p2.z + p2.x) + p2.z * p2.x / 3.0f),
					p.z * std::sqrt(1.0f - 0.5f * (p2.x + p2.y) + p2.x * p2.y / 3.0f)
				);

				object->vertices_[face].emplace(i, j, Vertex(n * object->radius_, 
					glm::vec2(glm::degrees(glm::acos(glm::dot(n, glm::vec3(1, 0, 0)))),
					glm::degrees(glm::acos(glm::dot(n, glm::vec3(0, 0, 1)))))));

				std::cout << to_string(object->vertices_[face].value(i, j).angles) << "\n";

				object->sphereProgression_++;
			}
		}
	}

	const uint32_t k = object->resolutionPlusOne_;
	for (uint32_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face)
	{
		for (uint32_t j = 0; j < object->resolution_; ++j)
		{
			for (uint32_t i = 0; i < object->resolution_; ++i)
			{
				//const uint32_t a = (face * k + j) * k + i;
				//const uint32_t b = (face * k + j) * k + i + 1;
				//const uint32_t c = (face * k + j + 1) * k + i;
				//const uint32_t d = (face * k + j + 1) * k + i + 1;

				auto& value = object->terrain_[face].value(i, j);
				value.coords.a.face = face;
				value.coords.a.i = i;
				value.coords.a.j = j;
			
				value.coords.b.face = face;
				value.coords.b.i = i + 1;
				value.coords.b.j = j;
				
				value.coords.c.face = face;
				value.coords.c.i = i;
				value.coords.c.j = j + 1;
				
				value.coords.d.face = face;
				value.coords.d.i = i + 1;
				value.coords.d.j = j + 1;
			}
		}
	}
}
