#include "Object3d.h"

#include <glm/gtx/transform.hpp>
#include <iostream>

ns::Object3d::Object3d(const glm::vec3& position)
	:
	position_(position)
{}

glm::vec3 ns::Object3d::position() const
{
	return position_;
}

glm::vec3 ns::Object3d::WorldPosition() const
{
	using namespace glm;
	//if there is no parent
	if (!parent_.has_value()) return position_;

	//if it is a geometric Object3d
	ns::GeometricObject3d* ptr = dynamic_cast<ns::GeometricObject3d*>(parent_.value());
	if (ptr) return vec3(ptr->modelMatrix() * vec4(position_, 1.0f));

	//if parent is an Object3d ot a DirectionalObject3d
	return parent_.value()->WorldPosition() + position_;
}

void ns::Object3d::setPosition(const glm::vec3& position)
{
	position_ = position;
}

void ns::Object3d::setParent(Object3d* parent)
{
	if (parent == this) {
		std::cerr << "error : try to parent an object with himself !\n";
		return;
	}
	parent_ = parent;
}

void ns::Object3d::removeParent()
{
	position_ = WorldPosition();
	parent_.reset();
}

bool ns::Object3d::hasParent() const
{
	return parent_.has_value();
}

ns::Object3d* ns::Object3d::parent() const
{
	return parent_.value_or(nullptr);
}

bool ns::Object3d::isGeometricObject3d()
{
	return dynamic_cast<ns::GeometricObject3d*>(this);
}

bool ns::Object3d::isDirectionalObject3d()
{
	return dynamic_cast<ns::DirectionalObject3d*>(this);
}

ns::DirectionalObject3d::DirectionalObject3d(const glm::vec3& position, const glm::vec3& direction)
	:
	Object3d(position)
{
	setDirection(direction);
}

void ns::DirectionalObject3d::setDirection(const glm::vec3& direction)
{
	direction_ = glm::normalize(direction);
}

glm::vec3 ns::DirectionalObject3d::direction()
{
	if (!parent_.has_value()) return direction_;

	using namespace glm;
	//if parent is a geometric object 
	GeometricObject3d* ptr = dynamic_cast<GeometricObject3d*>(parent_.value());
	if (ptr) return vec3(ptr->rotationMatrix() * vec4(direction_, 1));

	//if parent is a directional object
	DirectionalObject3d* ptr2 = dynamic_cast<DirectionalObject3d*>(parent_.value());
	if (ptr2) {
		return ptr2->direction();
	}
		
	//else
	return direction_;
}

ns::GeometricObject3d::GeometricObject3d(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& axis, float angle)
	:
	Object3d(position),
	scale_(scale),
	axis_(axis),
	angle_(angle)
{
	update();
}

void ns::GeometricObject3d::update()
{
	translationMatrix_ = glm::translate(WorldPosition());
	scaleMatrix_ = glm::scale(scale_);
	rotationMatrix_ = glm::rotate(angle_, axis_);
	modelMatrix_ = translationMatrix_ * rotationMatrix_ * scaleMatrix_;
}

void ns::GeometricObject3d::setScale(const glm::vec3& newScale)
{
	scale_ = newScale;
}

void ns::GeometricObject3d::setRotation(const glm::vec3& axe, float angleInRadians)
{
	angle_ = angleInRadians;
	axis_ = axe;
}

glm::vec3 ns::GeometricObject3d::scale() const
{
	return scale_;
}

glm::vec3 ns::GeometricObject3d::rotationAxe() const
{
	return axis_;
}

float ns::GeometricObject3d::rotationAngle() const
{
	return angle_;
}

const glm::mat4& ns::GeometricObject3d::modelMatrix() const
{
	return modelMatrix_;
}

const glm::mat4& ns::GeometricObject3d::translationMatrix() const
{
	return translationMatrix_;
}

const glm::mat4& ns::GeometricObject3d::scaleMatrix() const
{
	return scaleMatrix_;
}

const glm::mat4& ns::GeometricObject3d::rotationMatrix() const
{
	return rotationMatrix_;
}