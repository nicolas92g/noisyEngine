#include "SphereContainer.h"

//stl
#include <future>
#include <mutex>

const ns::Sphere::SphereContainer::Index ns::Sphere::SphereContainer::Index::null(NULL_FACE_INDEX);
std::array<std::vector<glm::ivec2>, ns::maximunRenderDistance> ns::Sphere::SphereContainer::loadingOrder = ns::Sphere::SphereContainer::getOrder();

ns::Sphere::SphereContainer::SphereContainer(uint32_t resolution, float sphereRadius)
	:
	resolution_(resolution + resolution % 2),//resolution is forced to be an even number
	resolutionPlusOne_(resolution_ + 1),
	radius_(sphereRadius),
	//sphereThread_(&genSphereVertices, this),
	terrain_({ 
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_)),
		BiArray<SphereContainer::Chunk>(glm::ivec2(resolution_))}),
	vertices_({
		BiArray<glm::vec3>(glm::ivec2(resolutionPlusOne_)),
		BiArray<glm::vec3>(glm::ivec2(resolutionPlusOne_)),
		BiArray<glm::vec3>(glm::ivec2(resolutionPlusOne_)),
		BiArray<glm::vec3>(glm::ivec2(resolutionPlusOne_)),
		BiArray<glm::vec3>(glm::ivec2(resolutionPlusOne_)),
		BiArray<glm::vec3>(glm::ivec2(resolutionPlusOne_))
	})
{
	genSphereVertices(this);
	//sphereThread_.join();
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
				verts.emplace_back(vertex(value.coords.d));
				verts.emplace_back(vertex(value.coords.c));
				verts.emplace_back(vertex(value.coords.c));
				verts.emplace_back(vertex(value.coords.a));
				verts.emplace_back(vertex(value.coords.a));
				verts.emplace_back(vertex(value.coords.b));
			}
		}
	}
	for (uint32_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face) {

	}
	return std::make_shared<ns::Mesh>(verts, std::vector<unsigned>(), Material(glm::vec3(1), .1, .1, glm::vec3(.1)), info_);
}

std::shared_ptr<ns::Sphere::SphereChunk> ns::Sphere::SphereContainer::findChunk(const glm::vec3& normalizedVector)
{
	const auto index = find(normalizedVector);
	if (index.isNull()) return std::shared_ptr<SphereChunk>();
	return chunk(index).mesh;
}

float ns::Sphere::SphereContainer::radius() const
{
	return radius_;
}

void ns::Sphere::SphereContainer::update(const glm::vec3& direction)
{
	const Index centralChunk = find(direction);
	if (!centralChunk.isNull())
		centralChunk_ = centralChunk;
	
	for (size_t d = 0; d < loadingOrder.size(); d++)
	{
		for (size_t i = 0; i < loadingOrder[d].size(); i++)
		{
			Index index = centralChunk_;
			index.add(loadingOrder[d][i], resolution_);

			if (loadChunk(index)) return;
		}
	}


}

void ns::Sphere::SphereContainer::draw(const ns::Shader& shader) const
{
	for (const Chunk* chunk : loadedChunks_) {
		chunk->mesh->draw(shader);
	}
}

bool ns::Sphere::SphereContainer::checkCoordIsInLimit(const glm::vec3& position, const ChunkLimits& limit) const
{
	const glm::vec3 pos = glm::normalize(position) * radius_;
	return  pos.x >= limit.minX and pos.x <= limit.maxX
		and pos.y >= limit.minY and pos.y <= limit.maxY
		and pos.z >= limit.minZ and pos.z <= limit.maxZ;
}

