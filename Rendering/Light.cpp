#include "Light.h"

uint32_t ns::DirectionalLight::number_(0);
uint32_t ns::PointLight::number_(0);
uint32_t ns::SpotLight::number_(0);

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

//--------------------------------
//	    directional light
//--------------------------------

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
	char name[18], buffer[40];
	sprintf_s(name, "dirLights[%d]", number_);

	sprintf_s(buffer, "%s.direction", name);
 	shader.set(buffer, direction_);

	sprintf_s(buffer, "%s.color", name);
	shader.set(buffer, color_);

	number_++;
}

uint32_t ns::DirectionalLight::number()
{
	return number_;
}

void ns::DirectionalLight::clear()
{
	number_ = 0;
}

//--------------------------------
//	      point light
//--------------------------------

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

void ns::PointLight::setParent(const Object3d* parent)
{
	parent_ = parent;
}

void ns::PointLight::removeParent()
{
	parent_.reset();
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
	char name[20], buffer[42];
	sprintf_s(name, "pointLights[%d]", number_);

	sprintf_s(buffer, "%s.position", name);
	shader.set(buffer, position_);

	sprintf_s(buffer, "%s.color", name);
	shader.set(buffer, color_);

	sprintf_s(buffer, "%s.attenuation", name);
	shader.set(buffer, attenuation_);

	number_++;
}

uint32_t ns::PointLight::number()
{
	return number_;
}

void ns::PointLight::clear()
{
	number_ = 0;
}

//--------------------------------
//	        spot light
//--------------------------------

ns::SpotLight::SpotLight(const glm::vec3& position, float attenuation, const glm::vec3& color, const glm::vec3& direction, float innerAngle, float outerAngle)
	:
	PointLight(position, attenuation, color)
{
	setAngle(innerAngle, outerAngle);
	setDirection(direction);
}

void ns::SpotLight::setAngle(float innerAngle, float outerAngle)
{
	innerCutOff_ = glm::cos(glm::radians(innerAngle));
	outerCutOff_ = glm::cos(glm::radians(outerAngle));
}

void ns::SpotLight::setDirection(const glm::vec3& direction)
{
	direction_ = glm::normalize(-direction);
}

void ns::SpotLight::send(const Shader& shader) const
{
	char name[20], buffer[42];
	sprintf_s(name, "spotLights[%d]", number_);

	sprintf_s(buffer, "%s.position", name);
	shader.set(buffer, position_);

	sprintf_s(buffer, "%s.color", name);
	shader.set(buffer, color_);

	sprintf_s(buffer, "%s.attenuation", name);
	shader.set(buffer, attenuation_);

	sprintf_s(buffer, "%s.direction", name);
	shader.set(buffer, direction_);

	sprintf_s(buffer, "%s.innerCutOff", name);
	shader.set(buffer, innerCutOff_);

	sprintf_s(buffer, "%s.outerCutOff", name);
	shader.set(buffer, outerCutOff_);

	number_++;
}

uint32_t ns::SpotLight::number()
{
	return number_;
}

void ns::SpotLight::clear()
{
	number_ = 0;
}

void ns::clearLights()
{
	PointLight::clear();
	DirectionalLight::clear();
	SpotLight::clear();
}
