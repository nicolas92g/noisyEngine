#include "Material.h"

#include <glad/glad.h>
#include <iostream>

std::vector<std::unique_ptr<ns::Texture>> ns::Material::textures;
bool ns::Material::describeMaterialsWhenCreate = false;

ns::Material::Material()
	:
	albedo_(.5f),
	roughness_(.8f),
	metallic_(.1f),
	name_("unnamed")
{
}

ns::Material::Material(aiMaterial* mtl, const std::string& texturesDirectory,  aiTexture** const embeddedTextures, int numberOfEmbeddedTextures)
	:
	albedo_(.5f),
	roughness_(.8f),
	metallic_(.1f),
	name_(mtl->GetName().C_Str())
{
	if (describeMaterialsWhenCreate) describeMaterial(mtl);
	aiString path;

	//albedo loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_BASE_COLOR)) {
		mtl->GetTexture(aiTextureType::aiTextureType_BASE_COLOR, 0, &path);
		albedoMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}
	else if (mtl->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE)) {
		mtl->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path);
		albedoMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}

	//roughness loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_SHININESS)) {
		mtl->GetTexture(aiTextureType::aiTextureType_SHININESS, 0, &path);
		roughnessMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}

	//metallic loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_SPECULAR)) {
		mtl->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &path);
		metallicMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}
	
	//normal loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_HEIGHT)) {
		mtl->GetTexture(aiTextureType::aiTextureType_HEIGHT, 0, &path);
		normalMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}
	else if (mtl->GetTextureCount(aiTextureType::aiTextureType_NORMALS)) {
		mtl->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &path);
		normalMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}

	//ambient occlusion loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_AMBIENT_OCCLUSION)) {
		mtl->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0, &path);
		ambientOcclusionMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}
	else if (mtl->GetTextureCount(aiTextureType::aiTextureType_EMISSIVE)) {
		mtl->GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &path);
		ambientOcclusionMap_ = addTexture(texturesDirectory, path, embeddedTextures, numberOfEmbeddedTextures);
	}

	if (!albedoMap_.has_value()) {
		aiColor4D diffuse;
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
		convertAiColor4dToVec3(diffuse, albedo_);
	}

	//if (!roughnessMap_.has_value()) {
	//	aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &roughness_);
	//	std::cout << roughness_ << std::endl;
	//}
}

void ns::Material::bind(const ns::Shader& shader) const
{
	short freeTextureSampler = 0;

	shader.set("mat.hasAlbedoMap", albedoMap_.has_value());
	shader.set("mat.hasRoughnessMap", roughnessMap_.has_value());
	shader.set("mat.hasMetallicMap", metallicMap_.has_value());
	shader.set("mat.hasNormalMap", normalMap_.has_value());
	shader.set("mat.hasAmbientOcclusionMap", ambientOcclusionMap_.has_value());

	if (albedoMap_.has_value()) {
		//send sampler texture to shader
		shader.set<int>("mat.albedoMap", freeTextureSampler);
		//bind texture on this sampler
		glActiveTexture(GL_TEXTURE0 + freeTextureSampler);
		albedoMap_.value().bind();
		//use next sampler
		freeTextureSampler++;
	}
	else {
		shader.set("mat.albedo", albedo_);
	}

	if (roughnessMap_.has_value()) {
		
		shader.set<int>("mat.roughnessMap", freeTextureSampler);

		glActiveTexture(GL_TEXTURE0 + freeTextureSampler);
		roughnessMap_.value().bind();
		freeTextureSampler++;
	}
	else {
		shader.set("mat.roughness", roughness_);
	}

	if (metallicMap_.has_value()) {
		
		shader.set<int>("mat.metallicMap", freeTextureSampler);

		glActiveTexture(GL_TEXTURE0 + freeTextureSampler);
		metallicMap_.value().bind();
		freeTextureSampler++;
	}
	else {
		shader.set("mat.metallic", metallic_);
	}

	if (normalMap_.has_value()) {
		
		shader.set<int>("mat.normalMap", freeTextureSampler);

		glActiveTexture(GL_TEXTURE0 + freeTextureSampler);
		normalMap_.value().bind();
		freeTextureSampler++;
	}

	if (ambientOcclusionMap_.has_value()) {
		
		shader.set<int>("mat.ambientOcclusionMap", freeTextureSampler);

		glActiveTexture(GL_TEXTURE0 + freeTextureSampler);
		ambientOcclusionMap_.value().bind();
		freeTextureSampler++;
	}
}

