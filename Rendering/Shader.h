#pragma once

//gl
#include <glad/glad.h>
#include <glm/glm.hpp>

//stl
#include <unordered_map>
#include <string>

//ns
#include "Window.h"

//enable shaders recompilation in the runtime with a key
#ifndef NDEBUG
#define RUNTIME_SHADER_RECOMPILATION
#define RECOMPILATION_KEY GLFW_KEY_X
#endif // NDEBUG


namespace ns {
	/**
	* @brief compile and use a shader program in glsl
	*/
	class Shader
	{
	public:
		/**
		 * @brief allow to describe the name of a shader stage
		 */
		enum class Stage
		{
			Vertex,
			Fragment,
			Geometry,
			Compute
		};
		/**
		 * @brief this struct allow to create a #define directly in the copy of the source code 
		 * if the define is already existing, the value will be modified
		 * the stage is the file where the #define has to be written (fragment shader, vertex shader or geometry shader) (compute shader)
		 */
		struct Define {
			std::string name;
			std::string value;
			Stage stage;
		};
		/**
		 * @brief read all the files and compile them as glsl shaders
		 * if you don't set the geometry stage, the default geometry shader will be used
		 * you can also add an array of defines that are written in the source code as #define name value
		 * reloadable is useful for debugging to reload the shader while the program is running
		 * errors of compiling are logs in the shell
		 * \param vertexPath
		 * \param fragmentPath
		 * \param geometryPath
		 * \param defines
		 * \param reloadable
		 */
		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const std::vector<Define>& defines = {}, bool reLoadable = false);
		/**
		 * @brief compile a compute shader
		 * defines has to have a stage value equal to Compute
		 * you can also reload compute shaders for debugging usage
		 * \param computeShaderFilePath
		 */
		Shader(const char* computeShaderFilePath, const std::vector<Define>& defines = {}, bool reLoadable = false);
		/**
		 * @brief delete the shader
		 */
		~Shader();
		/**
		 * bind this shader but check if this is not already used before
		 * need this line before a draw call to render with this shader
		 */
		void use() const;
		/**
		 * @brief bind this shader without checking if it is already used
		 */
		void forceUse() const;
		template<typename T>
		/**  
		 * change the value of a uniform var in the shader
		 * types supported are : int, unsigned int, bool, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, glm::ivec2, glm::ivec3, glm::ivec4
		 * this->use() is call in this method
		 * \param name
		 * \param value
		 */
		void set(const char* name, T const& value) const;
		/**
		 * @brief debug function that check a key to reload the shaders that are reloadable
		 * \param window
		 */
		static void update(const Window& window);
		
	protected:
		GLuint id;

		static GLuint currentlyBindedShader;
		mutable std::unordered_map<std::string, int> uniformsNames_;

#		ifdef RUNTIME_SHADER_RECOMPILATION
		std::vector<const char*> filepaths;
		std::vector<Define> defines;
		static std::list<Shader*> shaders;
		bool reloadable;
#		endif
		
		void loadShaderFrom(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<Define>& defines);
		void loadComputeShaderFrom(const char* computePath, const std::vector<Define>& defines);
		void setDefines(std::string& shaderCode, const std::vector<ns::Shader::Define>& defines, ns::Shader::Stage stage);

		static void removeCommentsFromGlslSource(std::string& source);
		static void treatUniformArrays(std::vector<std::string>& names, const std::string& source);
		void readUniformsNamesFromSource(const std::string& source);
		int getUniformLocation(const std::string& name) const;

		void compileShader(const char* vertexText, const char* fragmentText, const char* geometryText);
		static bool filepathToString(std::string& string, const char* filepath);
	};
};