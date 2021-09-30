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
	return  pos.x >= chunk.limit.minX and pos.x < chunk.limit.maxX
		and pos.y >= chunk.limit.minY and pos.y < chunk.limit.maxY
		and pos.z >= chunk.limit.minZ and pos.z < chunk.limit.maxZ;
}

void ns::Sphere::SphereContainer::fillChunkLimits(ChunkLimits& limit, const ChunkCoords& chunk)
{
	//put each position component in an array
	std::array<float, 4> Xs{ vertex(chunk.a).position.x , vertex(chunk.b).position.x , vertex(chunk.c).position.x , vertex(chunk.d).position.x };
	std::array<float, 4> Ys{ vertex(chunk.a).position.y , vertex(chunk.b).position.y , vertex(chunk.c).position.y , vertex(chunk.d).position.y };
	std::array<float, 4> Zs{ vertex(chunk.a).position.z , vertex(chunk.b).position.z , vertex(chunk.c).position.z , vertex(chunk.d).position.z };

	//sort those arrays
	std::sort(Xs.begin(), Xs.end());
	std::sort(Ys.begin(), Ys.end());
	std::sort(Zs.begin(), Zs.end());

	//store the maximun and minimun components
	limit.maxX = Xs[3];
	limit.minX = Xs[0];
	limit.maxY = Ys[3];
	limit.minY = Ys[0];
	limit.maxZ = Zs[3];
	limit.minZ = Zs[0];
}

void ns::Sphere::SphereContainer::fillChunkSubRegions(ChunksRegion& reg, uint8_t face)
{
	const Chunk& ChunkA = chunk(Index(face, reg.firstChunkIndex.x, reg.firstChunkIndex.y));
	const Chunk& ChunkB = chunk(Index(face, reg.firstChunkIndex.x, reg.lastChunkIndex.y));
	const Chunk& ChunkC = chunk(Index(face, reg.lastChunkIndex.x, reg.firstChunkIndex.y));
	const Chunk& ChunkD = chunk(Index(face, reg.lastChunkIndex.x, reg.lastChunkIndex.y));

	reg.limit.minX = std::min({ ChunkA.limit.minX, ChunkB.limit.minX, ChunkC.limit.minX, ChunkD.limit.minX });
	reg.limit.maxX = std::max({ ChunkA.limit.maxX, ChunkB.limit.maxX, ChunkC.limit.maxX, ChunkD.limit.maxX });
	reg.limit.minY = std::min({ ChunkA.limit.minY, ChunkB.limit.minY, ChunkC.limit.minY, ChunkD.limit.minY });
	reg.limit.maxY = std::max({ ChunkA.limit.maxY, ChunkB.limit.maxY, ChunkC.limit.maxY, ChunkD.limit.maxY });
	reg.limit.minZ = std::min({ ChunkA.limit.minZ, ChunkB.limit.minZ, ChunkC.limit.minZ, ChunkD.limit.minZ });
	reg.limit.maxZ = std::max({ ChunkA.limit.maxZ, ChunkB.limit.maxZ, ChunkC.limit.maxZ, ChunkD.limit.maxZ });

	using namespace glm;

	u16vec2 size(reg.lastChunkIndex.x - reg.firstChunkIndex.x, reg.lastChunkIndex.y - reg.firstChunkIndex.y);

	if (size.x > 3 and size.y > 3) {

		reg.innerRegions.emplace(2, 2);
		const u16vec2 middle = reg.firstChunkIndex + size / (u16)2;
		const u16vec2 middleL = middle - (u16)1;

		reg.innerRegions.value().emplace(0, 0, ChunksRegion(reg.firstChunkIndex, middleL));
		reg.innerRegions.value().emplace(1, 0, ChunksRegion(u16vec2(middle.x, reg.firstChunkIndex.y), u16vec2(reg.lastChunkIndex.x, middleL.y)));
		reg.innerRegions.value().emplace(0, 1, ChunksRegion(u16vec2(reg.firstChunkIndex.x, middle.y), u16vec2(middleL.x, reg.lastChunkIndex.y)));
		reg.innerRegions.value().emplace(1, 1, ChunksRegion(middle, reg.lastChunkIndex));

		fillChunkSubRegions(reg.innerRegions.value()[0], face);
		fillChunkSubRegions(reg.innerRegions.value()[1], face);
		fillChunkSubRegions(reg.innerRegions.value()[2], face);
		fillChunkSubRegions(reg.innerRegions.value()[3], face);
	}
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

				if(j == 0)
					std::cout << to_string(n) << ",\n";

				object->vertices_[face].emplace(i, j, { n * object->radius_ });
				object->sphereProgression_++;
			}
			std::cout << newl;
			//std::cout << object->sphereProgressionPercentage() << " %" << newl;
		}
		std::cout << "\n----\n\n";
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

				object->fillChunkLimits(value.limit, value.coords);
			}
		}
	}
}
