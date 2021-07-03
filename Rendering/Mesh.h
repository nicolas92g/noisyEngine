#pragma once
#include <glm/glm.hpp>
#include <vector>

//ns
#include "Texture.h"
#include "Shader.h"
#include "Material.h"
#include "Drawable.h"

namespace ns {

	struct Vertex {
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

		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		void genTangent();
		void genBitangent();
	};

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

	struct MeshConfigInfo {
		MeshConfigInfo() :
			name("unnamed"),
			supportNormalMapping(true),
			primitive(GL_TRIANGLES),
			indexType(GL_UNSIGNED_INT),
			hasBitangents(true),
			hasAnimations(false)
		{}

		std::string name;
		bool supportNormalMapping;
		bool hasBitangents;
		GLuint primitive;
		GLuint indexType;
		bool hasAnimations;
	};

	class Mesh : public Drawable
	{
	public:
		Mesh(const std::vector<Vertex>& vertices,
			const std::vector<unsigned int>& indices,
			const ns::Material& material, 
			const MeshConfigInfo& info = MeshConfigInfo());

		Mesh(const std::vector<Vertex>& vertices,
			const std::vector<VertexBoneData>& animData,
			const std::vector<unsigned int>& indices,
			const ns::Material& material,
			const MeshConfigInfo& info = MeshConfigInfo());

		~Mesh();

		virtual void draw(const Shader& shader) const override;

	protected:
		unsigned vertexArrayObject_;
		unsigned vertexBufferObject_;
		unsigned bonesBufferObject_;
		unsigned indexBufferObject_;

		const Material& material_;
		int numberOfVertices_;

		const MeshConfigInfo info_;

	protected:
		const void* getIndices(const std::vector<unsigned int>& indices,
			std::vector<unsigned char>& indicesBytes,
			std::vector<unsigned short>& indicesShorts) const;
		const short getIndexTypeSize() const;
	};
};
