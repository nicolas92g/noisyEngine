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