const std::string& ns::Material::name() const
{
	return name_;
}

void ns::Material::setRoughnessConstant(float roughness)
{
	roughness_ = std::max(.0f, roughness);
}

void ns::Material::setMetallicConstant(float metallic)
{
	metallic_ = std::max(.0f, metallic);
}

void ns::Material::setAlbedoColor(const glm::vec3& color)
{
	if (color.x < 0 || color.y < 0 || color.z < 0) return;
	albedo_ = color;
}

float ns::Material::roughness() const
{
	return roughness_;
}

float ns::Material::metallic() const
{
	return metallic_;
}

glm::vec3 ns::Material::albedo() const
{
	return albedo_;
}

void ns::Material::setAlbedoTexture(const TextureView& texture)
{
	removeAlbedoTexture();
	albedoMap_ = texture;
}

void ns::Material::setRoughnessTexture(const TextureView& texture)
{
	removeRoughnessTexture();
	roughnessMap_ = texture;
}

void ns::Material::setMetallicTexture(const TextureView& texture)
{
	removeMetallicTexture();
	metallicMap_ = texture;
}

void ns::Material::setNormalTexture(const TextureView& texture)
{
	removeNormalTexture();
	normalMap_ = texture;
}

void ns::Material::setAmbientOcclusionTexture(const TextureView& texture)
{
	removeAmbientOcclusionTexture();
	ambientOcclusionMap_ = texture;
}

void ns::Material::removeAlbedoTexture()
{
	if (albedoMap_.has_value()) removeTexture(albedoMap_.value());
}

void ns::Material::removeRoughnessTexture()
{
	if (roughnessMap_.has_value()) removeTexture(roughnessMap_.value());
}

void ns::Material::removeMetallicTexture()
{
	if (metallicMap_.has_value()) removeTexture(metallicMap_.value());
}

void ns::Material::removeNormalTexture()
{
	if (normalMap_.has_value()) removeTexture(normalMap_.value());
}

void ns::Material::removeAmbientOcclusionTexture()
{
	if (ambientOcclusionMap_.has_value()) removeTexture(ambientOcclusionMap_.value());
}

void ns::Material::convertAiColor4dToVec3(const aiColor4D& ai4d, glm::vec3& vec)
{
	vec.r = ai4d.r;
	vec.g = ai4d.g;
	vec.b = ai4d.b;
}

void ns::Material::describeMaterial(aiMaterial* mtl)
{
	aiString path;
	for (size_t i = 0; i < 18; i++)
	{
		if (mtl->GetTextureCount(static_cast<aiTextureType>(i))) {
			mtl->GetTexture(static_cast<aiTextureType>(i), 0, &path);
			std::cout << mtl->GetName().C_Str() << " has a texture of type : " << i << " at : " << path.C_Str() << std::endl;
		}
	}

	for (size_t i = 0; i < mtl->mNumProperties; i++)
	{
		std::cout << mtl->GetName().C_Str() << " has a property of type : " <<  mtl->mProperties[i]->mKey.C_Str() << std::endl;
	}
}

void ns::Material::clearTextures()
{
	textures.clear();
}

ns::TextureView ns::Material::addTexture(const std::string& directory, const aiString& path_, aiTexture** const embeddedTextures, int numberOfEmbeddedTextures)
{
	//embedded texture ?
	static std::string path;
	path = path_.C_Str();

	if (path.size() < 3 and path.find('*') != std::string::npos) {
		path = path[1];
		std::cout << "path = |" << path << "|" << std::endl;
		int textureIndex = stoi(path);

		if (textureIndex < numberOfEmbeddedTextures && textureIndex >= 0)
			textures.push_back(std::make_unique<Texture>(embeddedTextures[textureIndex]));
		else
			std::cerr << "ns::Material Error : invalid embbeded texture index ?!\n";
			
	}
	else {
		std::string filepath(directory + "/" + path);

		for (const auto& texture : textures) {
			if (texture->filePath() == filepath) {
				return *texture;
			}
		}
		textures.push_back(std::make_unique<Texture>(filepath.c_str()));
	}

	return *textures[textures.size() - 1];
}

void ns::Material::removeTexture(const TextureView& view)
{
	for (auto it = textures.begin(); it != textures.end(); ++it) {
		if (**it == view) {
			textures.erase(it);
			return;
		}
	}
}
