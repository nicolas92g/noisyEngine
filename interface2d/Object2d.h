#pragma once

//glm
#include <glm/glm.hpp>

//glad
#include <glad/glad.h>

//ns
#include <Rendering/Texture.h>
#include <Rendering/Drawable.h>

namespace ns 
{
	/**
	 * @brief describe a basic two dimensional object with a 2d position, a 2d scale and a 3d rotation
	 */
	class Object2d
	{
	public:
		/**
		 * @brief create an Object2d 
		 * create the model matrix with those values
		 * \param position
		 * \param scale
		 * \param rotationAngle
		 * \param rotationAxis
		 */
		Object2d(const glm::vec2& position = glm::vec3(0), const glm::vec2& scale = glm::vec2(100.f), float rotationAngle = 0.f, const glm::vec3& rotationAxe = glm::vec3(0, 0, 1));
		/**
		 * @brief return pos
		 * \return
		 */
		const glm::vec2& position() const;
		/**
		 * @brief return scale
		 * \return
		 */
		const glm::vec2& scale() const;
		/**
		 * @brief return rotation angle
		 * \return
		 */
		float rotationAngle() const;
		/**
		 * @brief return normalized rotation axis
		 * \return
		 */
		const glm::vec3& rotationAxis() const;
		/**
		 * @brief set the Object pos without any verification
		 * \param position
		 */
		void setPosition(const glm::vec2& position);
		/**
		 * @brief change the Object scale without any verification
		 * \param scale
		 */
		void setScale(const glm::vec2& scale);
		/**
		 * @brief change the rotation angle without anyu verification and change the rotation axis an normalize it
		 * \param angle
		 * \param axis
		 */
		void setRotation(float angle, const glm::vec3& axis = glm::vec3(0, 0, 1));
		/**
		 * @brief compute the model matrix of the 2D Object
		 */
		void update();
		/**
		 * @brief return the model transformation matrix that was previously computed by the update() method
		 * \return 
		 */
		const glm::mat4& modelMatrix();

	protected:
		//data
		glm::vec2 position_;
		glm::vec2 scale_;
		glm::vec3 rotationAxis_;
		float rotationAngle_;

		//result
		glm::mat4 model_;
	};

	
	class DrawableObject2d : public Object2d
	{
	public:
		/**
		 * @brief create a drawable geometric 2D Object 
		 * \param obj
		 * \param pos
		 * \param scale
		 * \param angle
		 * \param axis
		 */
		DrawableObject2d(Drawable& obj, const glm::vec2& pos = glm::vec2(100), const glm::vec2& scale = glm::vec2(100), float angle = .0f, const glm::vec3& axis = glm::vec3(0, 0, 1));

		void draw(const ns::Shader& shader) const;

	protected:
		Drawable& obj_;
	};


}
