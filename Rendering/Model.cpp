#include "Model.h"

//stl
#include <iostream>
#include <limits>
#include <fstream>

//assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

//glm
#include <glm/glm.hpp>

//ns
#include <Utils/utils.h>


ns::Model::Model(const std::string& modelFilePath)
{
	filepath_ = modelFilePath;
	dir_ = filepath_.substr(0, filepath_.find_last_of('/'));

	const std::string extension = modelFilePath.substr(modelFilePath.find_last_of('.') + 1);

	//importWithAssimp(); return;
	if (extension == "fbx") {
		importWithOpenFBX();
	}
	else importWithAssimp();
}

ns::Model::~Model(){} 

void ns::Model::draw(const ns::Shader& shader) const
{
	for (const auto& mesh : meshes_) {
		mesh->draw(shader);
	}
} 

bool ns::Model::importWithAssimp()
{
	const aiScene* scene;
	Assimp::Importer importer;
	scene = importer.ReadFile(filepath_,
		aiProcess_Triangulate
		| aiProcess_OptimizeMeshes
		| aiProcess_FlipUVs
		//| aiProcess_PreTransformVertices
		| aiProcess_JoinIdenticalVertices
		| aiProcess_CalcTangentSpace
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "error while loading the file : " << filepath_ << " with assimp\n"
			<< importer.GetErrorString() << std::endl;
		return false;
	}

	readNodesFromNode(scene->mRootNode, scene);
	return true;
}

bool ns::Model::importWithOpenFBX()
{
	using namespace ofbx;

	//open the file
	FILE* fp;
	fopen_s(&fp, filepath_.c_str(), "rb");
	if (!fp) return false;

	//determine the size of the file
	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	//buffer that store the all file content
	u8* content = new u8[file_size];
	fread(content, 1, file_size, fp);

	//load the fbx scene
	IScene& scene = *load(content, file_size, (u64)LoadFlags::TRIANGULATE);

	//free the file content buffer
	delete[] content;

	//check that loading was successful
	if (!&scene) {
		std::cerr << "failed to read file : "<< filepath_ << " with OpenFBX :\n" << getError() << std::endl;
		return false;
	}

	//goes through meshes
	for (int i = 0; i < scene.getMeshCount(); i++)
	{
		createMeshFromFBX(*scene.getMesh(i), scene);
	}

	return true;
}

void ns::Model::createMeshFromFBX(const ofbx::Mesh& mesh, ofbx::IScene& scene)
{
	using namespace ofbx;
	std::vector<ns::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<ns::TextureView> textures;

	ns::MeshConfigInfo info;
	info.supportNormalMapping = mesh.getGeometry()->getTangents() != nullptr;
	info.hasBitangents = false;
	info.name = mesh.name;
	info.primitive = GL_TRIANGLES;
	info.indexType = pickIndexType(mesh.getGeometry()->getVertexCount());

	const Geometry& geo = *mesh.getGeometry();
	//std::cout << "mesh " << mesh.name << " has " << geo.getIndexCount() << " indices and " << geo.getVertexCount() << " vertices\n";

	vertices.resize(mesh.getGeometry()->getVertexCount());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		Vertex v;
		//fill position
		v.position = {geo.getVertices()[i].x , geo.getVertices()[i].y, geo.getVertices()[i].z };

		//fill normals
		if (geo.getNormals() == nullptr) { std::cerr << "mesh " << info.name << " doesn't have normals !\n";}
		else v.normal = { geo.getNormals()[i].x, geo.getNormals()[i].y, geo.getNormals()[i].z };

		//fill uvs
		if (geo.getUVs() != nullptr)
			v.uv = {(float)geo.getUVs()[i].x, 1.0f - (float)geo.getUVs()[i].y };

		if (info.supportNormalMapping) {
			//fill tangents
			v.tangent = { geo.getTangents()[i].x , geo.getTangents()[i].y , geo.getTangents()[i].z };
		}
		else {
			v.genTangent();
		}
		vertices[i] = v;
	}

	//fill indices
	const int* faceIndices = geo.getFaceIndices();
	for (int i = 0; i < geo.getIndexCount(); ++i)
	{
		int idx = (faceIndices[i] < 0) ? -faceIndices[i] : (faceIndices[i] + 1);
		indices.push_back(static_cast<unsigned int>(idx - 1));
	}

	//fill material
	if (mesh.getMaterial(0) != nullptr) {
		materials_.push_back(std::make_shared<Material>(mesh.getMaterial(0), dir_));
	}

	meshes_.push_back(std::make_shared<ns::Mesh>(vertices, indices, *materials_[materials_.size() - 1], info));
}

void ns::Model::readNodesFromNode(aiNode* node, const aiScene* scene)
{
	//read this node
	for (size_t i = 0; i < node->mNumMeshes; i++)
		createMeshFromNode(scene->mMeshes[node->mMeshes[i]], scene);

	//read all the childs recursively
	for (size_t i = 0; i < node->mNumChildren; i++)
		readNodesFromNode(node->mChildren[i], scene);
}

void ns::Model::createMeshFromNode(aiMesh* mesh, const aiScene* scene)
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
		for (unsigned j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	//fill material
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* mtl = scene->mMaterials[mesh->mMaterialIndex];
		materials_.push_back(std::make_shared<ns::Material>(mtl, dir_));
	}

	meshes_.push_back(std::make_shared<ns::Mesh>(vertices, indices, *materials_[materials_.size() - 1].get(), info));
}

void ns::Model::getLights(const aiScene* scene)
{
	for (size_t i = 0; i < scene->mNumLights; i++)
	{
		switch (scene->mLights[i]->mType) {

		case aiLightSourceType::aiLightSource_DIRECTIONAL:
			lights_.push_back(std::make_shared<ns::DirectionalLight>(
				to_vec3(scene->mLights[i]->mDirection), to_vec3(scene->mLights[i]->mColorDiffuse)));
			break;
		case aiLightSourceType::aiLightSource_POINT:
			lights_.push_back(std::make_shared<ns::PointLight>(
				to_vec3(scene->mLights[i]->mPosition), scene->mLights[i]->mAttenuationLinear, to_vec3(scene->mLights[i]->mColorDiffuse)));
			break;
		case aiLightSourceType::aiLightSource_SPOT:
			lights_.push_back(std::make_shared<ns::SpotLight>(
				to_vec3(scene->mLights[i]->mPosition), scene->mLights[i]->mAttenuationLinear, to_vec3(scene->mLights[i]->mColorDiffuse),
				to_vec3(scene->mLights[i]->mDirection), 
				glm::degrees(scene->mLights[i]->mAngleInnerCone), glm::degrees(scene->mLights[i]->mAngleOuterCone)));
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
