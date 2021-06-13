#pragma once
#include "Mesh.h"

#include <assimp/scene.h>
#include <memory>

namespace ns {
	class Model : public Drawable
	{
		
	public:
		Model(const std::string& modelFilePath);

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		~Model();

		virtual void draw(const Shader& shader) const override;
		void describe() const;

		ns::Material* getMaterial(const std::string& materialName);
	protected:
		const aiScene* scene_;

		std::string filepath_;
		std::string dir_;

		std::vector<std::shared_ptr<ns::Mesh>> meshes_;
		std::vector<std::shared_ptr<ns::Material>> materials_;

	protected:
		void readNodesFromNode(aiNode* node);
		void createMeshFromNode(aiMesh* mesh);

		GLuint pickIndexType(size_t numberOfPositions) const;

		
	};
};
