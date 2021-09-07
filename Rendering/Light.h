#pragma once

#include "Shader.h"
#include "Object3d.h"

#include <glm/glm.hpp>

#include <optional>

#define NS_WHITE glm::vec3(1)
#define NS_BLACK glm::vec3(0)

namespace ns {
	class LightBase_
	{
	public:
		LightBase_(const glm::vec3& color);

		void setColor(const glm::vec3& color);
		glm::vec3 color() const;

		virtual void send(const Shader&) = 0;

	protected:
		glm::vec3 color_;
	};

	class attenuatedLightBase_ : public LightBase_
	{
	public:
		attenuatedLightBase_(const glm::vec3& color, float attenuation);

		void setAttenuation(float attenuationValue);
		float attenuationValue();

	protected:
		float attenuation_;
	};

	class DirectionalLight : public LightBase_, public DirectionalObject3d
	{
	public:
		DirectionalLight(const glm::vec3& direction = {.25f, -.25f, .5f}, const glm::vec3& color = NS_WHITE);

		virtual void send(const Shader& shader) override;

		static uint32_t number();
		static void clear();
		static DirectionalLight& nullLight();

	protected:
		static DirectionalLight nullDirectionalLightObject;

	private:
		static uint32_t number_;

		using DirectionalObject3d::position_;
		using DirectionalObject3d::position;
		using DirectionalObject3d::setPosition;
	};

	class PointLight : public attenuatedLightBase_, public Object3d
	{
	public:
		PointLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE);

		virtual void send(const Shader& shader) override;

		static uint32_t number();
		static void clear();

	private:
		static uint32_t number_;
	};

	class SpotLight : public attenuatedLightBase_, public DirectionalObject3d
	{
	public:
		SpotLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE,
			const glm::vec3& direction = {1, 0, 0}, float innerAngle = 16.0f, float outerAngle = 19.0f);

		void setAngle(float innerAngle, float outerAngle);

		virtual void send(const Shader& shader) override;
		
		static uint32_t number();
		static void clear();

	protected:
		float innerCutOff_;
		float outerCutOff_;

	private:
		using Object3d::position_;
		static uint32_t number_;
	};

	/**
	 * @brief clear all the lights (they are not suppressed but it allow to overwrite them by using send())
	 */
	void clearLights();
}
