#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <unordered_map>

#include <Utils/yamlConverter.h>

namespace ns {
	class Object3d
	{
	public:
		Object3d(const glm::vec3& position);

		virtual glm::vec3 position() const;
		virtual glm::vec3 WorldPosition() const;
		virtual const std::string& name() const;
		virtual void setPosition(const glm::vec3& position);
		virtual void setName(const std::string& name);

		virtual void setParent(Object3d* parent);
		virtual void removeParent();

		virtual bool hasParent() const;
		virtual Object3d* parent() const;

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

	class DirectionalObject3d : public Object3d
	{
	public:
		DirectionalObject3d(const glm::vec3& position, const glm::vec3& direction);

		void setDirection(const glm::vec3& direction);
		glm::vec3 direction();

	protected:
		glm::vec3 direction_;

		friend class Debug;
	};

	class GeometricObject3d : public Object3d
	{
	public:
		GeometricObject3d(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& axis, float angle);

		void update();

		void setScale(const glm::vec3& newScale);
		void setRotation(const glm::vec3& axe, float angleInRadians);


		glm::vec3 scale() const;
		glm::vec3 rotationAxis() const;
		float rotationAngle() const;

		const glm::mat4& modelMatrix() const;
		const glm::mat4& translationMatrix() const;
		const glm::mat4& scaleMatrix() const;
		const glm::mat4& rotationMatrix() const;

	protected:
		glm::vec3 scale_;
		glm::vec3 axis_;
		float angle_;

		glm::mat4 translationMatrix_;
		glm::mat4 scaleMatrix_;
		glm::mat4 rotationMatrix_;
		glm::mat4 modelMatrix_;

		friend class Debug;
	};
}
