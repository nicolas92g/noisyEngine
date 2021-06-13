#pragma once

//glm
#include <glm/glm.hpp>

//stl
#include <optional>
#include <memory>

//ns
#include "Texture.h"
#include "Shader.h"

//assimp
#include <assimp/material.h>

namespace ns {
	class Material
	{
	public:
		Material(aiMaterial* mtl, const std::string& texturesDirectory, aiTexture** const embeddedTextures, int numberOfEmbeddedTextures);

		void bind(const ns::Shader& shader) const;
		const std::string& name() const;

		void setRoughnessConstant(float roughness);
		void setMetallicConstant(float metallic);
		void setAlbedoColor(const glm::vec3& color);

		float roughness() const;
		float metallic() const;
		glm::vec3 albedo() const;

		void setAlbedoTexture(const TextureView& texture);
		void setRoughnessTexture(const TextureView& texture);
		void setMetallicTexture(const TextureView& texture);
		void setNormalTexture(const TextureView& texture);
		void setAmbientOcclusionTexture(const TextureView& texture);

		void removeAlbedoTexture();
		void removeRoughnessTexture();
		void removeMetallicTexture();
		void removeNormalTexture();
		void removeAmbientOcclusionTexture();

		static void convertAiColor4dToVec3(const aiColor4D& ai4d, glm::vec3& vec);
		static void describeMaterial(aiMaterial* mtl);

		//material textures usage:
		static void clearTextures();
		static bool describeMaterialsWhenCreate;

	private:
		std::string name_;

		//albedo value
		std::optional<TextureView> albedoMap_;
		glm::vec3 albedo_;

		//roughness value
		std::optional<TextureView> roughnessMap_;
		float roughness_;

		//metallic value
		std::optional<TextureView> metallicMap_;
		float metallic_;

		//normal mapping value
		std::optional<TextureView> normalMap_;

		//ambient occlusion value
		std::optional<TextureView> ambientOcclusionMap_;

		static std::vector<std::unique_ptr<ns::Texture>> textures;
		static TextureView addTexture(const std::string& directory, const aiString& path, aiTexture** const embeddedTextures, int numberOfEmbeddedTextures);
		static void removeTexture(const TextureView& view);
		
	};
}
