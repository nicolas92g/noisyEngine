#pragma once

//ns
#include "Shader.h"
#include "Drawable.h"

//glm
#include <glm/glm.hpp>

namespace ns {
	class Object3d
	{
	public:
		Object3d(const Drawable& model,
			glm::vec3 position = glm::vec3(0),
			glm::vec3 scale = glm::vec3(1),
			glm::vec3 rotationAxe = {0, 1, 0},
			float angleInRadians = .0f);

		void update();
		void draw(const Shader& shader) const;

		void setPosition(const glm::vec3& newPosition);
		void setScale(const glm::vec3& newScale);
		void setRotation(const glm::vec3& axe, float angleInRadians);


		glm::vec3 position() const;
		glm::vec3 scale() const;
		glm::vec3 rotationAxe() const;
		float rotationAngle() const;

		glm::mat4 modelMatrix() const;
		glm::mat4 translationMatrix() const;
		glm::mat4 scaleMatrix() const;
		glm::mat4 rotationMatrix() const;

	protected:
		const Drawable& model_;

		glm::vec3 position_;
		glm::vec3 scale_;
		glm::vec3 rotationAxe_;
		float rotationAngle_;

		glm::mat4 translationMatrix_;
		glm::mat4 scaleMatrix_;
		glm::mat4 rotationMatrix_;
		glm::mat4 modelMatrix_;
	};
}
