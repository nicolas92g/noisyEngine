#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include "Window.h"

//enable shaders recompilation in the runtime with a key
#ifndef NDEBUG
#define RUNTIME_SHADER_RECOMPILATION
#define RECOMPILATION_KEY GLFW_KEY_X
#endif // NDEBUG

/**
 * @brief compile and use a shader program in glsl
 */
namespace ns {
	class Shader
	{
	public:

		enum class Stage
		{
			Vertex,
			Fragment,
			Geometry,
			Compute
		};
		struct Define {
			std::string name;
			std::string value;
			Stage stage;
		};
		/**
		 * @brief read all the files in inputs and compile them as glsl shaders
		 * errors of compiling are logs in the shell
		 * \param vertexPath
		 * \param fragmentPath
		 * \param geometryPath
		 */
		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const std::vector<Define>& defines = {}, bool reLoadable = false);
		/**
		 * @brief compile a compute shader
		 * \param computeShaderFilePath
		 */
		Shader(const char* computeShaderFilePath, const std::vector<Define>& defines = {}, bool reLoadable = false);
		~Shader();
		
		/**
		 * bind this shader
		 * need this line before a draw call to render with this shader
		 */
		void use() const;

		template<typename T>
		/**
		 * change the value of a uniform var in the shader
		 * types supported are : int, unsigned int, bool, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4
		 * \param name
		 * \param value
		 */
		void set(const char* name, T value) const;

		static void update(const Window& window);
		
	protected:
		uint32_t id;

#		ifdef RUNTIME_SHADER_RECOMPILATION
		std::vector<const char*> filepaths;
		std::vector<Define> defines;
		static std::list<Shader*> shaders;
		bool reloadable;
#		endif
		
		void loadShaderFrom(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<Define>& defines);
		void loadComputeShaderFrom(const char* computePath, const std::vector<Define>& defines);
		void setDefines(std::string& shaderCode, const std::vector<ns::Shader::Define>& defines, ns::Shader::Stage stage);

		void compileShader(const char* vertexText, const char* fragmentText, const char* geometryText);
		static bool filepathToString(std::string& string, const char* filepath);
	};
};

template<typename T>
inline void ns::Shader::set(const char* name, T value) const
{
	static_assert(false, "type is not supported by ns::Shader class");
}
template <>
inline void ns::Shader::set(const char* name, int value) const
{
	use();
	glUniform1i(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, unsigned int value) const
{
	use();
	glUniform1ui(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, bool value) const
{
	use();
	glUniform1i(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, float value) const
{
	use();
	glUniform1f(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, double value) const
{
	use();
	glUniform1f(glGetUniformLocation(id, name), static_cast<float>(value));
}
template <>
inline void ns::Shader::set(const char* name, glm::vec2 value) const
{
	use();
	glUniform2fv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::vec3 value) const
{
	use();
	glUniform3fv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::vec4 value) const
{
	use();
	glUniform4fv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::mat4 value) const
{
	use();
	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, false, &value[0][0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::ivec2 value) const
{
	use();
	glUniform2iv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::ivec3 value) const
{
	use();
	glUniform3iv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::ivec4 value) const
{
	use();
	glUniform4iv(glGetUniformLocation(id, name), 1, &value[0]);
}