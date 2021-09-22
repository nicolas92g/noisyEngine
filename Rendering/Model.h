#pragma once
#include "Mesh.h"

#include <assimp/scene.h>

#include <memory>
#include <map>

#include <ofbx.h>

#include "Light.h"

namespace ns {
	/**
	 * @brief (TO DO) used for animation purposes
	 */
	struct BoneInfo 
	{
		BoneInfo() :
			offset(0),
			transformation(0)
		{}

		glm::mat4 offset;
		glm::mat4 transformation;
	};
	/**
	 * @brief allow to create some drawable object with an .obj, .fbx file or others
	 * this create an array of meshes that can be draw with draw()
	 * materials can be loaded from the files but it is recommanded to use a .nsmat file 
	 */
	class Model : public Drawable
	{
	public:
		/**
		 * @brief create the model with a file
		 * \param modelFilePath
		 */
		Model(const std::string& modelFilePath);
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		/**
		 * @brief free the buffers
		 */
		~Model();
		/**
		 * @brief draw all the meshes
		 * \param shader
		 */
		virtual void draw(const Shader& shader) const override;
		/**
		 * @brief for debugging purposes, log a description of the model
		 */
		void describe() const;
		/**
		 * @brief (TO DO) do not use this
		 * \return 
		 */
		std::vector<std::shared_ptr<ns::LightBase_>> getLights() const;
		/**
		 * @brief allow to pick a material by its name and edit it after.
		 * \param materialName
		 * \return 
		 */
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
