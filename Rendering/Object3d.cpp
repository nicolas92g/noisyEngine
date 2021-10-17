#include "Object3d.h"

#include <glm/gtx/transform.hpp>
#include <Utils/DebugLayer.h>

template<typename P, typename D>
unsigned ns::Object3d<P, D>::entityCount(0);

template<typename P, typename D>
std::unordered_map<std::string, ns::Object3d<P, D>*> objects;

template<typename P, typename D>
ns::Object3d<P, D>::Object3d<P, D>(const vec3p& position)
	:
	position_(position)
{
	name_ = "entity" + std::to_string(entityCount);
	entityCount++;
}

template<typename P, typename D>
ns::Object3d<P, D>::vec3p const& ns::Object3d<P, D>::position() const
{
	return position_;
}

template<typename P, typename D>
ns::Object3d<P, D>::vec3p ns::Object3d<P, D>::WorldPosition() const
{
	using namespace glm;
	//if there is no parent
	if (!parent_.has_value()) return position_;

	//if it is a geometric Object3d
	ns::GeometricObject3d<>* ptr = dynamic_cast<ns::GeometricObject3d<>*>(parent_.value());
	if (ptr) return vec3(ptr->modelMatrix() * vec4(position_, 1.0f));

	//if parent is an Object3d ot a DirectionalObject3d
	return parent_.value()->WorldPosition() + position_;
}

template<typename P, typename D>
const std::string& ns::Object3d<P, D>::name() const
{
	return name_;
}

template<typename P, typename D>
void ns::Object3d<P, D>::setPosition(const vec3p& position)
{
	position_ = position;
}

template<typename P, typename D>
void ns::Object3d<P, D>::setName(const std::string& name)
{
	name_ = name;
}

template<typename P, typename D>
void ns::Object3d<P, D>::setParent(Object3d* parent)
{
	if (parent == this) {
		Debug::get() << "error : try to parent an object with himself !\n";
		return;
	}
	if (parent)
		parent_ = parent;
}

template<typename P, typename D>
void ns::Object3d<P, D>::removeParent()
{
	position_ = WorldPosition();
	parent_.reset();
}

template<typename P, typename D>
bool ns::Object3d<P, D>::hasParent() const
{
	return parent_.has_value();
}

template<typename P, typename D>
ns::Object3d<P, D>* ns::Object3d<P, D>::parent() const
{
	return parent_.value_or(nullptr);
}

template<typename P, typename D>
YAML::Node ns::Object3d<P, D>::inputFromYAML(const std::string& filepath)
{
	using namespace YAML;

	Node conf = LoadFile(filepath);
	if (!conf) return conf;

	setPosition(conf[name_]["position"].as<glm::vec3>());
	return conf;
}

template<typename P, typename D>
bool ns::Object3d<P, D>::isGeometricObject3d()
{
	return dynamic_cast<ns::GeometricObject3d<P, D>*>(this);
}

template<typename P, typename D>
bool ns::Object3d<P, D>::isDirectionalObject3d()
{
	return dynamic_cast<ns::DirectionalObject3d<P, D>*>(this);
}

template<typename P, typename D>
ns::DirectionalObject3d<P, D>::DirectionalObject3d(const vec3p& position, const vec3d& direction)
	:
	Object3d<P, D>(position)
{
	setDirection(direction);
}

template<typename P, typename D>
void ns::DirectionalObject3d<P, D>::setDirection(const vec3d& direction)
{
	direction_ = glm::normalize(direction);
}

template<typename P, typename D>
const glm::vec<3, D>& ns::DirectionalObject3d<P, D>::direction() const
{
	if (!this->parent_.has_value()) return direction_;

	using namespace glm;
#	if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
	//if parent is a geometric object
	GeometricObject3d<P, D>* ptr = dynamic_cast<GeometricObject3d<P, D>*>(parent_.value());
	if (ptr) return vec3p(ptr->rotationMatrix() * vec4(direction_, 1));
#	endif

	//if parent is a directional object
	DirectionalObject3d<P, D>* ptr2 = dynamic_cast<DirectionalObject3d<P, D>*>(this->parent_.value());
	if (ptr2) {
		return ptr2->direction();
	}
		
	//else
	return direction_;
}

template<typename P, typename D>
ns::GeometricObject3d<P, D>::GeometricObject3d(const vec3p& position, const vec3p& scale, const vec3d& axis, D angle)
	:
	Object3d<P, D>(position),
	scale_(scale),
	axis_(axis),
	angle_(angle)
{
	update();
}

template<typename P, typename D>
void ns::GeometricObject3d<P, D>::update()
{
#if not NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
	static glm::mat4 translationMatrix_, scaleMatrix_, rotationMatrix_;
#endif // NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES

	translationMatrix_ = glm::translate(this->WorldPosition());
	scaleMatrix_ = glm::scale(scale_);
	rotationMatrix_ = glm::rotate(angle_, axis_);
	modelMatrix_ = translationMatrix_ * rotationMatrix_ * scaleMatrix_;
}

template<typename P, typename D>
void ns::GeometricObject3d<P, D>::setScale(const vec3p& newScale)
{
	scale_ = newScale;
}

template<typename P, typename D>
void ns::GeometricObject3d<P, D>::setRotation(const vec3d& axis, D angleInRadians)
{
	angle_ = angleInRadians;
	axis_ = glm::normalize(axis);
}

template<typename P, typename D>
const glm::vec<3, P>& ns::GeometricObject3d<P, D>::scale() const
{
	return scale_;
}

template<typename P, typename D>
const glm::vec<3, D>& ns::GeometricObject3d<P, D>::rotationAxis() const
{
	return axis_;
}

template<typename P, typename D>
D ns::GeometricObject3d<P, D>::rotationAngle() const
{
	return angle_;
}

template<typename P, typename D>
const glm::mat<4, 4, P>& ns::GeometricObject3d<P, D>::modelMatrix() const
{
	return modelMatrix_;
}


#if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
template<typename P, typename D>
const glm::mat<4, 4, P>& ns::GeometricObject3d<P, D>::translationMatrix() const
{
	return translationMatrix_;
}

template<typename P, typename D>
const glm::mat<4, 4, P>& ns::GeometricObject3d<P, D>::scaleMatrix() const
{
	return scaleMatrix_;
}

template<typename P, typename D>
const glm::mat<4, 4, P>& ns::GeometricObject3d<P, D>::rotationMatrix() const
{
	return rotationMatrix_;
}
#endif

//this function must not be used because it is use to fix some link issues
template<typename P, typename D>
void templateLinkFixerFunction_Object3d_() {
	_STL_REPORT_ERROR("the fix link function has been called");
	using namespace glm;
	//call all the functions that need to be linked well
	ns::Object3d<P, D> templatedObject1(vec3(0));
	ns::DirectionalObject3d<P, D> dirLight(vec3(0), vec3(0));
	dirLight.setDirection(vec3(0));
	dirLight.direction();
	ns::GeometricObject3d<P, D> templatedObject3(vec3(0), vec3(0), vec3(0), 0);
}

void LinkFixerFunction_Object3d_(){
	templateLinkFixerFunction_Object3d_<float, float>();
	templateLinkFixerFunction_Object3d_<double, float>();
	templateLinkFixerFunction_Object3d_<float, double>();
	templateLinkFixerFunction_Object3d_<double, double>();
}
