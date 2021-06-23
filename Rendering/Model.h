#pragma once
#include "Mesh.h"

#include <assimp/scene.h>
#include <memory>
#include <ofbx.h>

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
		std::string filepath_;
		std::string dir_;

		//mesh content
		std::vector<std::shared_ptr<Mesh>> meshes_;
		std::vector<std::shared_ptr<ns::Material>> materials_;
		std::vector<std::shared_ptr<LightBase_>> lights_;

		//loading with assimp
	protected:
		bool importWithAssimp();
		
		void readNodesFromNode(aiNode* node, const aiScene* scene);
		void createMeshFromNode(aiMesh* mesh, const aiScene* scene);

		void getLights(const aiScene* scene);

		//loading with OpenFBX
	protected:
		bool importWithOpenFBX();

		void createMeshFromFBX(const ofbx::Mesh& mesh, ofbx::IScene& scene);

	protected:
		GLuint pickIndexType(size_t numberOfPositions) const;
	};
};
