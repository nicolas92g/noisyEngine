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
	/**
	 * @brief this describe an object that allow to draw an object by reference but add a 3D position, scale and rotation
	 */
	class DrawableObject3d : public GeometricObject3d
	{
	public:
		/**
		 * @brief need a Drawable object (it is possible to create a custom Drawable object or to use for example the ns::Model class)
		 * \param model the object with a draw call
		 * \param position position where the object will be draw
		 * \param scale scale of the object 
		 * \param axis rotation axis of the object 
		 * \param angleInRadians angle of the rotation of the object
		 */
		DrawableObject3d(Drawable& model,
			const glm::vec3& position = glm::vec3(0),
			const glm::vec3& scale = glm::vec3(1),
			const glm::vec3& axis = {0, 1, 0},
			float angleInRadians = .0f);
		/**
		 * @brief new draw call that draw the object and send the position, scale and rotation before
		 * \param shader
		 */
		void draw(const Shader& shader) const;
		/**
		 * @brief do not use this (not finished)
		 * \return 
		 */
		const std::vector<std::shared_ptr<ns::LightBase_>>& getLights() const;

	protected:
		Drawable& model_;
		std::vector<std::shared_ptr<ns::LightBase_>> lights_;

		friend class Debug;
	};
}
