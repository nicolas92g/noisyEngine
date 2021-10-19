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
	class SphereContainer : public Drawable
	{
	public:
		/**
		 * @brief create the sphere container
		 * \param resolution
		 * \param sphereRadius
		 */
		SphereContainer(uint32_t resolution, float sphereRadius);
		/**
		 * @brief return the progression of the sphere creation
		 * \return 
		 */
		double sphereProgressionPercentage() const;
		/**
		 * @brief return a mesh that render the chunk grid of the sphere 
		 * \return 
		 */
		std::shared_ptr<Mesh> getDebugSphere() const;
		/**
		 * @brief input a position relative to the sphere and normalized
		 * \param normalizedVector
		 * \return
		 */
		std::shared_ptr<SphereChunk> findChunk(const glm::vec3& normalizedVector);
		/**
		 * @brief return the radius of the sphere
		 * \return 
		 */
		float radius() const;

		void update(const glm::vec3& direction);

		void DEBUG_showOrigin() {
			
			for (size_t i = 1; i < NUMBER_OF_FACES_IN_A_CUBE + 1; i++)
			{
				for (size_t x = 0; x < i; x++)
				{
					if (loadChunk(Index(i - 1, x, 0))) return;
				}
				loadChunk(Index(i - 1, i + 1, 0));
			}
		}

		void clear() {
			for (size_t i = 0; i < loadedChunks_.size(); i++)
			{
				if (unloadChunk(loadedChunks_[i]->index)) return;
			}
		}

		virtual void draw(const ns::Shader& shader) const override;
		

		static std::array<std::vector<glm::ivec2>, ns::maximunRenderDistance> getOrder();
		static std::array<std::vector<glm::ivec2>, ns::maximunRenderDistance> loadingOrder;

		friend class SphereChunk;
	protected:

		//define the value that allow to access memory
		struct Index {
			Index(uint8_t face = NULL_FACE_INDEX, uint16_t i = 0, uint16_t j = 0) : face(face), i(i), j(j){}

			uint8_t face;	//index between 0 and 5, it is the face of the cube
			uint16_t i;		//x on the square face
			uint16_t j;		//y on the square face

			bool isNull() const { return (face == NULL_FACE_INDEX); }
			static const Index null;

			void add(glm::ivec2 offset, uint32_t resolution, uint8_t itCount = 0);
		};

		//define some chunk coordinates (a square with four vertices but vertices are not copied, they are indexed)
		struct ChunkCoords {
			Index a;
			Index b;
			Index c;
			Index d;
		};

		//store the min and max of each position components
		struct ChunkLimits {
			float minX;
			float maxX;
			float minY;
			float maxY;
			float minZ;
			float maxZ;
		};

		//describe a zone of chunks to allow fast search of a chunk
		struct ChunksRegion {
			ChunksRegion(glm::u16vec2 first = { 0,0 }, glm::u16vec2 last = { 0,0 }) : firstChunkIndex(first), lastChunkIndex(last) {}
			ChunkLimits limit{};//limits of the region
			glm::u16vec2 firstChunkIndex;	//smallest chunk index
			glm::u16vec2 lastChunkIndex;	//biggest chunk index
			std::shared_ptr<ns::BiArray<ChunksRegion>> innerRegions;//array of four regions or nothing if it is too small
		};

		//describe a chunk
		struct Chunk {
			std::shared_ptr<SphereChunk> mesh;//mesh of the chunk
			ChunkCoords coords{};//position of the chunk
			ChunkLimits limit{};//limits of the chunk
			Index index;	//allow to find chunk in the memory 
		};

	protected:
		//parameters
		const uint32_t resolution_;
		const uint32_t resolutionPlusOne_;
		const float radius_;

		//data
		std::array<ns::BiArray<Chunk>, NUMBER_OF_FACES_IN_A_CUBE> terrain_; //terrain
		std::array<ns::BiArray<glm::vec3>, NUMBER_OF_FACES_IN_A_CUBE> vertices_;//spheric grid vertices 
		std::array<ChunksRegion, NUMBER_OF_FACES_IN_A_CUBE> subRegions;	//sub regions of the chunk that allow to quickly search for a chunk
		std::vector<Chunk*> loadedChunks_;		//pointers of the chunks that are loaded
		Index centralChunk_;
		
		//multi-threading
		std::thread sphereThread_;
		std::atomic_uint32_t sphereProgression_;

	protected:
		bool checkCoordIsInLimit(const glm::vec3& pos, const ChunkLimits& limit) const;//allow to know if a position is in a chunk
		void fillChunkLimits(ChunkLimits& limit, const ChunkCoords& chunkPos);//compute the limits of a chunk
		void fillChunkSubRegions(ChunksRegion& chunk, uint8_t face);		//compute the sub-regions of a face

		const ChunksRegion& findRegion(const ChunksRegion& region, const glm::vec3& pos) const;//recursive function that find the smallest sub region that contain the chunk

		const glm::vec3& vertex(const Index& index) const;//get a vertex of the spheric grid
		glm::vec3& vertex(const Index& index);	//get a vertex of the spheric grid

		const Chunk& chunk(const Index& index) const;//get a chunk of the terrain
		Chunk& chunk(const Index& index);//get a chunk of the terrain
		
		Index find(const glm::vec3& normalizedVector) const;//find a chunk index with the normalized position relative to the sphere
		Index find(const Index& previousIndex, const glm::vec3& normalizedVector) const;//find a chunk index by searching around the previous chunk

		bool isLoaded(const Index& chunk) const;//allow to know if a chunk is loaded 
		bool loadChunk(const Index& chunk);
		bool unloadChunk(const Index& chunk);

		static void genSphereVertices(SphereContainer* object);//create the grid in the object (multi-threadable function)

		static void logRegion(const ChunksRegion& region){
			dout << "\nstart :\nfirst = " << to_string((glm::ivec2)region.firstChunkIndex) <<
				"\nlast = " << to_string((glm::ivec2)region.lastChunkIndex) << "\n-------->\n\n";

			if (region.innerRegions) {
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[0].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[0].lastChunkIndex) << "\n--\n";
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[1].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[1].lastChunkIndex) << "\n--\n";
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[2].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[2].lastChunkIndex) << "\n--\n";
				dout << "first = " << to_string((glm::ivec2)(*region.innerRegions)[3].firstChunkIndex) <<
					"\nlast = " << to_string((glm::ivec2)(*region.innerRegions)[3].lastChunkIndex) << "\n\n";
			}
		}
	};
}
