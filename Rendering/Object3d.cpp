#include "Object3d.h"

#include <glm/gtx/transform.hpp>

ns::Object3d::Object3d(const Drawable& model, glm::vec3 position, glm::vec3 scale, glm::vec3 rotationAxe, float angle)
	:
	model_(model),
	position_(position),
	scale_(scale),
	rotationAxe_(rotationAxe),
	rotationAngle_(angle)
{
	update();
}

void ns::Object3d::update()
{
	translationMatrix_ = glm::translate(position_);
	scaleMatrix_ = glm::scale(scale_);
	rotationMatrix_ = glm::rotate(rotationAngle_, rotationAxe_);
	modelMatrix_ = translationMatrix_ * rotationMatrix_ * scaleMatrix_;
}

void ns::Object3d::draw(const Shader& shader) const
{
	shader.set("model", modelMatrix_);
	model_.draw(shader);
}

void ns::Object3d::setPosition(const glm::vec3& newPosition)
{
	position_ = newPosition;
}

void ns::Object3d::setScale(const glm::vec3& newScale)
{
	scale_ = newScale;
}

void ns::Object3d::setRotation(const glm::vec3& axe, float angleInRadians)
{
	rotationAngle_ = angleInRadians;
	rotationAxe_ = axe;
}

glm::vec3 ns::Object3d::position() const
{
	return position_;
}

glm::vec3 ns::Object3d::scale() const
{
	return scale_;
}

glm::vec3 ns::Object3d::rotationAxe() const
{
	return rotationAxe_;
}

float ns::Object3d::rotationAngle() const
{
	return rotationAngle_;
}

glm::mat4 ns::Object3d::modelMatrix() const
{
	return modelMatrix_;
}

glm::mat4 ns::Object3d::translationMatrix() const
{
	return translationMatrix_;
}

glm::mat4 ns::Object3d::scaleMatrix() const
{
	return scaleMatrix_;
}

glm::mat4 ns::Object3d::rotationMatrix() const
{
	return rotationMatrix_;
}
