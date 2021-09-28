#pragma once
#include <vector>
#include "Object2d.h"

namespace ns {
	class Renderer2d
	{
	public:
		Renderer2d(Window& win, const std::vector<const DrawableObject2d*>& entities = {});
		~Renderer2d();

		void render() const;

		void clear();
		void addEntity(DrawableObject2d& entity);
		void removeEntity(DrawableObject2d& entity);
		void setEntities(const std::vector<const DrawableObject2d*>& entities);

	private:
		ns::Window& win_;
		std::vector<const ns::DrawableObject2d*> entities_;
		Shader shader_;
	};
}
