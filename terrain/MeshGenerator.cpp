#include "MeshGenerator.h"
#include <Utils/DebugLayer.h>

#include <Utils/BiArray.h>

ns::MeshGenerator::MeshGenerator(const HeightmapGenerator& heightGen, const MeshGenerator::Settings& settings)
	:
	settings_(settings),
	data_(heightGen.data_),
	heightMapSettings_(heightGen.settings_)
{}

void ns::MeshGenerator::operator()(const MeshGenerator::Input& heightmap, Result& result)
{
	const MapSizeType chunkWorldPosition = heightMapSettings_.chunkPhysicalSize * (MapSizeType)heightmap.chunk;

	if(settings_.normals == Settings::Normals::flat)
		genFlatNormalsMesh(heightmap, result, chunkWorldPosition);
	else if (settings_.normals == Settings::Normals::smooth)
		genSmoothNormalsMesh(heightmap, result, chunkWorldPosition);
}

void ns::MeshGenerator::genFlatNormalsMesh(const MeshGenerator::Input& heightmap, Result& result, const MapSizeType& chunkPosition)
{

	result.indexed = false;

	result.vertices.resize(heightMapSettings_.numberOfPartitions.x * heightMapSettings_.numberOfPartitions.y * 6);

	size_t verticesIndex = 0;
	for (size_t i = 0; i < heightMapSettings_.numberOfPartitions.x; i++) {

		for (size_t j = 0; j < heightMapSettings_.numberOfPartitions.y; j++) {

			const size_t heightMapWidth = heightMapSettings_.numberOfPartitions.x + 1;
			const glm::vec3 a(VertexWorldPosition(chunkPosition, heightmap.values[TWO_DIM((i + 0), (j + 0), heightMapWidth)], i + 0, j + 0));
			const glm::vec3 b(VertexWorldPosition(chunkPosition, heightmap.values[TWO_DIM((i + 1), (j + 0), heightMapWidth)], i + 1, j + 0));
			const glm::vec3 c(VertexWorldPosition(chunkPosition, heightmap.values[TWO_DIM((i + 0), (j + 1), heightMapWidth)], i + 0, j + 1));
			const glm::vec3 d(VertexWorldPosition(chunkPosition, heightmap.values[TWO_DIM((i + 1), (j + 1), heightMapWidth)], i + 1, j + 1));
			
			Triangle triangle(a, b, c);
			triangle.genNormal();

			result.vertices.emplace(result.vertices.begin() + verticesIndex + 0, a, triangle.normal);
			result.vertices.emplace(result.vertices.begin() + verticesIndex + 1, b, triangle.normal);
			result.vertices.emplace(result.vertices.begin() + verticesIndex + 2, c, triangle.normal);

			triangle = Triangle(c, b, d);
			triangle.genNormal();

			result.vertices.emplace(result.vertices.begin() + verticesIndex + 3, c, triangle.normal);
			result.vertices.emplace(result.vertices.begin() + verticesIndex + 4, b, triangle.normal);
			result.vertices.emplace(result.vertices.begin() + verticesIndex + 5, d, triangle.normal);

			verticesIndex += 6;
		}
	}
}

void ns::MeshGenerator::genSmoothNormalsMesh(const MeshGenerator::Input& heightmap, Result& result, const MapSizeType& chunkPosition)
{
	result.primitiveType = GL_LINE_STRIP;
	result.primitiveType = GL_TRIANGLES;
	result.indexed = true;
	
	const glm::ivec2 size(heightMapSettings_.numberOfPartitions.x + 1, heightMapSettings_.numberOfPartitions.y + 1);

	BiArray<glm::vec3> positions(size + glm::ivec2(2), glm::vec3(1));
	BiArray<MeshGenerator::Quad> quads(size + glm::ivec2(1), MeshGenerator::Quad(0, 0, 0, 0));
	BiArray<glm::vec3> normals(size, glm::vec3(1));

	for (auto& value : positions) {
		value = glm::vec3(-1);
	}

	fillPositions(positions, heightmap, chunkPosition);
	fillTriangles(quads, positions);

	result.vertices.resize((size_t)size.x * size.y);
	for (size_t j = 0; j < size.y; j++)
	{
		for (size_t i = 0; i < size.x; i++) 
		{
			result.vertices.emplace(result.vertices.begin() + TWO_DIM(i,j, size.x), positions.value(i + 1, j + 1));
		}
	}

	result.indices.resize(((size_t)size.x - 1) * ((size_t)size.y - 1) * 6);

	for (int j = 0; j < size.y - 1; j++)
	{
		for (int i = 0; i < size.x - 1; i++)
		{
			const int index = TWO_DIM(i, j, size.x) * 6;

			result.indices.emplace(result.indices.begin() + index + 0, TWO_DIM(i + 0, j + 0, size.x));
			result.indices.emplace(result.indices.begin() + index + 1, TWO_DIM(i + 1, j + 0, size.x));
			result.indices.emplace(result.indices.begin() + index + 2, TWO_DIM(i + 0, j + 1, size.x));
			result.indices.emplace(result.indices.begin() + index + 3, TWO_DIM(i + 1, j + 0, size.x));
			result.indices.emplace(result.indices.begin() + index + 4, TWO_DIM(i + 1, j + 1, size.x));
			result.indices.emplace(result.indices.begin() + index + 5, TWO_DIM(i + 0, j + 1, size.x));
		}
	}




	/*for (size_t j = 0; j < quads.y(); j++)
	{
		for (size_t i = 0; i < quads.x(); i++)
		{
			if ((i == 0 and j == 0)) continue;
			if ((i == 0 and j == quads.y() - 1)) continue;
			if ((i == quads.x() - 1 and j == 0)) continue;
			if ((i == quads.x() - 1 and j == quads.y() - 1)) continue;

			result.vertices.emplace_back(positions[quads.value(i, j).triangle_a.a], quads.value(i, j).triangle_a.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_a.b], quads.value(i, j).triangle_a.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_a.b], quads.value(i, j).triangle_a.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_a.c], quads.value(i, j).triangle_a.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_a.c], quads.value(i, j).triangle_a.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_a.a], quads.value(i, j).triangle_a.normal);

			
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_b.a], quads.value(i, j).triangle_b.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_b.b], quads.value(i, j).triangle_b.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_b.b], quads.value(i, j).triangle_b.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_b.c], quads.value(i, j).triangle_b.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_b.c], quads.value(i, j).triangle_b.normal);
			result.vertices.emplace_back(positions[quads.value(i, j).triangle_b.a], quads.value(i, j).triangle_b.normal);

		}
	}*/

	for (size_t i = 0; i < positions.x(); i++)
	{
		Debug::get() << i << " : \t";
		for (size_t j = 0; j < positions.y(); j++)
		{
			Debug::get() << to_string((glm::ivec3)positions.value(i, j)) << "\t\t";
		}
		Debug::get() << '\n';
	}
	Debug::get().log();
}

