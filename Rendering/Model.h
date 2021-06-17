#pragma once
#include "Mesh.h"

#include <assimp/scene.h>
#include <memory>

#include "Light.h"

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

		std::vector<std::shared_ptr<ns::LightBase_>> getLights() const;
		ns::Material* getMaterial(const std::string& materialName);
	protected:
		const aiScene* scene_;

		std::string filepath_;
		std::string dir_;

		std::vector<std::shared_ptr<Mesh>> meshes_;
		std::vector<std::shared_ptr<ns::Material>> materials_;
		std::vector<std::shared_ptr<LightBase_>> lights_;

	protected:
		void readNodesFromNode(aiNode* node);
		void createMeshFromNode(aiMesh* mesh);

		void getLights();

		GLuint pickIndexType(size_t numberOfPositions) const;

		
	};
};
