#include "Scene.h"

ns::Scene::Scene(
	DirectionalLight& dirLight,
	const std::vector<DrawableObject3d*>& entities,
	const std::vector<DrawableObject3d*>& stationaries,
	const std::vector<attenuatedLightBase_*>& lights)
	:
	entities_(entities),
	stationaries_(stationaries),
	lights_(lights),
	dirLight_(&dirLight)
{
	updateStationaries();
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

uint32_t ns::Scene::numStationaries()
{
	return static_cast<uint32_t>(stationaries_.size());
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
	for (const DrawableObject3d* stationary : stationaries_)
	{
		stationary->draw(shader);
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

void ns::Scene::updateStationaries()
{
	//update motionless Objects
	for (DrawableObject3d* motionLess : stationaries_)
	{
		motionLess->update();
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

void ns::Scene::addStationary(DrawableObject3d& object)
{
	addElement(&object, stationaries_);
}

void ns::Scene::removeStationary(DrawableObject3d& object)
{
	removeElement(&object, stationaries_);
}

void ns::Scene::clearStationaries()
{
	stationaries_.clear();
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

	if(other.stationaries_.size())
		stationaries_.insert(stationaries_.end(), other.stationaries_.begin(), other.stationaries_.end());
	
	if(other.lights_.size())
		lights_.insert(lights_.end(), other.lights_.begin(), other.lights_.end());
}

void ns::Scene::operator=(const Scene& other)
{
	entities_ = other.entities_;
	stationaries_ = other.stationaries_;
	lights_ = other.lights_;
	dirLight_ = other.dirLight_;
}

void ns::Scene::operator=(Scene&& other) noexcept
{
	entities_ = std::move(other.entities_);
	stationaries_ = std::move(other.stationaries_);
	lights_ = std::move(other.lights_);
	dirLight_ = other.dirLight_;
}

ns::Scene ns::operator+(Scene scene, const Scene& other)
{
	scene += other;
	return scene;
}