void ns::Sphere::SphereContainer::fillChunkLimits(ChunkLimits& limit, const ChunkCoords& chunk)
{
	//put each position component in an array
	std::array<float, 4> Xs{ vertex(chunk.a).x , vertex(chunk.b).x , vertex(chunk.c).x , vertex(chunk.d).x };
	std::array<float, 4> Ys{ vertex(chunk.a).y , vertex(chunk.b).y , vertex(chunk.c).y , vertex(chunk.d).y };
	std::array<float, 4> Zs{ vertex(chunk.a).z , vertex(chunk.b).z , vertex(chunk.c).z , vertex(chunk.d).z };

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

	reg.limit.minX = std::min({ ChunkA.limit.minX, ChunkB.limit.minX, ChunkC.limit.minX, ChunkD.limit.minX }) - .1;
	reg.limit.maxX = std::max({ ChunkA.limit.maxX, ChunkB.limit.maxX, ChunkC.limit.maxX, ChunkD.limit.maxX }) + .1;
	reg.limit.minY = std::min({ ChunkA.limit.minY, ChunkB.limit.minY, ChunkC.limit.minY, ChunkD.limit.minY }) - .1;
	reg.limit.maxY = std::max({ ChunkA.limit.maxY, ChunkB.limit.maxY, ChunkC.limit.maxY, ChunkD.limit.maxY }) + .1;
	reg.limit.minZ = std::min({ ChunkA.limit.minZ, ChunkB.limit.minZ, ChunkC.limit.minZ, ChunkD.limit.minZ }) - .1;
	reg.limit.maxZ = std::max({ ChunkA.limit.maxZ, ChunkB.limit.maxZ, ChunkC.limit.maxZ, ChunkD.limit.maxZ }) + .1;

	using namespace glm;

	u16vec2 size(reg.lastChunkIndex.x - reg.firstChunkIndex.x, reg.lastChunkIndex.y - reg.firstChunkIndex.y);

	if (size.x > 3 and size.y > 3) {
		//cut a region into four sub-regions
		reg.innerRegions = std::make_shared<BiArray<ChunksRegion>>(2, 2);
		const u16vec2 middleL = reg.firstChunkIndex + size / (u16)2;
		const u16vec2 middle = middleL + (u16)1;

		reg.innerRegions->value(0, 0) = ChunksRegion(reg.firstChunkIndex, middleL);
		reg.innerRegions->value(1, 0) = ChunksRegion(u16vec2(middle.x, reg.firstChunkIndex.y), u16vec2(reg.lastChunkIndex.x, middleL.y));
		reg.innerRegions->value(0, 1) = ChunksRegion(u16vec2(reg.firstChunkIndex.x, middle.y), u16vec2(middleL.x, reg.lastChunkIndex.y));
		reg.innerRegions->value(1, 1) = ChunksRegion(middle, reg.lastChunkIndex);

		fillChunkSubRegions((*reg.innerRegions)[0], face);
		fillChunkSubRegions((*reg.innerRegions)[1], face);
		fillChunkSubRegions((*reg.innerRegions)[2], face);
		fillChunkSubRegions((*reg.innerRegions)[3], face);
	}
}

const ns::Sphere::SphereContainer::ChunksRegion& ns::Sphere::SphereContainer::findRegion(const ChunksRegion& region, const glm::vec3& pos) const
{
	if (!region.innerRegions)
		return region;

	for (uint8_t i = 0; i < 4; i++)
	{
		if (checkCoordIsInLimit(pos, (*region.innerRegions)[i].limit))
			return findRegion((*region.innerRegions)[i], pos);
	}

	return region;
}

const glm::vec3& ns::Sphere::SphereContainer::vertex(const Index& index) const
{
#	ifndef NDEBUG
	_STL_ASSERT(!index.isNull(), "try to access a null index !");
#	endif // !NDEBUG

	return vertices_[index.face].value(index.i, index.j);
}

glm::vec3& ns::Sphere::SphereContainer::vertex(const Index& index)
{
#	ifndef NDEBUG
	_STL_ASSERT(!index.isNull(), "try to access a null index !");
#	endif // !NDEBUG

	return vertices_[index.face].value(index.i, index.j);
}

const ns::Sphere::SphereContainer::Chunk& ns::Sphere::SphereContainer::chunk(const Index& index) const 
{
#	ifndef NDEBUG
	_STL_ASSERT(!index.isNull(), "try to access a null index !");
#	endif // !NDEBUG

	return terrain_[index.face].value(index.i, index.j);
}

ns::Sphere::SphereContainer::Chunk& ns::Sphere::SphereContainer::chunk(const ns::Sphere::SphereContainer::Index& index) 
{
#	ifndef NDEBUG
	_STL_ASSERT(!index.isNull(), "try to access a null index !");
#	endif // !NDEBUG

	return terrain_[index.face].value(index.i, index.j);
}

bool ns::Sphere::SphereContainer::isLoaded(const Index& index) const
{
	return chunk(index).mesh.operator bool();
}

ns::Sphere::SphereContainer::Index ns::Sphere::SphereContainer::find(const glm::vec3& position) const
{
#	ifndef NDEBUG
	auto l = glm::length(position);
	_STL_ASSERT(std::abs(1.f - l) < .1f, ("the vec3 inserted into SphereContainer::find(vec3) was not normalized ! l " + std::to_string(l)).c_str());
#	endif // !NDEBUG

	static std::vector<uint8_t> faces(6);
	faces.clear();

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
					return Index(face, i, j);
				}
			}
		}
	}
	return Index::null;
}

ns::Sphere::SphereContainer::Index ns::Sphere::SphereContainer::find(const Index& previousIndex, const glm::vec3& normalizedVector) const
{
	// TODO : will be easier to implement when the chunk loading system will be done
	_STL_REPORT_ERROR("not implemented yet !");
	return Index();
}

bool ns::Sphere::SphereContainer::loadChunk(const Index& index)
{
	if (!isLoaded(index)) {
		auto& c = chunk(index);

		std::array<glm::vec3, 4> square{
			vertex(c.coords.a),
			vertex(c.coords.b),
			vertex(c.coords.c),
			vertex(c.coords.d)
		};

		c.mesh = std::make_shared<SphereChunk>(square, radius_, 20);
		c.index = index;
		loadedChunks_.push_back(&c);
		return true;
	}
	return false;
}

bool ns::Sphere::SphereContainer::unloadChunk(const Index& index)
{
	if (isLoaded(index)) {
		bool founded = false;
		for (auto it = loadedChunks_.begin(); it != loadedChunks_.end(); ++it)
		{
			if (*it == &chunk(index)) {
				loadedChunks_.erase(it);
				founded = true;
				break;
			}
		}
		
		chunk(index).mesh.reset();
		return true;
	}
	return false;
}

