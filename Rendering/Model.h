#pragma once
#include "Mesh.h"

#include <assimp/scene.h>

#include <memory>
#include <map>

#include <ofbx.h>

#include "Light.h"

namespace ns {
	struct BoneInfo 
	{
		BoneInfo() :
			offset(0),
			transformation(0)
		{}

		glm::mat4 offset;
		glm::mat4 transformation;
	};

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
		std::vector<std::unique_ptr<Mesh>> meshes_;
		std::vector<std::unique_ptr<ns::Material>> materials_;
		std::vector<std::shared_ptr<LightBase_>> lights_;

		//animation content
		unsigned numBones_;
		std::map<std::string, unsigned> boneMapping_;
		std::vector<BoneInfo> boneInfo_;
		
	protected:	//loading with assimp
		bool importWithAssimp();
		
		void readNodesFromAssimp(aiNode* node, const aiScene* scene);
		void createMeshFromAssimp(aiMesh* mesh, const aiScene* scene);

		void getLightsFromAssimp(const aiScene* scene);
		void loadBonesFromAssimp(const aiScene& scene, const aiMesh& mesh, std::vector<VertexBoneData>& bones);

		
	protected:	//loading with OpenFBX
		bool importWithOpenFBX();

		void createMeshFromFBX(const ofbx::Mesh& mesh, ofbx::IScene& scene);

	protected:
		GLuint pickIndexType(size_t numberOfPositions) const;

		friend class Debug;
	};
};
