#include "utils.h"

std::string ns::to_string(const glm::vec2& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + " )";
}

std::string ns::to_string(const glm::vec3& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + " )";
}

std::string ns::to_string(const glm::vec4& vec)
{
	return "( " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ", " + std::to_string(vec.w) + " )";
}

std::string ns::to_string(const glm::mat4& mat)
{
	return ns::to_string(mat[0]) + ns::to_string(mat[1]) + ns::to_string(mat[2]) + ns::to_string(mat[3]);
}

glm::vec3 ns::to_vec3(const aiVector3D& vec)
{
	return { vec.x, vec.y, vec.z };
}

glm::vec3 ns::to_vec3(const aiColor3D& vec)
{
	return { vec.r, vec.g, vec.b };
}

glm::vec3 ns::to_vec3(const ofbx::Color& vec)
{
	return {vec.r, vec.g, vec.b};
}

glm::vec3 ns::to_vec3(const ofbx::Vec3& vec)
{
	return { vec.x, vec.y, vec.z };
}

glm::vec4 ns::to_vec4(const ofbx::Vec4& vec)
{
	return { vec.x, vec.y, vec.z, vec.w };
}
