/*****************************************************************//**
 * \file   Material.h
 * \brief  material system of noisy
 * 
 * \author Nicolas Guillot
 * \date   September 2021
 *********************************************************************/

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

//OpenFBX
#include <ofbx.h>

namespace ns {
	/**
	 * @brief store a material made for a physically based rendering system.
	 * the components are : 
	 * - roughness					(a float or a one channel texture)		
	 * - metallic					(a float or a one channel texture)
	 * - albedo						(a vec3 or a three channel texture)
	 * - emission					(a vec3 or a three channel texture)
	 * - emission strength			(a float)
	 * - ambient occlusion			(a one channel texture or 1)
	 * - normal mapping				(a three channel texture or nothing)
	 */
	class Material
	{
	public:
		Material(const glm::vec3& albedo = glm::vec3(.5f), float roughness = .1f, float metallic = .01f, const glm::vec3& emission = glm::vec3(0.f), const std::string& exportName = "none");
		Material(aiMaterial* mtl, const std::string& texturesDirectory, const std::string& exportName = "none");
		Material(const ofbx::Material* mtl, const std::string& texturesDirectory, const std::string& exportName = "none");
		/**
		 * @brief This constructor is able to read some YAML files.
		 * It can read a constant value or a texture path.
		 * The texture path is relative to the material path 
		 * so it is recommanded to have the texture in the same directory as the material file.
		 * an example will be :
		 * ---
		 * albedo : albedo.png
		 * emission :
		 *  - 3
		 *  - 1
		 *  - 2
		 * roughness : .6
		 * ---
		 * \param filepath
		 */
		Material(const std::string& YAMLfilepath);

		void exportYAML() const;
		void importYAML();

		void bind(const ns::Shader& shader) const;
		const std::string& name() const;

		void setEmissionConstant(const glm::vec3& emission);
		void setRoughnessConstant(float roughness);
		void setMetallicConstant(float metallic);
		void setAlbedoColor(const glm::vec3& color);

		float roughness() const;
		glm::vec3 emission() const;
		float metallic() const;
		glm::vec3 albedo() const;

		void setAlbedoTexture(const TextureView& texture);
		void setRoughnessTexture(const TextureView& texture);
		void setMetallicTexture(const TextureView& texture);
		void setEmissionTexture(const TextureView& texture);
		void setNormalTexture(const TextureView& texture);
		void setAmbientOcclusionTexture(const TextureView& texture);

		void removeAlbedoTexture();
		void removeRoughnessTexture();
		void removeMetallicTexture();
		void removeEmissionTexture();
		void removeNormalTexture();
		void removeAmbientOcclusionTexture();

		std::string directory() const;

		static void convertAiColor4dToVec3(const aiColor4D& ai4d, glm::vec3& vec);
		static void describeMaterial(aiMaterial* mtl);
		static void displayTextures(const ofbx::Material* mtl);

		//material textures usage:
		static void clearTextures();
		static bool describeMaterialsWhenCreate;
		static const Material& getDefault();

	private:
		//name of the material
		std::string name_;
		//file where the material has been loaded
		std::string filepath_;

		//albedo value
		std::optional<TextureView> albedoMap_;
		glm::vec3 albedo_;

		//roughness value
		std::optional<TextureView> roughnessMap_;
		float roughness_;

		//metallic value
		std::optional<TextureView> metallicMap_;
		float metallic_;

		//emission value
		std::optional<TextureView> emissionMap_;
		float emissionStrength_;
		glm::vec3 emission_;

		//normal mapping value
		std::optional<TextureView> normalMap_;

		//ambient occlusion value
		std::optional<TextureView> ambientOcclusionMap_;

		static std::vector<std::unique_ptr<ns::Texture>> textures;
		static TextureView addTexture(const std::string& directory, const std::string& path);
		static void removeTexture(const TextureView& view);

		static Material defaultMaterial;

		friend class Debug;
	};
}
