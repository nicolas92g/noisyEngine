#include "Scene.h"

ns::Scene::Scene(
	DirectionalLight& dirLight,
	const std::vector<DrawableObject3d*>& entities,
	const std::vector<DrawableObject3d*>& statics,
	const std::vector<attenuatedLightBase_*>& lights)
	:
	entities_(entities),
	statics_(statics),
	lights_(lights),
	dirLight_(&dirLight)
{
	updateStatics();
}

ns::Scene::Scene(const Scene& other)
{
	*this = other;
}

ns::Scene::Scene(Scene&& other) noexcept
{
	*this = other;
}

uint32_t ns::Scene::numEntities()
{
	return static_cast<uint32_t>(entities_.size());
}

uint32_t ns::Scene::numStatics()
{
	return static_cast<uint32_t>(statics_.size());
}

uint32_t ns::Scene::numLights()
{
	return static_cast<uint32_t>(lights_.size());
}

void ns::Scene::sendLights(const ns::Shader& shader) const
{
	clearLights();
	for (attenuatedLightBase_* light : lights_)
	{
		light->send(shader);
	}

	dirLight_->send(shader);
}

void ns::Scene::draw(const ns::Shader& shader) const
{
	glDepthFunc(GL_LESS);

	//draw motionless Objects
	for (const DrawableObject3d* staticObject : statics_)
	{
		staticObject->draw(shader);
	}

	//draw entities
	for (const DrawableObject3d* entity : entities_)
	{
		entity->draw(shader);
	}
}

void ns::Scene::update()
{
	//update entities
	for (DrawableObject3d* entity : entities_)
	{
		entity->update();
	}
}

void ns::Scene::updateStatics()
{
	//update motionless Objects
	for (DrawableObject3d* staticObject : statics_)
	{
		staticObject->update();
	}
}

void ns::Scene::addEntity(DrawableObject3d& object)
{
	addElement(&object, entities_);
}

void ns::Scene::removeEntity(DrawableObject3d& object)
{
	removeElement(&object, entities_);
}

void ns::Scene::clearEntities()
{
	entities_.clear();
}

void ns::Scene::addStatic(DrawableObject3d& object)
{
	addElement(&object, statics_);
}

void ns::Scene::removeStatic(DrawableObject3d& object)
{
	removeElement(&object, statics_);
}

void ns::Scene::clearStatics()
{
	statics_.clear();
}

void ns::Scene::addLight(attenuatedLightBase_& light)
{
	addElement(&light, lights_);
}

void ns::Scene::removeLight(attenuatedLightBase_& light)
{
	removeElement(&light, lights_);
}

void ns::Scene::addLights(const std::vector<std::shared_ptr<attenuatedLightBase_>>& lights)
{
	const size_t size = lights_.size();
	lights_.resize(size + lights.size());

	for (size_t i = size; i < lights_.size(); i++)
	{
		//add light
		lights_[i] = lights[i].get();
	}
}

void ns::Scene::setDirectionalLight(DirectionalLight& dirLight)
{
	dirLight_ = &dirLight;
}

ns::DirectionalLight& ns::Scene::directionalLight()
{
	return *dirLight_;
}

void ns::Scene::operator+=(const Scene& other)
{
	if(other.entities_.size())
		entities_.insert(entities_.end(), other.entities_.begin(), other.entities_.end());

	if(other.statics_.size())
		statics_.insert(statics_.end(), other.statics_.begin(), other.statics_.end());
	
	if(other.lights_.size())
		lights_.insert(lights_.end(), other.lights_.begin(), other.lights_.end());
}

void ns::Scene::operator=(const Scene& other)
{
	entities_ = other.entities_;
	statics_ = other.statics_;
	lights_ = other.lights_;
	dirLight_ = other.dirLight_;
}

void ns::Scene::operator=(Scene&& other) noexcept
{
	entities_ = std::move(other.entities_);
	statics_ = std::move(other.statics_);
	lights_ = std::move(other.lights_);
	dirLight_ = other.dirLight_;
}

ns::Scene ns::operator+(Scene scene, const Scene& other)
{
	scene += other;
	return scene;
}
