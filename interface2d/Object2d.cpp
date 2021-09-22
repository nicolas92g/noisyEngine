#include "Object2d.h"

#include <glm/gtx/transform.hpp>

ns::Object2d::Object2d(const glm::vec2& position, const glm::vec2& scale, float rotationAngle, const glm::vec3& rotationAxis) : 
	position_(position),
	scale_(scale),
	rotationAngle_(rotationAngle),
	rotationAxis_(glm::normalize(rotationAxis))
{
	update();
}

const glm::vec2& ns::Object2d::position() const
{
	return position_;
}

const glm::vec2& ns::Object2d::scale() const
{
	return scale_;
}

float ns::Object2d::rotationAngle() const
{
	return rotationAngle_;
}

const glm::vec3& ns::Object2d::rotationAxis() const
{
	return rotationAxis_;
}

void ns::Object2d::setPosition(const glm::vec2& position)
{
	position_ = position;
}

void ns::Object2d::setScale(const glm::vec2& scale)
{
	scale_ = scale;
}

void ns::Object2d::setRotation(float angle, const glm::vec3& axis)
{
	rotationAngle_ = angle;
	rotationAxis_ = glm::normalize(axis);
}

void ns::Object2d::update()
{
	static glm::mat4 translation, rotation, scaling;

	translation = glm::translate(glm::vec3(position_, 0));
	rotation = glm::rotate(rotationAngle_, rotationAxis_);
	scaling = glm::scale(glm::vec3(scale_, 1));

	model_ = translation * rotation * scaling;
}

const glm::mat4& ns::Object2d::modelMatrix()
{
	return model_;
}

ns::DrawableObject2d::DrawableObject2d(Drawable& obj, const glm::vec2& pos, const glm::vec2& scale, float angle, const glm::vec3& axis) 
	:
	Object2d(pos, scale, angle, axis),
	obj_(obj)
{}

void ns::DrawableObject2d::draw(const ns::Shader& shader) const
{
	shader.set("model", model_);
	obj_.draw(shader);
}
