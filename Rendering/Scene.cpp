#include "Scene.h"

ns::Scene::Scene(
	const std::vector<DrawableObject3d*>& entities,
	const std::vector<DrawableObject3d*>& stationaries,
	const std::vector<LightBase_*>& lights )
	:
	entities_(entities),
	stationaries_(stationaries),
	lights_(lights)
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

void ns::Scene::sendLights(const ns::Shader& shader) const
{
	clearLights();
	for (LightBase_* light : lights_)
	{
		light->send(shader);
	}
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

void ns::Scene::addLight(LightBase_& light)
{
	addElement(&light, lights_);
}

void ns::Scene::removeLight(LightBase_& light)
{
	removeElement(&light, lights_);
}

void ns::Scene::addLights(const std::vector<std::shared_ptr<LightBase_>>& lights)
{
	const size_t size = lights_.size();
	lights_.resize(size + lights.size());

	for (size_t i = size; i < lights_.size(); i++)
	{
		lights_[i] = lights[i].get();
	}
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
}

void ns::Scene::operator=(Scene&& other) noexcept
{
	entities_ = std::move(other.entities_);
	stationaries_ = std::move(other.stationaries_);
	lights_ = std::move(other.lights_);
}
