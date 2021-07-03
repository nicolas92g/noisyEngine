#include "Material.h"

#include <glad/glad.h>
#include <iostream>

#include <Utils/utils.h>
#include <Utils/DebugLayer.h>

std::vector<std::unique_ptr<ns::Texture>> ns::Material::textures;
bool ns::Material::describeMaterialsWhenCreate = false;

ns::Material::Material(const glm::vec3& albedo, float roughness, float metallic, float emission)
	:
	albedo_(albedo),
	roughness_(roughness),
	metallic_(metallic),
	emission_(emission)
{}

ns::Material::Material(aiMaterial* mtl, const std::string& texturesDirectory)
	:
	Material()
{
	if (describeMaterialsWhenCreate) describeMaterial(mtl);
	aiString path;

	//albedo loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_BASE_COLOR)) {
		mtl->GetTexture(aiTextureType::aiTextureType_BASE_COLOR, 0, &path);
		albedoMap_ = addTexture(texturesDirectory, path.C_Str());
	}
	else if (mtl->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE)) {
		mtl->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path);
		albedoMap_ = addTexture(texturesDirectory, path.C_Str());
	}

	//roughness loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_SHININESS)) {
		mtl->GetTexture(aiTextureType::aiTextureType_SHININESS, 0, &path);
		roughnessMap_ = addTexture(texturesDirectory, path.C_Str());
	}

	//metallic loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_SPECULAR)) {
		mtl->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &path);
		metallicMap_ = addTexture(texturesDirectory, path.C_Str());
	}

	//emission loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_EMISSIVE)) {
		mtl->GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &path);
		emissionMap_ = addTexture(texturesDirectory, path.C_Str());
	}
	
	//normal loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_HEIGHT)) {
		mtl->GetTexture(aiTextureType::aiTextureType_HEIGHT, 0, &path);
		normalMap_ = addTexture(texturesDirectory, path.C_Str());
	}
	else if (mtl->GetTextureCount(aiTextureType::aiTextureType_NORMALS)) {
		mtl->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &path);
		normalMap_ = addTexture(texturesDirectory, path.C_Str());
	}

	//ambient occlusion loading
	if (mtl->GetTextureCount(aiTextureType::aiTextureType_AMBIENT_OCCLUSION)) {
		mtl->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0, &path);
		ambientOcclusionMap_ = addTexture(texturesDirectory, path.C_Str());
	}

	if (!albedoMap_.has_value()) {
		aiColor4D diffuse;
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
		convertAiColor4dToVec3(diffuse, albedo_);
	}
}

ns::Material::Material(const ofbx::Material* mtl, const std::string& texturesDirectory)
	: 
	Material()
{
	albedo_ = to_vec3(mtl->getDiffuseColor());
	metallic_ = mtl->getReflectionFactor();
	roughness_ = mtl->getShininessExponent();
	name_ = mtl->name;

	const ofbx::Texture* tex;
	char buffer[200];
	
	tex = mtl->getTexture(ofbx::Texture::TextureType::DIFFUSE);
	if (tex) {
		tex->getFileName().toString(buffer);
		albedoMap_ = addTexture("", buffer);
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::SPECULAR);
	if (tex) {
		tex->getFileName().toString(buffer);
		metallicMap_ = addTexture("", buffer);
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::REFLECTION);
	if (tex) {
		tex->getFileName().toString(buffer);
		metallicMap_ = addTexture("", buffer);
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::SHININESS);
	if (tex) {
		tex->getFileName().toString(buffer);
		roughnessMap_ = addTexture("", buffer);
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::NORMAL);
	if (tex) {
		tex->getFileName().toString(buffer);
		normalMap_ = addTexture("", buffer);
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::EMISSIVE);
	if (tex) {
		tex->getFileName().toString(buffer);
		ambientOcclusionMap_ = addTexture("", buffer);
	}

}

void ns::Material::bind(const ns::Shader& shader) const
{
	short freeTextureSampler = 0;

	shader.set("mat.hasAlbedoMap", albedoMap_.has_value());
	shader.set("mat.hasRoughnessMap", roughnessMap_.has_value());
	shader.set("mat.hasMetallicMap", metallicMap_.has_value());
	shader.set("mat.hasEmissionMap", emissionMap_.has_value());
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

	if (emissionMap_.has_value()) {

		shader.set<int>("mat.emissionMap", freeTextureSampler);

		glActiveTexture(GL_TEXTURE0 + freeTextureSampler);
		emissionMap_.value().bind();
		freeTextureSampler++;
	}
	else {
		shader.set("mat.emission", emission_);
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

void ns::Material::setEmissionConstant(const glm::vec3& emission)
{
	if (emission.x < 0 || emission.y < 0 || emission.z < 0) return;
	emission_ = emission;
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

glm::vec3 ns::Material::emission() const
{
	return emission_;
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

void ns::Material::setEmissionTexture(const TextureView& texture)
{
	removeEmissionTexture();
	emissionMap_ = texture;
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

void ns::Material::removeEmissionTexture()
{
	if (emissionMap_.has_value()) removeTexture(emissionMap_.value());
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
			Debug::get() << mtl->GetName().C_Str() << " has a texture of type : " << i << " at : " << path.C_Str() << std::endl;
		}
	}

	for (size_t i = 0; i < mtl->mNumProperties; i++)
	{
		Debug::get() << mtl->GetName().C_Str() << " has a property of type : " <<  mtl->mProperties[i]->mKey.C_Str() << std::endl;
	}
}

void ns::Material::clearTextures()
{
	textures.clear();
}

ns::TextureView ns::Material::addTexture(const std::string& directory, const std::string& path)
{
	std::string filepath;
	if(!directory.empty())
		filepath = directory + "/" + path;
	else
		filepath = path;

	for (const auto& texture : textures) {
		if (texture->filePath() == filepath) {
			return *texture;
		}
	}
	textures.push_back(std::make_unique<Texture>(filepath.c_str()));

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

void ns::Material::displayTextures(const ofbx::Material* mtl)
{
	const ofbx::Texture* tex;
	char buffer[200];

	tex = mtl->getTexture(ofbx::Texture::TextureType::DIFFUSE);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture  diffuse : " << buffer << std::endl;
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::SPECULAR);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture specular : " << buffer << std::endl;
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::SHININESS);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture  SHININESS : " << buffer << std::endl;
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::NORMAL);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture NORMAL  : " << buffer << std::endl;
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::EMISSIVE);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture  EMISSIVE : " << buffer << std::endl;
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::AMBIENT);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture AMBIENT  : " << buffer << std::endl;
	}

	tex = mtl->getTexture(ofbx::Texture::TextureType::REFLECTION);
	if (tex) {
		tex->getFileName().toString(buffer);
		Debug::get() << "texture  REFLECTION : " << buffer << std::endl;
	}
}