glm::vec3 ns::MeshGenerator::VertexWorldPosition(const MapSizeType& chunkPos, float height, int x, int y)
{
	return glm::vec3( chunkPos.x + x * data_.primitiveSize.x, height, chunkPos.y + y * data_.primitiveSize.y);
}

void ns::MeshGenerator::fillPositions(ns::BiArray<glm::vec3>& positions, const MeshGenerator::Input& heightmap, const MapSizeType chunkPosition)
{
	const glm::ivec2 size(heightMapSettings_.numberOfPartitions.x + 1, heightMapSettings_.numberOfPartitions.y + 1);

	for (size_t i = 0; i < size.x; i++)
	{
		for (size_t j = 0; j < size.y; j++)
		{
			positions.value(i + 1, j + 1) = VertexWorldPosition(chunkPosition, heightmap.values[TWO_DIM(i, j, size.x)], i, j);
		}
	}

	checkNeighborsLinesXandZ(heightmap, chunkPosition);

	for (size_t i = 0; i < size.x; i++) {
		positions.value(i + 1, 0) = heightmap.neighbors[Input::bottom]->positions[i];
		positions.value(i + 1, positions.y() - 1) = heightmap.neighbors[Input::top]->positions[i];
	}

	for (size_t j = 0; j < size.y; j++) {
		positions.value(0, j + 1) = heightmap.neighbors[Input::left]->positions[j];
		positions.value(positions.x() - 1, j + 1) = heightmap.neighbors[Input::right]->positions[j];
	}
}

void ns::MeshGenerator::fillTriangles(BiArray<MeshGenerator::Quad>& quads, const ns::BiArray<glm::vec3>& positions)
{
#	ifndef NDEBUG
	//check triangles array size
	_STL_VERIFY(quads.x() == positions.x() - 1, "fillTriangles() error : quad array has an invalid size !");
	_STL_VERIFY(quads.y() == positions.y() - 1, "fillTriangles() error : quad array has an invalid size !");
#	endif // !NDEBUG

	for (size_t j = 0; j < quads.y(); j++)
	{
		for (size_t i = 0; i < quads.x(); i++)
		{
			quads.value(i, j).triangle_a = IndexedTriangle(positions.index(i, j), positions.index(i + 1, j), positions.index(i, j + 1));
			quads.value(i, j).triangle_b = IndexedTriangle(positions.index(i + 1, j), positions.index(i + 1, j + 1), positions.index(i, j + 1));
		}
	}
}

void ns::MeshGenerator::checkNeighborsLinesXandZ(const MeshGenerator::Input& heightmap, const MapSizeType chunkPosition)
{
	const glm::ivec2 size(heightMapSettings_.numberOfPartitions.x + 1, heightMapSettings_.numberOfPartitions.y + 1);

	//check left
	HeightmapGenerator::NeighborChunkLine* line = heightmap.neighbors[HeightmapGenerator::Result::left];
	if (!line->xzComputed) {
		for (size_t j = 0; j < size.y; j++)
		{
			line->positions[j] = VertexWorldPosition(chunkPosition, line->positions[j].y, -1, j);
		}
		line->xzComputed = true;
	}

	//check right
	line = heightmap.neighbors[HeightmapGenerator::Result::right];
	if (!line->xzComputed){
		for (size_t j = 0; j < size.y; j++)
		{
			line->positions[j] = VertexWorldPosition(chunkPosition, line->positions[j].y, size.x, j);
		}
		line->xzComputed = true;
	}

	//check bottom
	line = heightmap.neighbors[HeightmapGenerator::Result::bottom];
	if (!line->xzComputed) {
		for (size_t i = 0; i < size.x; i++)
		{
			line->positions[i] = VertexWorldPosition(chunkPosition, line->positions[i].y, i, -1);
		}
		line->xzComputed = true;
	}

	//check top
	line = heightmap.neighbors[HeightmapGenerator::Result::top];
	if (!line->xzComputed) {
		for (size_t i = 0; i < size.x; i++)
		{
			line->positions[i] = VertexWorldPosition(chunkPosition, line->positions[i].y, i, size.y);
		}
		line->xzComputed = true;
	}
}
