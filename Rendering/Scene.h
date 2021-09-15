#pragma once

//ns
#include "DrawableObject3d.h"
#include "Light.h"

//stl
#include <vector>

namespace ns {
	/**
	 * @brief a container which can store some entities, some static objects, some lights(point lights and spot lights) and a directional light
	 * all objects are stored as pointer, this allow to draw the scene and to update it.
	 */
	class Scene
	{
	public:
		/**
		 * @brief initialize the scene with some objects
		 * \param dirLight
		 * \param entities
		 * \param statics
		 * \param lights
		 */
		Scene(DirectionalLight& dirLight,
			const std::vector<DrawableObject3d*>& entities = {},
			const std::vector<DrawableObject3d*>& statics = {},
			const std::vector<attenuatedLightBase_*>& lights = std::vector<attenuatedLightBase_*>(0)
		);
		/**
		 * @brief lvalue copy constructor
		 * \param other
		 */
		Scene(const Scene& other);
		/**
		 * @brief rvalue copy constructor
		 * \param other
		 * \return 
		 */
		Scene(Scene&& other) noexcept;
		/**
		 * @brief return the number of entities that the scene contain
		 * \return 
		 */
		uint32_t numEntities();
		/**
		 * @brief return the number of static objects that the scene contain
		 * \return
		 */
		uint32_t numStatics();
		/**
		 * @brief return the number of lights (the directional light is not counted)
		 * \return 
		 */
		uint32_t numLights();
		/**
		 * @brief send all the lights that the scene contain to one shader
		 * \param shader
		 */
		void sendLights(const ns::Shader& shader) const;
		/**
		 * @brief call the draw calls of the entities and the statics
		 * \param shader
		 */
		void draw(const ns::Shader& shader) const;
		/**
		 * @brief update only the entities
		 */
		void update();
		/**
		 * @brief update only the statics
		 */
		void updateStatics();
		/**
		 * @brief add an entity to the scene by checking its pointer is not already in the scene
		 * \param object
		 */
		void addEntity(DrawableObject3d& object);
		/**
		 * @brief remove the entity from the scene if it exist
		 * \param object
		 */
		void removeEntity(DrawableObject3d& object);
		/**
		 * @brief remove all entities
		 */
		void clearEntities();
		/**
		 * @brief add a static object to the scene by checking its pointer is not already in the scene
		 * \param object
		 */
		void addStatic(DrawableObject3d& object);
		/**
		 * @brief remove the static object from the scene if it exist
		 * \param object
		 */
		void removeStatic(DrawableObject3d& object);
		/**
		 * @brief remove all static objects
		 */
		void clearStatics();
		/**
		 * @brief add a light to the scene by checking its pointer is not already in the scene
		 * \param light
		 */
		void addLight(attenuatedLightBase_& light);
		/**
		 * @brief remove a light from the scene if it exist
		 * \param light
		 */
		void removeLight(attenuatedLightBase_& light);
		/**
		 * @brief add an array of lights withount checking if the pointer are already in the scene !
		 * unsafe method !
		 * \param lights
		 */
		void addLights(const std::vector<std::shared_ptr<attenuatedLightBase_>>& lights);
		/**
		 * @brief set the only directional light of the scene
		 * \param dirLight
		 */
		void setDirectionalLight(DirectionalLight& dirLight);
		/**
		 * @brief return the only directional light of the scene
		 * \return 
		 */
		DirectionalLight& directionalLight();
		/**
		 * @brief add the objects from another scene to this scene
		 * \param other
		 */
		void operator+=(const Scene& other);
		/**
		 * @brief copy a scene into this by lvalue
		 * \param other
		 */
		void operator=(const Scene& other);
		/**
		 * @brief copy a scene into this by rvalue
		 * \param other
		 */
		void operator=(Scene&& other) noexcept;
		/**
		 * @brief create a scene by adding two scenes together
		 */
		friend Scene operator+(Scene scene, const Scene& other);
		

	protected:
		std::vector<DrawableObject3d*> entities_;		//movable Objects
		std::vector<DrawableObject3d*> statics_;	    //motionless Objects
		std::vector<attenuatedLightBase_*> lights_;		//lights Objects using polymorphism
		DirectionalLight* dirLight_;					//single directional light

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
