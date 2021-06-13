#include "Scene.h"

ns::Scene::Scene(const std::vector<Object3d*>& entities, const std::vector<Object3d*>& stationaries)
	:
	entities_(entities),
	stationaries_(stationaries)
{
	updateStationaries();
}

void ns::Scene::draw(const ns::Shader& shader) const
{
	//draw motionless Objects
	for (const Object3d* stationary : stationaries_)
	{
		stationary->draw(shader);
	}

	//draw entities
	for (const Object3d* entity : entities_)
	{
		entity->draw(shader);
	}
}

void ns::Scene::update()
{
	//update entities
	for (Object3d* entity : entities_)
	{
		entity->update();
	}
}

void ns::Scene::updateStationaries()
{
	//update motionless Objects
	for (Object3d* motionLess : stationaries_)
	{
		motionLess->update();
	}
}
