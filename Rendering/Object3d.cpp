#include "Object3d.h"

#include <glm/gtx/transform.hpp>
#include <Utils/DebugLayer.h>

unsigned ns::Object3d::entityCount(0);
std::unordered_map < std::string, ns::Object3d* > objects;

ns::Object3d::Object3d(const glm::vec3& position)
	:
	position_(position)
{
	name_ = "entity" + std::to_string(entityCount);
	entityCount++;
}

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

const std::string& ns::Object3d::name() const
{
	return name_;
}

void ns::Object3d::setPosition(const glm::vec3& position)
{
	position_ = position;
}

void ns::Object3d::setName(const std::string& name)
{
	name_ = name;
}

void ns::Object3d::setParent(Object3d* parent)
{
	if (parent == this) {
		Debug::get() << "error : try to parent an object with himself !\n";
		return;
	}
	if (parent)
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

YAML::Node ns::Object3d::inputFromYAML(const std::string& filepath)
{
	using namespace YAML;

	Node conf = LoadFile(filepath);
	if (!conf) return conf;

	setPosition(conf[name_]["position"].as<glm::vec3>());
	return conf;
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

glm::vec3 ns::DirectionalObject3d::direction() const
{
	if (!parent_.has_value()) return direction_;

	using namespace glm;
#	if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
	//if parent is a geometric object
	GeometricObject3d* ptr = dynamic_cast<GeometricObject3d*>(parent_.value());
	if (ptr) return vec3(ptr->rotationMatrix() * vec4(direction_, 1));
#	endif

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
#if not NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
	static glm::mat4 translationMatrix_, scaleMatrix_, rotationMatrix_;
#endif // NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES

	translationMatrix_ = glm::translate(WorldPosition());
	scaleMatrix_ = glm::scale(scale_);
	rotationMatrix_ = glm::rotate(angle_, axis_);
	modelMatrix_ = translationMatrix_ * rotationMatrix_ * scaleMatrix_;
}

void ns::GeometricObject3d::setScale(const glm::vec3& newScale)
{
	scale_ = newScale;
}

void ns::GeometricObject3d::setRotation(const glm::vec3& axis, float angleInRadians)
{
	angle_ = angleInRadians;
	axis_ = glm::normalize(axis);
}

glm::vec3 ns::GeometricObject3d::scale() const
{
	return scale_;
}

glm::vec3 ns::GeometricObject3d::rotationAxis() const
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

#if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
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
#endif