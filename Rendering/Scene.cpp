#include "Scene.h"

template<typename P, typename D>
ns::Scene<P,D>::Scene(
	DirectionalLight& dirLight,
	const std::vector<DrawableObject3d<P, D>*>& entities,
	const std::vector<DrawableObject3d<P, D>*>& statics,
	const std::vector<attenuatedLightBase_*>& lights)
	:
	entities_(entities),
	statics_(statics),
	lights_(lights),
	dirLight_(&dirLight)
{
	updateStatics();
}

template<typename P, typename D>
ns::Scene<P, D>::Scene(const Scene<P, D>& other)
{
	*this = other;
}

template<typename P, typename D>
ns::Scene<P, D>::Scene(Scene<P, D>&& other) noexcept
{
	*this = other;
}

template<typename P, typename D>
uint32_t ns::Scene<P, D>::numEntities()
{
	return static_cast<uint32_t>(entities_.size());
}

template<typename P, typename D>
uint32_t ns::Scene<P, D>::numStatics()
{
	return static_cast<uint32_t>(statics_.size());
}

template<typename P, typename D>
uint32_t ns::Scene<P, D>::numLights()
{
	return static_cast<uint32_t>(lights_.size());
}

template<typename P, typename D>
void ns::Scene<P, D>::sendLights(const ns::Shader& shader) const
{
	clearLights();
	for (attenuatedLightBase_* light : lights_)
	{
		light->send(shader);
	}

	dirLight_->send(shader);
}

template<typename P, typename D>
void ns::Scene<P, D>::draw(const ns::Shader& shader) const
{
	glDepthFunc(GL_LESS);

	//draw motionless Objects
	for (const DrawableObject3d<P, D>* staticObject : statics_)
	{
		staticObject->draw(shader);
	}

	//draw entities
	for (const DrawableObject3d<P, D>* entity : entities_)
	{
		entity->draw(shader);
	}
}

template<typename P, typename D>
void ns::Scene<P, D>::update()
{
	//update entities
	for (DrawableObject3d<P, D>* entity : entities_)
	{
		entity->update();
	}
}

template<typename P, typename D>
void ns::Scene<P, D>::updateStatics()
{
	//update motionless Objects
	for (DrawableObject3d<P, D>* staticObject : statics_)
	{
		staticObject->update();
	}
}

template<typename P, typename D>
void ns::Scene<P, D>::addEntity(DrawableObject3d<P, D>& object)
{
	addElement(&object, entities_);
}

template<typename P, typename D>
void ns::Scene<P, D>::removeEntity(DrawableObject3d<P, D>& object)
{
	removeElement(&object, entities_);
}

template<typename P, typename D>
void ns::Scene<P, D>::clearEntities()
{
	entities_.clear();
}

template<typename P, typename D>
void ns::Scene<P, D>::addStatic(DrawableObject3d<P, D>& object)
{
	addElement(&object, statics_);
}

template<typename P, typename D>
void ns::Scene<P, D>::removeStatic(DrawableObject3d<P, D>& object)
{
	removeElement(&object, statics_);
}

template<typename P, typename D>
void ns::Scene<P, D>::clearStatics()
{
	statics_.clear();
}

template<typename P, typename D>
void ns::Scene<P, D>::addLight(attenuatedLightBase_& light)
{
	addElement(&light, lights_);
}

template<typename P, typename D>
void ns::Scene<P, D>::removeLight(attenuatedLightBase_& light)
{
	removeElement(&light, lights_);
}

template<typename P, typename D>
void ns::Scene<P, D>::addLights(const std::vector<std::shared_ptr<attenuatedLightBase_>>& lights)
{
	const size_t size = lights_.size();
	lights_.resize(size + lights.size());

	for (size_t i = size; i < lights_.size(); i++)
	{
		//add light
		lights_[i] = lights[i].get();
	}
}

template<typename P, typename D>
void ns::Scene<P, D>::setDirectionalLight(DirectionalLight& dirLight)
{
	dirLight_ = &dirLight;
}

template<typename P, typename D>
ns::DirectionalLight& ns::Scene<P, D>::directionalLight()
{
	return *dirLight_;
}

template<typename P, typename D>
const ns::DirectionalLight& ns::Scene<P, D>::getDirectionalLight() const
{
	return *dirLight_;
}

template<typename P, typename D>
void ns::Scene<P, D>::operator+=(const Scene<P, D>& other)
{
	if(other.entities_.size())
		entities_.insert(entities_.end(), other.entities_.begin(), other.entities_.end());

	if(other.statics_.size())
		statics_.insert(statics_.end(), other.statics_.begin(), other.statics_.end());
	
	if(other.lights_.size())
		lights_.insert(lights_.end(), other.lights_.begin(), other.lights_.end());
}

template<typename P, typename D>
void ns::Scene<P, D>::operator=(const Scene<P, D>& other)
{
	entities_ = other.entities_;
	statics_ = other.statics_;
	lights_ = other.lights_;
	dirLight_ = other.dirLight_;
}

template<typename P, typename D>
void ns::Scene<P, D>::operator=(Scene<P, D>&& other) noexcept
{
	entities_ = std::move(other.entities_);
	statics_ = std::move(other.statics_);
	lights_ = std::move(other.lights_);
	dirLight_ = other.dirLight_;
}

template<typename P, typename D>
template<typename T>
void ns::Scene<P, D>::addElement(T* element, std::vector<T*>& arr)
{
	for (const T* obj : arr) if (obj == element) return;
	arr.push_back(element);
}


template<typename P, typename D>
template<typename T>
void ns::Scene<P, D>::removeElement(T* element, std::vector<T*>& arr)
{
	for (auto it = arr.begin(); it != arr.end(); ++it)
		if (*it == element) arr.erase(it);
}

template<typename P, typename D>
void templateFixLinkFunction_Scene_(){
	_STL_REPORT_ERROR("the fix link function has been called !");
	using namespace ns;
	DrawableObject3d<P, D>* d = nullptr;
	Shader* s = nullptr;
	Scene<P, D> scene(*new DirectionalLight());
	Scene<P, D> scene2(scene);
	scene.update();
	scene.addEntity(*d);
	scene.addStatic(*d);
	scene.sendLights(*s);
	scene.draw(*s);
	scene.getDirectionalLight();

}

void FixLinkFunction_Scene_() {
	templateFixLinkFunction_Scene_<float, float>();
	templateFixLinkFunction_Scene_<double, float>();
	templateFixLinkFunction_Scene_<float, double>();
	templateFixLinkFunction_Scene_<double, double>();
}
