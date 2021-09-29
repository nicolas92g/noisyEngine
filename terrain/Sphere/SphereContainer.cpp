#include "SphereContainer.h"

//stl
#include <future>
#include <mutex>

ns::Sphere::SphereContainer::SphereContainer(uint32_t resolution, float sphereRadius)
	:
	resolution_(resolution),
	resolutionPlusOne_(resolution_ + 1),
	radius_(sphereRadius),
	sphereThread_(&genSphereVertices, this),
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
	info_.indexedVertices = false;
	info_.primitive = GL_LINES;
	std::vector<ns::Vertex> verts;
	verts.reserve((size_t)NUMBER_OF_FACES_IN_A_CUBE * resolution_ * resolution_ * 6);

	for (uint32_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face)
	{
		for (uint32_t j = 0; j < resolution_; ++j)
		{
			for (uint32_t i = 0; i < resolution_; ++i)
			{
				auto& value = terrain_[face].value(i, j);
				verts.emplace_back(vertex(value.coords.d).position);
				verts.emplace_back(vertex(value.coords.c).position);
				verts.emplace_back(vertex(value.coords.c).position);
				verts.emplace_back(vertex(value.coords.a).position);
				verts.emplace_back(vertex(value.coords.a).position);
				verts.emplace_back(vertex(value.coords.b).position);
			}
		}
	}

	return std::make_shared<ns::Mesh>(verts, std::vector<unsigned>(), Material(glm::vec3(1), .1, .1, glm::vec3(.1)), info_);
}

std::shared_ptr<ns::Sphere::SphereChunk> ns::Sphere::SphereContainer::chunk(glm::vec2 angles)
{
	return std::shared_ptr<ns::Sphere::SphereChunk>();
}

bool ns::Sphere::SphereContainer::checkCoordIsInChunk(glm::vec3 pos, const Chunk& chunk)
{
	pos = glm::normalize(pos) * radius_;
	return  pos.x >= chunk.Xs[0] and pos.x <= chunk.Xs[3] 
		and pos.y >= chunk.Ys[0] and pos.y <= chunk.Ys[3] 
		and pos.z >= chunk.Zs[0] and pos.z <= chunk.Zs[3];
}

//this create the sphere terrain grid vertices
void ns::Sphere::SphereContainer::genSphereVertices(SphereContainer* object)
{
	using namespace glm;
	Timer t("generate spherified cube");
	static const vec3 origins[NUMBER_OF_FACES_IN_A_CUBE]
	{
		vec3(-1.0, -1.0, -1.0),
		vec3(1.0, -1.0, -1.0),
		vec3(1.0, -1.0, 1.0),
		vec3(-1.0, -1.0, 1.0),
		vec3(-1.0, 1.0, -1.0),
		vec3(-1.0, -1.0, 1.0)
	};
	static const vec3 rights[NUMBER_OF_FACES_IN_A_CUBE]
	{
		vec3(2.0, 0.0, 0.0),
		vec3(0.0, 0.0, 2.0),
		vec3(-2.0, 0.0, 0.0),
		vec3(0.0, 0.0, -2.0),
		vec3(2.0, 0.0, 0.0),
		vec3(2.0, 0.0, 0.0)
	};
	static const vec3 ups[NUMBER_OF_FACES_IN_A_CUBE]
	{
		vec3(0.0, 2.0, 0.0),
		vec3(0.0, 2.0, 0.0),
		vec3(0.0, 2.0, 0.0),
		vec3(0.0, 2.0, 0.0),
		vec3(0.0, 0.0, 2.0),
		vec3(0.0, 0.0, -2.0)
	};
	static const vec3 normals[NUMBER_OF_FACES_IN_A_CUBE]
	{
		vec3(0.0, 0.0, -1.0),
		vec3(1.0, 0.0 ,0.0),
		vec3(0.0, 0.0, 1.0),
		vec3(-1.0, 0.0, 0.0),
		vec3(0.0, 1.0 ,0.0),
		vec3(0.0, -1.0, 0.0)
	};

	const float step = 1.f / (float)object->resolution_;
	object->sphereProgression_ = 0;

	for (uint8_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; face++)
	{
		//std::cout << "face " << (int)face << std::endl;
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

				object->vertices_[face].emplace(i, j, { n * object->radius_ });

				object->sphereProgression_++;
			}
			std::cout << object->sphereProgressionPercentage() << " %" << newl;
		}
	}

	const uint32_t k = object->resolutionPlusOne_;
	for (uint32_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face)
	{
		for (uint32_t j = 0; j < object->resolution_; ++j)
		{
			for (uint32_t i = 0; i < object->resolution_; ++i)
			{
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

				const auto& chunk = value.coords;

				value.Xs = { object->vertex(chunk.a).position.x , object->vertex(chunk.b).position.x , object->vertex(chunk.c).position.x , object->vertex(chunk.d).position.x };
				value.Ys = { object->vertex(chunk.a).position.y , object->vertex(chunk.b).position.y , object->vertex(chunk.c).position.y , object->vertex(chunk.d).position.y };
				value.Zs = { object->vertex(chunk.a).position.z , object->vertex(chunk.b).position.z , object->vertex(chunk.c).position.z , object->vertex(chunk.d).position.z };

				std::sort(value.Xs.begin(), value.Xs.end());
				std::sort(value.Ys.begin(), value.Ys.end());
				std::sort(value.Zs.begin(), value.Zs.end());
			}
		}
	}
}