//this create the sphere terrain grid vertices
void ns::Sphere::SphereContainer::genSphereVertices(SphereContainer* object)
{
	using namespace glm;
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
	//not used but may be useful in the futur i don't know
	static const vec3 normals[NUMBER_OF_FACES_IN_A_CUBE]
	{
		vec3(0.0, 0.0, -1.0),
		vec3(1.0, 0.0 ,0.0),
		vec3(0.0, 0.0, 1.0),
		vec3(-1.0, 0.0, 0.0),
		vec3(0.0, 1.0 ,0.0),
		vec3(0.0, -1.0, 0.0)
	};

	Timer t("generate spherified cube");

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
			//std::cout << object->sphereProgressionPercentage() << " %" << newl;
		}
	}

	const uint32_t k = object->resolutionPlusOne_;
	for (uint32_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face)
	{
		for (uint32_t j = 0; j < object->resolution_; ++j)
		{
			for (uint32_t i = 0; i < object->resolution_; ++i)
			{
				auto& const value = object->terrain_[face].value(i, j);
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
				//object->loadChunk(Index(face, i, j));
			}
		}
	}

	for (uint8_t face = 0; face < NUMBER_OF_FACES_IN_A_CUBE; ++face) {
		//init first sub-region
		auto& region = object->subRegions[face];

		region.firstChunkIndex = glm::u16vec2(0);
		region.lastChunkIndex = glm::u16vec2(object->resolution_ - 1);

		//recursively create all the sub-regions
		object->fillChunkSubRegions(region, face);
		//logRegion(region);
	}
}

std::array<std::vector<glm::ivec2>, ns::maximunRenderDistance> ns::Sphere::SphereContainer::getOrder()
{
	//(dst, dst) / (-1, 0) * dst * 2 / (0, -1) * dst * 2 / (1, 0) * dst * 2 / (0, 1) * (dst * 2 - 1)
	std::array<std::vector<glm::ivec2>, maximunRenderDistance> ret;

	//central chunk offset is of course null
	ret[0].push_back(glm::ivec2(0, 0));

	for (size_t dst = 1; dst < maximunRenderDistance; dst++)
	{
		std::vector<glm::ivec2>& circle = ret[dst];
		circle.resize(8 * dst);

		//start by the chunk that is at ivec2(dst - 1, dst)
		circle[0] = glm::ivec2(dst - 1, dst);
		unsigned cursor = 1;

		//then go to the top-left corner
		for (size_t i = 0; i < dst * 2 - 1; i++)
		{
			circle[cursor] = circle[cursor - 1] + glm::ivec2(-1, 0);
			cursor++;
		}
		//then to the bottom-left corner
		for (size_t i = 0; i < dst * 2; i++)
		{
			circle[cursor] = circle[cursor - 1] + glm::ivec2(0, -1);
			cursor++;
		}
		//then to the bottom-right corner
		for (size_t i = 0; i < dst * 2; i++)
		{
			circle[cursor] = circle[cursor - 1] + glm::ivec2(1, 0);
			cursor++;
		}
		//then just under the start at ivec2(dst, dst - 1)
		for (size_t i = 0; i < dst * 2; i++)
		{
			circle[cursor] = circle[cursor - 1] + glm::ivec2(0, 1);
			cursor++;
		}
		//and all the chunks in the square that has a size of dst * 2 + 1 are in the array at the index dst
	}
	return ret;
}

void ns::Sphere::SphereContainer::Index::add(glm::ivec2 offset, uint32_t resolution)
{
	using namespace glm;

	ivec2 pos = ivec2(i, j) + offset;

	bool left = pos.x < 0;
	bool right = pos.x >= resolution;
	bool bottom = pos.y < 0;
	bool top = pos.y >= resolution;

	if (!(left or right or bottom or top)) {
		i = pos.x;
		j = pos.y;
		return;
	}

	switch (face) {
		case 0:
			if (left) {
				face = 3;
				add(offset + glm::ivec2(resolution, 0), resolution);
			}
			else if (right) {
				face = 1;
				add(offset - glm::ivec2(resolution, 0), resolution);
			}
			if (bottom) {
				face = 5;
				add(offset + glm::ivec2(0, resolution), resolution);
			}
			else if (top) {
				face = 4;
				add(offset - glm::ivec2(0, resolution), resolution);
			}

			break;
		case 1:
			if (left) {
				face = 0;
				add(offset + glm::ivec2(resolution, 0), resolution);
			}
			else if (right) {
				face = 2;
				add(offset - glm::ivec2(resolution, 0), resolution);
			}
			if (bottom) {
				face = 5;
				std::swap(i, j);
				add(offset + glm::ivec2(resolution, 0), resolution);
			}
			else if (top) {
				face = 4;
				std::swap(i, j);
				add(offset - glm::ivec2(resolution, 0), resolution);
			}
			break;
		case 2:

			break;
		case 3:

			break;
		case 4:

			break;
		case 5:

			break;
		default:
			break;
	}
}
