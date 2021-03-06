#pragma once
#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
//#include <ofbx.h>
#include "DebugLayer.h"
#include <glad/glad.h>
#include "yamlConverter.h"


namespace ns {
	constexpr double PI = glm::pi<double>();
	constexpr double PI_f = glm::pi<float>();

	std::string to_string(const glm::vec2& vec);
	std::string to_string(const glm::vec3& vec);
	std::string to_string(const glm::vec4& vec);
	std::string to_string(const glm::ivec2& vec);
	std::string to_string(const glm::ivec3& vec);
	std::string to_string(const glm::ivec4& vec);
	std::string to_string(const glm::mat4& mat);

	glm::vec3 to_vec3(const aiVector3D& vec);
	glm::vec3 to_vec3(const aiColor3D& vec);
	//glm::vec3 to_vec3(const ofbx::Color& vec);
	//glm::vec3 to_vec3(const ofbx::Vec3& vec);
	//glm::vec4 to_vec4(const ofbx::Vec4& vec);
	void to_mat4(glm::mat4& output, const aiMatrix4x4* mat);
	void clearConfigFile();
	glm::vec4 getClearColor();
	void SetupImGuiStyle(bool bStyleDark_, float alpha_);
	void saveConfiguration();

	bool isFileExist(const std::string& filepath);
	YAML::Node loadAndCheckFile(const std::string& filename);

	float noise(const glm::vec3& input);

	glm::vec3 genNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	extern YAML::Node conf;
}
