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

		virtual void send(const Shader&) const = 0;

	protected:
		glm::vec3 color_;
	};

	class DirectionalLight : public LightBase_
	{
	public:
		DirectionalLight(const glm::vec3& direction = {.25f, -.25f, .5f}, const glm::vec3& color = NS_WHITE);

		void setDirection(const glm::vec3& direction);
		glm::vec3 direction() const;

		virtual void send(const Shader& shader) const override;

		static uint32_t number();
		static void clear();

	protected:
		glm::vec3 direction_;

	private:
		static uint32_t number_;
	};

	class PointLight : public LightBase_
	{
	public:
		PointLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE);

		void setPosition(const glm::vec3& position);
		void setAttenuation(float attenuationValue);
		void setParent(const Object3d* parent);
		void removeParent();

		glm::vec3 position();
		float attenuationValue();

		virtual void send(const Shader& shader) const override;

		static uint32_t number();
		static void clear();

	protected:
		glm::vec3 position_;
		float attenuation_;
		std::optional<const Object3d*> parent_;

	private:
		static uint32_t number_;
	};

	class SpotLight : public PointLight
	{
	public:
		SpotLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE,
			const glm::vec3& direction = {1, 0, 0}, float innerAngle = 16.0f, float outerAngle = 19.0f);

		void setAngle(float innerAngle, float outerAngle);
		void setDirection(const glm::vec3& direction);

		virtual void send(const Shader& shader) const override;
		
		static uint32_t number();
		static void clear();

	protected:
		glm::vec3 direction_;
		float innerCutOff_;
		float outerCutOff_;

	private:
		static uint32_t number_;
	};

	/**
	 * @brief clear all the lights (they are not suppressed but it allow to overwrite them by using send())
	 */
	void clearLights();
}
