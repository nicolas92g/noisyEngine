#pragma once
#include <glm/glm.hpp>
#include <vector>

//ns
#include "Texture.h"
#include "Shader.h"
#include "Material.h"
#include "Drawable.h"

namespace ns {
	/**
	 * @brief struct that describe a single vertex in the 3D pipeline,
	 * it contain a position, a normal vector, some texture coordinates (uv) 
	 * and a tangent and bitangent vector for normal mapping
	 */
	struct Vertex {
		/**
		 * @brief full constructor with some default values equal to 0
		 * \param pos
		 * \param normal
		 * \param uv
		 * \param tangent
		 * \param bi
		 */
		Vertex(
			const glm::vec3& pos = glm::vec3(0.f), 
			const glm::vec3& normal = glm::vec3(0.f), 
			const glm::vec2& uv = glm::vec2(0.f), 
			const glm::vec3& tangent = glm::vec3(0.f), 
			const glm::vec3& bi = glm::vec3(0.f)) 
			:
			position(pos),
			normal(normal),
			uv(uv),
			tangent(tangent),
			bitangent(bi)
		{}
		/**
		 * @brief generate the tangent based on the normal vector
		 */
		void genTangent();
		/**
		 * @brief generate the bitangent based on the normal and the tangent
		 */
		void genBitangent();
		/**
		 * @brief store the position of the vertex
		 */
		glm::vec3 position;
		/**
		 * @brief store the normal vector of the vertex
		 */
		glm::vec3 normal;
		/**
		 * @brief store the texture coordinates of the vertex
		 */
		glm::vec2 uv;
		/**
		 * @brief store the tangent vector of the vertex
		 */
		glm::vec3 tangent;
		/**
		 * @brief store the bitangent vector of the vertex
		 */
		glm::vec3 bitangent;
	};
	/**
	 * @brief (TO DO) used for animation purposes
	 */
	struct VertexBoneData
	{
		VertexBoneData() :
			ids(glm::ivec4(0)),
			weights(0.f)
		{}

		void addBone(unsigned id, float weight) 
		{
			for (uint8_t i = 0; i < 4; i++)
			{
				float* const w = &weights.x + i;
				if (*w == 0.f) {
					*w = weight;
					*(&ids.x + i) = static_cast<int>(id);
				}
			}
		}

		glm::ivec4 ids;
		glm::vec4 weights;
	};
	/**
	 * @brief mesh configuration struct (this is used in the Mesh constructor)
	 */
	struct MeshConfigInfo {
		std::string name = "unnamed";
		bool supportNormalMapping = true;
		bool hasBitangents = true;
		GLuint primitive = GL_TRIANGLES;
		GLuint indexType = GL_UNSIGNED_INT;
		bool hasAnimations = false;
		bool indexedVertices = true;
	};
	/**
	 * @brief describe a Mesh with a single Material, that is drawable with one draw call
	 */
	class Mesh : public Drawable
	{
	public:
		/**
		 * @brief default constructor of a mesh with an array of vertices, an array of indices or not, a material and a configuration 
		 * \param vertices
		 * \param indices
		 * \param material
		 * \param info
		 */
		Mesh(const std::vector<Vertex>& vertices,
			const std::vector<unsigned int>& indices,
			const ns::Material& material = Material::getDefault(), 
			const MeshConfigInfo& info = MeshConfigInfo());
		/**
		 * @brief (TO DO) constructor to animate the mesh
		 * \param vertices
		 * \param animData
		 * \param indices
		 * \param material
		 * \param info
		 */
		Mesh(const std::vector<Vertex>& vertices,
			const std::vector<VertexBoneData>& animData,
			const std::vector<unsigned int>& indices,
			const ns::Material& material,
			const MeshConfigInfo& info = MeshConfigInfo());
		/**
		 * @brief free the buffers
		 */
		~Mesh();
		/**
		 * @brief draw the mesh
		 * \param shader
		 */
		virtual void draw(const ns::Shader& shader) const override;

	protected:
		unsigned vertexArrayObject_;
		unsigned vertexBufferObject_;
		unsigned bonesBufferObject_;
		unsigned indexBufferObject_;

		Material material_;
		int numberOfVertices_;

		const MeshConfigInfo info_;

	protected:
		const void* getIndices(const std::vector<unsigned int>& indices,
			std::vector<unsigned char>& indicesBytes,
			std::vector<unsigned short>& indicesShorts) const;
		const short getIndexTypeSize() const;

		friend class Debug;
	};
};
