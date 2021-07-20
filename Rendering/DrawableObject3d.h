#pragma once

//stl
#include <memory>
#include <vector>

//ns
#include "Shader.h"
#include "Drawable.h"
#include "Object3d.h"
#include "Light.h"

//glm
#include <glm/glm.hpp>

namespace ns {
	class DrawableObject3d : public GeometricObject3d
	{
	public:
		DrawableObject3d(Drawable& model,
			const glm::vec3& position = glm::vec3(0),
			const glm::vec3& scale = glm::vec3(1),
			const glm::vec3& axis = {0, 1, 0},
			float angleInRadians = .0f);

		void draw(const Shader& shader) const;
		const std::vector<std::shared_ptr<ns::LightBase_>>& getLights() const;

	protected:
		Drawable& model_;
		std::vector<std::shared_ptr<ns::LightBase_>> lights_;

		friend class Debug;
	};
}
