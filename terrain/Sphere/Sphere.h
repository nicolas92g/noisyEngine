#pragma once
#include <configNoisy.hpp>

#define NUMBER_OF_FACES_IN_A_CUBE 6U
#define NULL_FACE_INDEX 7U

/**
 * @brief (TO DO) contain all the classes that can be used to generate and render a spheric world (a planet)
 */
namespace ns::Sphere{
	constexpr glm::vec3 SPHERE_LOCAL_CENTER = glm::vec3(0);

	struct BasicSphere {
		glm::vec3 center;
		float radius;
	};
}
