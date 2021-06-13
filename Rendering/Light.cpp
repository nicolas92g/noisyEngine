#include "Light.h"

ns::LightBase_::LightBase_(const glm::vec3& color)
{
	color_ = color;
}

void ns::LightBase_::setColor(const glm::vec3& color)
{
	if (color.x < .0f || color.y < .0f || color.z < .0f) return;

	color_ = color;
}

glm::vec3 ns::LightBase_::color() const
{
	return color_;
}

ns::DirectionalLight::DirectionalLight(const glm::vec3& direction, const glm::vec3& color)
	: 
	LightBase_(color)
{
	setDirection(direction);
}

void ns::DirectionalLight::setDirection(const glm::vec3& direction)
{
	direction_ = glm::normalize(direction);
}

glm::vec3 ns::DirectionalLight::direction() const
{
	return direction_;
}

void ns::DirectionalLight::send(const Shader& shader) const
{
	shader.set("dirLight.direction", direction_);
	shader.set("dirLight.color", color_);
}

ns::PointLight::PointLight(const glm::vec3& position, float attenuation, const glm::vec3& color)
	:
	LightBase_(color),
	position_(position)
{
	setAttenuation(attenuation);
}

void ns::PointLight::setPosition(const glm::vec3& position)
{
	position_ = position;
}

void ns::PointLight::setAttenuation(float attenuationValue)
{
	attenuation_ = std::max(attenuationValue, .0f);
}

glm::vec3 ns::PointLight::position()
{
	return position_;
}

float ns::PointLight::attenuationValue()
{
	return attenuation_;
}

void ns::PointLight::send(const Shader& shader) const
{

}
