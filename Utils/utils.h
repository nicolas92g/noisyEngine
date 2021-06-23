#pragma once
#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <ofbx.h>

namespace ns {
	std::string to_string(const glm::vec2& vec);
	std::string to_string(const glm::vec3& vec);
	std::string to_string(const glm::vec4& vec);
	std::string to_string(const glm::mat4& mat);

	glm::vec3 to_vec3(const aiVector3D& vec);
	glm::vec3 to_vec3(const aiColor3D& vec);
	glm::vec3 to_vec3(const ofbx::Color& vec);
	glm::vec3 to_vec3(const ofbx::Vec3& vec);

	glm::vec4 to_vec4(const ofbx::Vec4& vec);

}

