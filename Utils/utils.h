#pragma once
#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

namespace ns {
	std::string to_string(const glm::vec2& vec);
	std::string to_string(const glm::vec3& vec);
	std::string to_string(const glm::vec4& vec);
	std::string to_string(const glm::mat4& mat);
	glm::vec3 to_vec3(const aiVector3D& vec);
	glm::vec3 to_vec3(const aiColor3D& vec);
}

