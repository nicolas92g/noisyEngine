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
		Scene(const Scene& other);
		Scene(Scene&& other) noexcept;
		

		void sendLights(const ns::Shader& shader) const;
		void draw(const ns::Shader& shader) const;
		void update();

		void updateStationaries();

		void addEntity(DrawableObject3d& object);
		void removeEntity(DrawableObject3d& object);
		void clearEntities();

		void addStationary(DrawableObject3d& object);
		void removeStationary(DrawableObject3d& object);
		void clearStationaries();

		void addLight(LightBase_& light);
		void removeLight(LightBase_& light);
		void addLights(const std::vector<std::shared_ptr<LightBase_>>& lights);

		void operator+=(const Scene& other);
		void operator=(const Scene& other);
		void operator=(Scene&& other) noexcept;
		

	protected:
		std::vector<DrawableObject3d*> entities_;		//movable Objects
		std::vector<DrawableObject3d*> stationaries_;	//motionless Objects
		std::vector<LightBase_*> lights_;				//lights Objects using polymorphism

		friend class Debug;

		template<typename T>
		static void addElement(T* element, std::vector<T*>& arr);

		template<typename T>
		static void removeElement(T* element, std::vector<T*>& arr);

	};

	template<typename T>
	inline void Scene::addElement(T* element, std::vector<T*>& arr)
	{
		for (const T* obj : arr) if (obj == element) return;

		arr.push_back(element);
	}
	template<typename T>
	inline void Scene::removeElement(T* element, std::vector<T*>& arr)
	{
		for (auto it = arr.begin(); it != arr.end(); ++it)
			if (*it == element) arr.erase(it);
	}
}
