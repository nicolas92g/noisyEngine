#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <unordered_map>

#include <Utils/yamlConverter.h>

#include <configNoisy.hpp>

namespace ns {
	/**
	 * @brief allow to describe a basic Object in 3 dimensions with a position and a parent or not
	 */
	class Object3d
	{
	public:
		/**
		 * @brief create it
		 * \param position
		 */
		Object3d(const glm::vec3& position);
		/**
		 * @brief get the position in the world if it don't have any parent else return the position relative to the parent
		 * \return 
		 */
		virtual glm::vec3 position() const;
		/**
		 * @brief get the position in the world even if it has a parent
		 * \return 
		 */
		virtual glm::vec3 WorldPosition() const;
		/**
		 * @brief return the name of the object
		 * \return 
		 */
		virtual const std::string& name() const;
		/**
		 * @brief change the position of the object without any verification
		 * \param position
		 */
		virtual void setPosition(const glm::vec3& position);
		/**
		 * @brief change the name of the object
		 * \param name
		 */
		virtual void setName(const std::string& name);
		/**
		 * @brief change the parent of the object and check that the parent is not himself or null
		 * \param parent
		 */
		virtual void setParent(Object3d* parent);
		/**
		 * @brief remove the parent of this object
		 */
		virtual void removeParent();
		/**
		 * @brief say if this has a parent
		 * \return 
		 */
		virtual bool hasParent() const;
		/**
		 * @brief return a pointer to the parent
		 * \return 
		 */
		virtual Object3d* parent() const;
		/**
		 * @brief do not use this, (not finished)
		 * \param filepath
		 * \return 
		 */
		virtual YAML::Node inputFromYAML(const std::string& filepath);

	protected:
		glm::vec3 position_;
		std::optional<Object3d*> parent_;
		std::string name_;

		virtual bool isGeometricObject3d();
		virtual bool isDirectionalObject3d();

		friend class Debug;
		static unsigned entityCount;
		static std::unordered_map<std::string, Object3d*> objects;
	};
	/**
	 * @brief is an Object3d but with a direction 
	 */
	class DirectionalObject3d : public Object3d
	{
	public:
		/**
		 * @brief create the object
		 * \param position
		 * \param direction
		 */
		DirectionalObject3d(const glm::vec3& position, const glm::vec3& direction);
		/**
		 * @brief normalize the new direction
		 * \param direction
		 */
		void setDirection(const glm::vec3& direction);
		/**
		 * @brief return the direction
		 * \return 
		 */
		glm::vec3 direction() const;

	protected:
		glm::vec3 direction_;

		//give the private access to the Debug class 
		friend class Debug;
	};
	/**
	 * @brief this is an Object3d but with a scale and a rotation and is able to generate some matrices with those values
	 */
	class GeometricObject3d : public Object3d
	{
	public:
		/**
		 * @brief create the object
		 * \param position
		 * \param scale
		 * \param axis
		 * \param angle
		 */
		GeometricObject3d(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& axis, float angle);
		/**
		 * @brief regenerate the matrices
		 */
		void update();
		/**
		 * @brief change the scale of the object without any verification
		 * \param newScale
		 */
		void setScale(const glm::vec3& newScale);
		/**
		 * @brief change the rotation values, the axis is normalized
		 * \param axis
		 * \param angleInRadians
		 */
		void setRotation(const glm::vec3& axis, float angleInRadians);
		/**
		 * @brief return the scale
		 * \return 
		 */
		glm::vec3 scale() const;
		/**
		 * @brief return the rotation axis
		 * \return 
		 */
		glm::vec3 rotationAxis() const;
		/**
		 * @brief return the rotation angle
		 * \return 
		 */
		float rotationAngle() const;
		/**
		 * @brief just return the previous reult matrix that has been calculated by the last update() call
		 * \return 
		 */
		const glm::mat4& modelMatrix() const;

#		if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
		/**
		 * @brief just return the previous translation matrix that has been calculated by the last update() call
		 * \return
		 */
		const glm::mat4& translationMatrix() const;
		/**
		 * @brief just return the previous scaling matrix that has been calculated by the last update() call
		 * \return
		 */
		const glm::mat4& scaleMatrix() const;
		/**
		 * @brief just return the previous rotation matrix that has been calculated by the last update() call
		 * \return
		 */
		const glm::mat4& rotationMatrix() const;
#		endif

	protected:
		glm::vec3 scale_;
		glm::vec3 axis_;
		float angle_;

		glm::mat4 modelMatrix_;
#		if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
		glm::mat4 scaleMatrix_;
		glm::mat4 rotationMatrix_;
		glm::mat4 translationMatrix_;
#		endif
		//give the private access to the debug class
		friend class Debug;
	};
}
