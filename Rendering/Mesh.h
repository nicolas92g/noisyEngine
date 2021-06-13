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

		Vertex() :
			position(glm::vec3(0)),
			normal(glm::vec3(0)),
			uv(glm::vec2(0)),
			tangent(glm::vec3(0)),
			bitangent(glm::vec3(0))
		{}

		Vertex(const glm::vec3& pos) :
			position(pos),
			normal(glm::vec3(0)),
			uv(glm::vec2(0)),
			tangent(glm::vec3(0)),
			bitangent(glm::vec3(0))
		{}

		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	struct MeshConfigInfo {
		MeshConfigInfo() :
			name("unnamed"),
			supportNormalMapping(true),
			primitive(GL_TRIANGLES),
			indexType(GL_UNSIGNED_INT)
		{}
		std::string name;
		bool supportNormalMapping;
		GLuint primitive;
		GLuint indexType;
	};

	class Mesh : public Drawable
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, 
			const std::vector<unsigned int>& indices,
			const ns::Material& material, 
			const MeshConfigInfo& info = MeshConfigInfo());
		~Mesh();

		virtual void draw(const Shader& shader) const override;

	protected:
		unsigned int vertexArrayObject_;
		unsigned int vertexBufferObject_;
		unsigned int indexBufferObject_;

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
