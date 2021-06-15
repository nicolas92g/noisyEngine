#pragma once

//ns
#include "DrawableObject3d.h"
#include "Light.h"

//stl
#include <vector>

namespace ns {
	class Scene
	{
	public:
		Scene(
			const std::vector<DrawableObject3d*>& entities = {},
			const std::vector<DrawableObject3d*>& stationaries = {},
			const std::vector<LightBase_*>& lights = std::vector<LightBase_*>(0)
		);
		void sendLights(const ns::Shader& shader) const;
		void draw(const ns::Shader& shader) const;
		void update();

		void updateStationaries();

	protected:
		std::vector<DrawableObject3d*> entities_;		//movable Objects
		std::vector<DrawableObject3d*> stationaries_;	//motionless Objects
		std::vector<LightBase_*> lights_;				//lights Objects using polymorphism
	};
}
