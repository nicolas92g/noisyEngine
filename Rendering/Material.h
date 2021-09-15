/*****************************************************************//**
 * \file   Material.h
 * \brief  pbr material system
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
		 * 
		 * emission :
		 *  - 3
		 *  - 1
		 *  - 2
		 * 
		 * roughness : .6
		 * ---
		 * \param filepath
		 */
		Material(const std::string& YAMLfilepath);
		/**
		 * @brief export the material in the file located at filepath_
		 */
		void exportYAML() const;
		/**
		 * @brief load the material from the file located at filepath_
		 */
		void importYAML();
		/**
		 * @brief send the material to the shader
		 * \param shader
		 */
		void bind(const ns::Shader& shader) const;
		/**
		 * @brief return the nale of the material
		 * \return 
		 */
		const std::string& name() const;
		/**
		 * @brief set the emission, if a texture is used for this property this will be useless, remove the texture first
		 * \param emission
		 */
		void setEmissionConstant(const glm::vec3& emission);
		/**
		 * @brief set the roughness, if a texture is used for this property this will be useless, remove the texture first
		 * \param roughness
		 */
		void setRoughnessConstant(float roughness);
		/**
		 * @brief set the metalness, if a texture is used for this property this will be useless, remove the texture first
		 * \param metallic
		 */
		void setMetallicConstant(float metallic);
		/**
		 * @brief set the albedo, if a texture is used for this property this will be useless, remove the texture first
		 * \param color
		 */
		void setAlbedoColor(const glm::vec3& color);
		/**
		 * @brief return the current roughness property of this material
		 * \return 
		 */
		float roughness() const;
		/**
		 * @brief return the current emssion property of this material
		 * \return 
		 */
		glm::vec3 emission() const;
		/**
		 * @brief return the current metallic property of this material
		 * \return 
		 */
		float metallic() const;
		/**
		 * @brief return the current albedo property of this material
		 * \return 
		 */
		glm::vec3 albedo() const;
		/**
		 * @brief set a texture for a material property and remove the old texture if there is one
		 * \param texture
		 */
		void setAlbedoTexture(const TextureView& texture);
		/**
		 * @brief set a texture for a material property and remove the old texture if there is one
		 * \param texture
		 */
		void setRoughnessTexture(const TextureView& texture);
		/**
		 * @brief set a texture for a material property and remove the old texture if there is one
		 * \param texture
		 */
		void setMetallicTexture(const TextureView& texture);
		/**
		 * @brief set a texture for a material property and remove the old texture if there is one
		 * \param texture
		 */
		void setEmissionTexture(const TextureView& texture);
		/**
		 * @brief set a texture for a material property and remove the old texture if there is one
		 * \param texture
		 */
		void setNormalTexture(const TextureView& texture);
		/**
		 * @brief set a texture for a material property and remove the old texture if there is one
		 * \param texture
		 */
		void setAmbientOcclusionTexture(const TextureView& texture);
		/**
		 * @brief remove a texture from the property, after that, this material property will be defined by a constant
		 */
		void removeAlbedoTexture();
		/**
		 * @brief remove a texture from the property, after that, this material property will be defined by a constant
		 */
		void removeRoughnessTexture();
		/**
		 * @brief remove a texture from the property, after that, this material property will be defined by a constant
		 */
		void removeMetallicTexture();
		/**
		 * @brief remove a texture from the property, after that, this material property will be defined by a constant
		 */
		void removeEmissionTexture();
		/**
		 * @brief remove a texture from the property, after that, this material property will be defined by a constant
		 */
		void removeNormalTexture();
		/**
		 * @brief remove a texture from the property, after that, this material property will be defined by a constant
		 */
		void removeAmbientOcclusionTexture();
		/**
		 * @brief allow to know the directory of the file from which this material has been imported
		 * \return 
		 */
		std::string directory() const;
		/**
		 * @brief util
		 * \param ai4d
		 * \param vec
		 */
		static void convertAiColor4dToVec3(const aiColor4D& ai4d, glm::vec3& vec);
		/**
		 * @brief for debug purpose, log the material
		 * \param mtl
		 */
		static void describeMaterial(aiMaterial* mtl);
		/**
		 * @brief debug purpose
		 * \param mtl
		 */
		static void displayTextures(const ofbx::Material* mtl);

		//material textures usage:
		/**
		 * @brief unload all the materials textures, destroy all your materials before !
		 */
		static void clearTextures();
		static bool describeMaterialsWhenCreate;
		/**
		 * @brief return a default material
		 * \return 
		 */
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
