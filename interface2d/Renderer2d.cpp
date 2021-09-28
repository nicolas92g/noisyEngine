#include "Renderer2d.h"

#include "TexturedSquare.h"
#include <glm/gtx/transform.hpp>

ns::Renderer2d::Renderer2d(Window& win, const std::vector<const DrawableObject2d*>& entities)
	:
	win_(win),
	entities_(entities),
	shader_(NS_PATH"assets/shaders/main/interface2d.vert", NS_PATH"assets/shaders/main/interface2d.frag", nullptr, {}, true)
{
	TexturedSquare::createSquare();
}

ns::Renderer2d::~Renderer2d()
{
	TexturedSquare::destroySquare();
}

void ns::Renderer2d::render() const
{
	shader_.set("projection", glm::ortho<float>(0.f, win_.width(), 0.f, win_.height()));
	for (const auto& entity : entities_)
	{
		entity->draw(shader_);
	}
}

void ns::Renderer2d::clear()
{
	entities_.clear();
}

void ns::Renderer2d::addEntity(DrawableObject2d& entity)
{
	for (const auto& obj : entities_)
	{
		if (obj == &entity) return;
	}
	entities_.push_back(&entity);
}

void ns::Renderer2d::removeEntity(DrawableObject2d& entity)
{
	for (auto it = entities_.begin(); it != entities_.end(); ++it) {
		if(*it == &entity)
			entities_.erase(it);
	}
}

void ns::Renderer2d::setEntities(const std::vector<const DrawableObject2d*>& entities)
{
	entities_ = entities;
}
