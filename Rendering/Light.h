#pragma once

#include "Shader.h"

#include <glm/glm.hpp>

#define NS_WHITE glm::vec3(1)
#define NS_BLACK glm::vec3(0)

namespace ns {
	class LightBase_
	{
	public:
		LightBase_(const glm::vec3& color);

		virtual void setColor(const glm::vec3& color);
		virtual glm::vec3 color() const;
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

	protected:
		glm::vec3 direction_;
	};

	class PointLight : public LightBase_
	{
	public:
		PointLight(const glm::vec3& position = NS_BLACK, float attenuation = .2f, const glm::vec3& color = NS_WHITE);

		void setPosition(const glm::vec3& position);
		void setAttenuation(float attenuationValue);

		glm::vec3 position();
		float attenuationValue();

		virtual void send(const Shader& shader) const override;

	protected:
		glm::vec3 position_;
		float attenuation_;
	};
}