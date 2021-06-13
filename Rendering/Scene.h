#pragma once

//ns
#include "Object3d.h"

//stl
#include <vector>

namespace ns {
	class Scene
	{
	public:
		Scene(
			const std::vector<Object3d*>& entities = std::vector<Object3d*>(0), 
			const std::vector<Object3d*>& stationaries = std::vector<Object3d*>(0));

		void draw(const ns::Shader& shader) const;
		void update();

		void updateStationaries();

	protected:
		std::vector<Object3d*> entities_;		//movable Objects
		std::vector<Object3d*> stationaries_;	//motionless Objects
	};
}
