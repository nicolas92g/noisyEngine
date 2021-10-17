/*****************************************************************//**
 * \file   Light.h
 * \brief  hdr and gamma corrected light system 
 * (without quadratic attenuation and colors with values greater than ones)
 * 
 * \author Nicolas Guillot
 * \date   September 2021
 *********************************************************************/

#pragma once

#include "Shader.h"
#include "Object3d.h"

#include <glm/glm.hpp>

#include <optional>

#define NS_WHITE glm::vec3(1)
#define NS_BLACK glm::vec3(0)

namespace ns {
	/**
	 * @brief abstract class that describe a basic light with a color
	 * color values can be greater than one to indicate the power of the light
	 */
	class LightBase_
	{
	public:
		/**
		 * @brief basic constructor
		 * \param color
		 */
		LightBase_(const glm::vec3& color);
		/**
		 * @brief basic modifier
		 * \param color
		 */
		void setColor(const glm::vec3& color);
		/**
		 * @brief basic accessor
		 * \return 
		 */
		const glm::vec3& color() const;
		/**
		 * @brief for all types of lights, the send call put lights in a shader's uniform buffer
		 * you have to use the same shader until you call clearLights()
		 * \param the shader that will store the light
		 */
		virtual void send(const Shader&) = 0;

	protected:
		glm::vec3 color_;
	};
	/**
	 * @brief abstract class that describe a basic light with a color and an attenuation factor
	 */
	class attenuatedLightBase_ : public LightBase_
	{
	public:
		/**
		 * @brief basic contructor
		 * \param color
		 * \param attenuation
		 */
		attenuatedLightBase_(const glm::vec3& color, float attenuation);
		/**
		 * @brief basic modifier, the modified value is certified to be positive or null
		 * \param attenuationValue
		 */
		void setAttenuation(float attenuationValue);
		/**
		 * @brief basic accessor
		 * \return a positive or null value
		 */
		float attenuationValue();

	protected:
		float attenuation_;
	};
	/**
	 * @brief a light without attenuation that is only described by a direction and a color
	 */
	class DirectionalLight : public LightBase_, public DirectionalObject3d<>
	{
	public:
		/**
		 * @brief directional light constructor
		 * \param a normalized direction
		 * \param color
		 */
		DirectionalLight(const glm::vec3& direction = {.25f, -.25f, .5f}, const glm::vec3& color = NS_WHITE);
		/**
		 * @brief the directional light version of the send() method
		 * \param shader
		 */
		virtual void send(const Shader& shader) override;
		/**
		 * @brief return the counter of directional lights that has been sent
		 * \return 
		 */
		static uint32_t number();
		/**
		 * @brief reset the counter to zero
		 */
		static void clear();
		/**
		 * @brief return a static directional light that is suppossed to be infinitly black
		 * \return 
		 */
		static DirectionalLight& nullLight();

	protected:
		static DirectionalLight nullDirectionalLightObject;

	private:
		//dir lights counter (send() add one to this and clearLights() reset this to zero)
		static uint32_t number_;

		//remove the direction property
		using DirectionalObject3d<>::position_;
		using DirectionalObject3d<>::position;
		using DirectionalObject3d<>::setPosition;
	};
	/**
	 * @brief a point light is a light with a position, a attenuation factor and a color
	 */
	class PointLight : public attenuatedLightBase_, public Object3d<>
	{
	public:
		/**
		 * @brief create a pointlight
		 * \param position
		 * \param attenuation
		 * \param color
		 */
		PointLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE);
		/**
		 * @brief point light version of the send method
		 * \param shader
		 */
		virtual void send(const Shader& shader) override;
		/**
		 * @brief return the counter of points lights
		 * \return 
		 */
		static uint32_t number();
		/**
		 * @brief reset the points lights counter to zero
		 */
		static void clear();

	private:
		static uint32_t number_;
	};
	/**
	 * @brief almost the same as a pointlight but with a direction and some angles to allow the light to be 
	 * in a cone in the scene to simulate a spot 
	 * there is two angles, the inner angle is where the full power of the light end and where the smooth transition begin 
	 * the outer angle is where the transition end.
	 * https://learnopengl.com/Lighting/Light-casters
	 */
	class SpotLight : public attenuatedLightBase_, public DirectionalObject3d<>
	{
	public:
		/**
		 * @brief full constructor that allow to initialize all the properties
		 * \param position
		 * \param attenuation
		 * \param color
		 * \param direction
		 * \param innerAngle
		 * \param outerAngle
		 */
		SpotLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE,
			const glm::vec3& direction = {1, 0, 0}, float innerAngle = 16.0f, float outerAngle = 19.0f);
		/**
		 * @brief angles of the spot's cone
		 * set the internal values that are not the angles but their cosinus
		 * \param innerAngle
		 * \param outerAngle
		 */
		void setAngle(float innerAngle, float outerAngle);
		/**
		 * @brief spot version of the send() method
		 * \param shader
		 */
		virtual void send(const Shader& shader) override;
		/**
		 * @brief return the counter of spots
		 * \return 
		 */
		static uint32_t number();
		/**
		 * @brief reset to zero the spot's counter
		 */
		static void clear();

	protected:
		float innerCutOff_;
		float outerCutOff_;

	private:
		using Object3d<>::position_;
		static uint32_t number_;
	};

	/**
	 * @brief clear all the lights from the shaders (they are not suppressed but it allow to overwrite them by using send())
	 * all the lights counters is reset to zero
	 */
	void clearLights();
}
