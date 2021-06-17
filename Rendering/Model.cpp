#include "Model.h"

//stl
#include <iostream>
#include <limits>

//assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

//glm
#include <glm/glm.hpp>

//ns
#include <Utils/utils.h>

ns::Model::Model(const std::string& modelFilePath)
{
	Assimp::Importer importer;
	scene_ = importer.ReadFile(modelFilePath,
		aiProcess_Triangulate
		| aiProcess_OptimizeMeshes
		| aiProcess_FlipUVs
		| aiProcess_PreTransformVertices
		| aiProcess_JoinIdenticalVertices
		| aiProcess_CalcTangentSpace
	);

	if (!scene_ || scene_->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene_->mRootNode) {
		std::cerr << "error while loading the file : " << modelFilePath << " with assimp\n"
			<< importer.GetErrorString() << std::endl;
		return;
	}

	filepath_ = modelFilePath;
	dir_ = filepath_.substr(0, filepath_.find_last_of('/'));

	//loadEmbeddedTextures();

	readNodesFromNode(scene_->mRootNode);
}

ns::Model::~Model(){}

void ns::Model::draw(const ns::Shader& shader) const
{
	for (const auto& mesh : meshes_) {
		mesh->draw(shader);
	}
}

void ns::Model::readNodesFromNode(aiNode* node)
{
	//read this node
	for (size_t i = 0; i < node->mNumMeshes; i++)
		createMeshFromNode(scene_->mMeshes[node->mMeshes[i]]);

	//read all the childs recursively
	for (size_t i = 0; i < node->mNumChildren; i++)
		readNodesFromNode(node->mChildren[i]);
}

void ns::Model::createMeshFromNode(aiMesh* mesh)
{
	std::vector<ns::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<ns::TextureView> textures;
	
	ns::MeshConfigInfo info;
	info.supportNormalMapping = mesh->HasTangentsAndBitangents();
	info.name = mesh->mName.C_Str();
	info.primitive = GL_TRIANGLES;
	info.indexType = pickIndexType(mesh->mNumVertices);

	//fill vertices
	vertices.resize(mesh->mNumVertices);
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex v;
		//fill position
		v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		//fill normals
		v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		//fill uvs
		if (mesh->HasTextureCoords(0))
			v.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		else v.uv = {.0f, .0f};

		if (info.supportNormalMapping) {
			//fill tangents
			v.tangent = { mesh->mTangents[i].x , mesh->mTangents[i].y , mesh->mTangents[i].z };

			//fill bitangents
			v.bitangent = { mesh->mBitangents[i].x , mesh->mBitangents[i].y , mesh->mBitangents[i].z };
		}
		else {
			v.tangent = glm::vec3(0);
			v.bitangent = glm::vec3(0);
		}

		vertices[i] = v;
	}

	//fill indices
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for (unsigned short j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	//fill material
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* mtl = scene_->mMaterials[mesh->mMaterialIndex];
		materials_.push_back(std::make_shared<ns::Material>(mtl, dir_, scene_->mTextures, scene_->mNumTextures));
	}

	meshes_.push_back(std::make_shared<ns::Mesh>(vertices, indices, *materials_[materials_.size() - 1].get(), info));
}

void ns::Model::getLights()
{
	for (size_t i = 0; i < scene_->mNumLights; i++)
	{
		switch (scene_->mLights[i]->mType) {

		case aiLightSourceType::aiLightSource_DIRECTIONAL:
			lights_.push_back(std::make_shared<ns::DirectionalLight>(
				to_vec3(scene_->mLights[i]->mDirection), to_vec3(scene_->mLights[i]->mColorDiffuse)));
			break;
		case aiLightSourceType::aiLightSource_POINT:
			lights_.push_back(std::make_shared<ns::PointLight>(
				to_vec3(scene_->mLights[i]->mPosition), scene_->mLights[i]->mAttenuationLinear, to_vec3(scene_->mLights[i]->mColorDiffuse)));
			break;
		case aiLightSourceType::aiLightSource_SPOT:
			lights_.push_back(std::make_shared<ns::SpotLight>(
				to_vec3(scene_->mLights[i]->mPosition), scene_->mLights[i]->mAttenuationLinear, to_vec3(scene_->mLights[i]->mColorDiffuse),
				to_vec3(scene_->mLights[i]->mDirection), 
				glm::degrees(scene_->mLights[i]->mAngleInnerCone), glm::degrees(scene_->mLights[i]->mAngleOuterCone)));
			break;

		default:
			break;
		}
	}	
}

GLuint ns::Model::pickIndexType(size_t numberOfPositions) const
{
	if (numberOfPositions <= std::numeric_limits<unsigned char>::max()) {
		return GL_UNSIGNED_BYTE;
	}
	else if (numberOfPositions <= std::numeric_limits<unsigned short>::max()) {
		return GL_UNSIGNED_SHORT;
	}
	else { 
		return GL_UNSIGNED_INT;
	}
}

void ns::Model::describe() const
{
	std::cout << "model : " << filepath_ <<
		"\n num meshes = " << meshes_.size() << "\n";
	for (const auto& mesh : meshes_)
		std::cout;
}

std::vector<std::shared_ptr<ns::LightBase_>> ns::Model::getLights() const
{
	return lights_;
}

ns::Material* ns::Model::getMaterial(const std::string& materialName)
{
	for (const auto& mtl : materials_) {
		if (mtl->name() == materialName)
			return mtl.get();
	}
	return nullptr;
}
