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
	template<typename P = DEFAULT_PTYPE, typename D = DEFAULT_DTYPE>
	/**
	 * @brief this describe an object that allow to draw an object by reference but add a 3D position, scale and rotation
	 */
	class DrawableObject3d : public GeometricObject3d<P, D>
	{
	public:
		using vec3p = glm::vec<3, P>;
		using mat4p = glm::mat<4, 4, P>;
		using vec3d = glm::vec<3, D>;
		/**
		 * @brief need a Drawable object (it is possible to create a custom Drawable object or to use for example the ns::Model class)
		 * \param model the object with a draw call
		 * \param position position where the object will be draw
		 * \param scale scale of the object 
		 * \param axis rotation axis of the object 
		 * \param angleInRadians angle of the rotation of the object
		 */
		DrawableObject3d(Drawable& mesh,
			const vec3p& position = vec3p(0),
			const vec3p& scale = vec3p(1),
			const vec3d& axis = vec3p(0, 1, 0),
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
		/**
		 * @brief edit the meshes of this object
		 * \param mesh
		 */
		void setMesh(Drawable& mesh);

	protected:
		Drawable* model_;
		std::vector<std::shared_ptr<ns::LightBase_>> lights_;

		friend class Debug;
	};
	/**
	 * @brief used to draw a 3D vector
	 */
	class Line : public DirectionalObject3d<>{
	public:
		Line(const glm::vec3& vector, const glm::vec3& origin = NS_BLACK) : DirectionalObject3d<>(origin, vector), length_(1)
		{
			glGenVertexArrays(1, &VAO_);
			glGenBuffers(1, &VBO_);

			update();
		}
		void update() 
		{
			const glm::vec3 vector = glm::normalize(direction_) * length_;
			float data[] = {
				position_.x, position_.y, position_.z,
				position_.x + vector.x, position_.y + vector.y, position_.z + vector.z
			};

			glBindVertexArray(VAO_);

			glBindBuffer(GL_ARRAY_BUFFER, VBO_);
			glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), data, GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

			glBindVertexArray(0);
		}
		void draw(ns::Shader& shader) const
		{
			shader.use();
			glBindVertexArray(VAO_);
			glDrawArrays(GL_LINES, 0, 2);
		}
		void setLength(float length) {
			length_ = std::max(.001f, length);
		}
		~Line() 
		{
			glDeleteBuffers(1, &VBO_);
			glDeleteVertexArrays(1, &VAO_);
		}
	protected:
		GLuint VAO_;
		GLuint VBO_;
		float length_;
	};
}
