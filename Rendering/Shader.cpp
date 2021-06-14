#include "Shader.h"

#include <fstream>
#include <fstream>
#include <map>
#include <KHR/khrplatform.h>
#include <iostream>


#ifdef RUNTIME_SHADER_RECOMPILATION
std::list<ns::Shader*> ns::Shader::shaders;
#endif

//this constructor require filepath of shaders
ns::Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<Define> &defines, bool reLoadable)
{
	loadShaderFrom(vertexPath, fragmentPath, geometryPath, defines);

#	ifdef RUNTIME_SHADER_RECOMPILATION
	if (reLoadable) {
		shaders.push_back(this);
		filepaths = { vertexPath , fragmentPath , geometryPath };
		this->reloadable = reLoadable;
	}
#	endif // RUNTIME_SHADER_RECOMPILATION
}

ns::Shader::Shader(const char* computeShaderFilePath, const std::vector<Define>& defines, bool reLoadable)
{
	loadComputeShaderFrom(computeShaderFilePath, defines);

#	ifdef RUNTIME_SHADER_RECOMPILATION
	if (reLoadable) {
		shaders.push_back(this);
		filepaths = { computeShaderFilePath };
	}
#	endif // RUNTIME_SHADER_RECOMPILATION
}

ns::Shader::~Shader()
{
	glDeleteProgram(id);

#	ifdef RUNTIME_SHADER_RECOMPILATION
	if (!reloadable) return;

	auto it = std::find(shaders.begin(), shaders.end(), this);
	if (it == shaders.end()) return;

	shaders.erase(it);
#	endif
}

bool ns::Shader::filepathToString(std::string& string, const char* filepath)
{
	std::ifstream file(filepath);
	std::string line;
	if (!file.is_open()) return false;
	
	string = "";
	while (getline(file, line)) {
		string.append(line += '\n');
	}
	file.close();
	return true;
}

void ns::Shader::update(const Window& window)
{
#	ifdef RUNTIME_SHADER_RECOMPILATION
	static bool recompileKeyState = false;

	
	if (window.key(RECOMPILATION_KEY) and !recompileKeyState) {
		std::cout << "reloading shaders";
		for (Shader* shader : shaders) {
			if (shader->filepaths.size() > 1)
			{
				shader->loadShaderFrom(shader->filepaths[0], shader->filepaths[1], shader->filepaths[2], {});
			}
			else 
			{
				shader->loadComputeShaderFrom(shader->filepaths[0], {});
			}
			std::cout << ".";
		}
		std::cout << std::endl;
	}

	recompileKeyState = window.key(RECOMPILATION_KEY);
#	endif // RUNTIME_SHADER_RECOMPILATION
}

//this function require filepath of shaders
void ns::Shader::loadShaderFrom(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<Define>& defines)
{
	std::string vertex;
	if (!filepathToString(vertex, vertexPath)) {
		std::cout << "error : can't find vertex shader's file at path :\n" << vertexPath << "\n";
	}


	std::string fragment;
	if (!filepathToString(fragment, fragmentPath)) {
		std::cout << "error : can't find fragment shader's file at path :\n" << fragmentPath << "\n";
	}

	std::string geometry;
	if (geometryPath != nullptr and strlen(geometryPath) > 1 and !filepathToString(geometry, geometryPath)) {
		std::cout << "error : can't find geometry shader's file at path :\n" << geometryPath << "\n";
	}

	setDefines(vertex, defines, ns::Shader::Stage::Vertex);
	setDefines(fragment, defines, ns::Shader::Stage::Fragment);
	setDefines(geometry, defines, ns::Shader::Stage::Geometry);

	compileShader(vertex.c_str(), fragment.c_str(), geometry.c_str());
}

void ns::Shader::loadComputeShaderFrom(const char* computePath, const std::vector<ns::Shader::Define>& defines)
{
	std::string computeText;
	if (!filepathToString(computeText, computePath)) {
		std::cout << "error : can't find compute shader's file at path :\n" << computePath << "\n";
	}

	const char* text = computeText.c_str();

	uint32_t computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &text, NULL);
	glCompileShader(computeShader);

	int* vertexCompilationSuccess = new int();
	char errorLog[512];
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, vertexCompilationSuccess);
	if (!*vertexCompilationSuccess) {
		glGetShaderInfoLog(computeShader, 512, NULL, errorLog);
		std::cerr << "ERROR in the compute shader \n" << errorLog << '\n';
	}

	id = glCreateProgram();
	glAttachShader(id, computeShader);
	glLinkProgram(id);

	int linkSuccess;
	glGetProgramiv(id, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(id, 512, NULL, errorLog);
		std::cerr << "ERROR while linking compute shader \n" << errorLog << '\n';
	}
}

void ns::Shader::setDefines(std::string& shaderCode, const std::vector<ns::Shader::Define>& defines, ns::Shader::Stage stage)
{
	for (const auto& define : defines)
	{
		//check that the define is in the wanted stage
		if (define.stage != stage) continue;

		//search the #define line
		std::string line = "#define " + define.name;
		size_t existingDefine = shaderCode.find(line);

		//if define already exist 
		if (existingDefine != std::string::npos) {
			const size_t size = shaderCode.find('\n', existingDefine + line.size()) - (existingDefine + line.size());
			shaderCode.erase(existingDefine + line.size(), size);
			shaderCode.insert(existingDefine + line.size(), " " + define.value);
		}
		//if it doesn't exist put it just after the #version XXX line
		else {
			size_t pos = shaderCode.find('\n', shaderCode.find("#version"));
			shaderCode.insert(pos + 1, line + " " + define.value + '\n');
		}
	}
}

void ns::Shader::use() const
{
	glUseProgram(id);
}

void ns::Shader::compileShader(const char* vertex, const char* fragment, const char* geometry)
{
	//vertex Shader
	uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex, NULL);
	glCompileShader(vertexShader);

	//check if vertex compilation success
	int* vertexCompilationSuccess = new int();
	char errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, vertexCompilationSuccess);
	if (!*vertexCompilationSuccess) {
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR in the vertex shader \n" << errorLog << '\n';
	}

	uint32_t geometryShader;
	const bool useGeometryShader = geometry != nullptr and strlen(geometry) > 1;
	if (useGeometryShader) {
		//geometry shader 
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometry, NULL);
		glCompileShader(geometryShader);

		//check if geometry compilation success
		int* geometryCompilationSuccess = new int();
		char errorLog[512];
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, geometryCompilationSuccess);
		if (!*geometryCompilationSuccess) {
			glGetShaderInfoLog(geometryShader, 512, NULL, errorLog);
			std::cerr << "ERROR in the geometry shader \n" << errorLog << '\n';
		}
	}

	//fragment shader
	uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment, NULL);
	glCompileShader(fragmentShader);

	int* fragmentCompilationSuccess = new int();
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, fragmentCompilationSuccess);
	if (!*fragmentCompilationSuccess) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR in the fragment shader \n" << errorLog << '\n';
	}
	//create a programm object
	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	if (useGeometryShader) glAttachShader(id, geometryShader);
	glLinkProgram(id);

	//verify if link was a success
	int linkSuccess;
	glGetProgramiv(id, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(id, 512, NULL, errorLog);
		std::cerr << "ERROR while linking shaders \n" << errorLog << '\n';
	}
	use();
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}


