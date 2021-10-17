#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <unordered_map>

#include <Utils/yamlConverter.h>

#include <configNoisy.hpp>

#define DEFAULT_PTYPE float
#define DEFAULT_DTYPE float

namespace ns {
	
	//P is a high precision type, P stand for position
	template<typename P = DEFAULT_PTYPE, typename D = DEFAULT_DTYPE>
	/**
	 * @brief allow to describe a basic Object in 3 dimensions with a position and a parent or not
	 */
	class Object3d
	{
	public:
		//type used to store high precision values like position 
		using vec3p = glm::vec<3, P>;
		/**
		 * @brief create it
		 * \param position
		 */
		Object3d(const vec3p& position);
		/**
		 * @brief get the position in the world if it don't have any parent else return the position relative to the parent
		 * \return 
		 */
		virtual const vec3p& position() const;
		/**
		 * @brief get the position in the world even if it has a parent
		 * \return 
		 */
		virtual vec3p WorldPosition() const;
		/**
		 * @brief return the name of the object
		 * \return 
		 */
		virtual const std::string& name() const;
		/**
		 * @brief change the position of the object without any verification
		 * \param position
		 */
		virtual void setPosition(const vec3p& position);
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
		vec3p position_;
		std::optional<Object3d<P, D>*> parent_;
		std::string name_;

		virtual bool isGeometricObject3d();
		virtual bool isDirectionalObject3d();

		friend class Debug;
		static unsigned entityCount;
		static std::unordered_map<std::string, Object3d<P, D>*> objects;
	};

	//P is a high precision type but D can be lower due to lower needs (D stand for direction)
	template<typename P = DEFAULT_PTYPE, typename D = DEFAULT_DTYPE>
	/**
	 * @brief is an Object3d but with a direction 
	 */
	class DirectionalObject3d : public Object3d<P, D>
	{
	public:
		using vec3d = glm::vec<3, D>;
		using vec3p = glm::vec<3, P>;
		/**
		 * @brief create the object
		 * \param position
		 * \param direction
		 */
		DirectionalObject3d(const vec3p& position, const vec3d& direction);
		/**
		 * @brief normalize the new direction
		 * \param direction
		 */
		void setDirection(const vec3d& direction);
		/**
		 * @brief return the direction
		 * \return 
		 */
		const vec3d& direction() const;

	protected:
		vec3d direction_;

		//give the private access to the Debug class 
		friend class Debug;
	};

	//P is a high precision type but D can be lower due to lower needs (D stand for direction)
	template<typename P = DEFAULT_PTYPE, typename D = DEFAULT_DTYPE>
	/**
	 * @brief this is an Object3d but with a scale and a rotation and is able to generate some matrices with those values
	 */
	class GeometricObject3d : public Object3d<P, D>
	{
	public:
		using vec3p = glm::vec<3, P>;
		using mat4p = glm::mat<4, 4, P>;
		using vec3d = glm::vec<3, D>;
		/**
		 * @brief create the object
		 * \param position
		 * \param scale
		 * \param axis
		 * \param angle
		 */
		GeometricObject3d(const vec3p& position, const vec3p& scale, const vec3d& axis, D angle);
		/**
		 * @brief regenerate the matrices
		 */
		void update();
		/**
		 * @brief change the scale of the object without any verification
		 * \param newScale
		 */
		void setScale(const vec3p& newScale);
		/**
		 * @brief change the rotation values, the axis is normalized
		 * \param axis
		 * \param angleInRadians
		 */
		void setRotation(const vec3d& axis, D angleInRadians);
		/**
		 * @brief return the scale
		 * \return 
		 */
		const vec3p& scale() const;
		/**
		 * @brief return the rotation axis
		 * \return 
		 */
		const vec3d& rotationAxis() const;
		/**
		 * @brief return the rotation angle
		 * \return 
		 */
		D rotationAngle() const;
		/**
		 * @brief just return the previous reult matrix that has been calculated by the last update() call
		 * \return 
		 */
		const mat4p& modelMatrix() const;

#		if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
		/**
		 * @brief just return the previous translation matrix that has been calculated by the last update() call
		 * \return
		 */
		const mat4p& translationMatrix() const;
		/**
		 * @brief just return the previous scaling matrix that has been calculated by the last update() call
		 * \return
		 */
		const mat4p& scaleMatrix() const;
		/**
		 * @brief just return the previous rotation matrix that has been calculated by the last update() call
		 * \return
		 */
		const mat4p& rotationMatrix() const;
#		endif

	protected:
		vec3p scale_;
		vec3d axis_;
		D angle_;

		mat4p modelMatrix_;
#		if NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES
		mat4p scaleMatrix_;
		mat4p rotationMatrix_;
		mat4p translationMatrix_;
#		endif
		//give the private access to the debug class
		friend class Debug;
	};
}
