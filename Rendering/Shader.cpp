#include "Shader.h"

#include <fstream>
#include <fstream>
#include <map>
#include <KHR/khrplatform.h>
#include <iostream>

#include <Utils/DebugLayer.h>

GLuint ns::Shader::currentlyBindedShader = 0;

#ifdef RUNTIME_SHADER_RECOMPILATION
std::list<ns::Shader*> ns::Shader::shaders;
#endif

//this constructor require filepath of shaders
ns::Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<Define> &defines, bool reLoadable)
{
	loadShaderFrom(vertexPath, fragmentPath, geometryPath, defines);

#	ifdef RUNTIME_SHADER_RECOMPILATION
	this->reloadable = reLoadable;
	if (reLoadable) {
		shaders.push_back(this);
		filepaths = { vertexPath , fragmentPath , geometryPath };
		this->defines = defines;
	}
#	endif // RUNTIME_SHADER_RECOMPILATION
}

ns::Shader::Shader(const char* computeShaderFilePath, const std::vector<Define>& defines, bool reLoadable)
{
	loadComputeShaderFrom(computeShaderFilePath, defines);

#	ifdef RUNTIME_SHADER_RECOMPILATION
	this->reloadable = reLoadable;
	if (reLoadable) {
		shaders.push_back(this);
		filepaths = { computeShaderFilePath };
		this->defines = defines;
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
		dout << "reloading shaders";
		for (Shader* shader : shaders) {
			if (shader->filepaths.size() > 1)
			{
				shader->loadShaderFrom(shader->filepaths[0], shader->filepaths[1], shader->filepaths[2], shader->defines);
			}
			else 
			{
				shader->loadComputeShaderFrom(shader->filepaths[0], shader->defines);
			}
			dout << ".";
		}
		dout << std::endl;
	}

	recompileKeyState = window.key(RECOMPILATION_KEY);
#	endif // RUNTIME_SHADER_RECOMPILATION
}

//this function require filepath of shaders
void ns::Shader::loadShaderFrom(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<Define>& defines)
{
	std::string vertex;
	if (!filepathToString(vertex, vertexPath)) {
		dout << "error : can't find vertex shader's file at path :\n" << vertexPath << "\n";
	}


	std::string fragment;
	if (!filepathToString(fragment, fragmentPath)) {
		dout << "error : can't find fragment shader's file at path :\n" << fragmentPath << "\n";
	}

	std::string geometry;
	if (geometryPath != nullptr and strlen(geometryPath) > 1 and !filepathToString(geometry, geometryPath)) {
		dout << "error : can't find geometry shader's file at path :\n" << geometryPath << "\n";
	}

	setDefines(vertex, defines, ns::Shader::Stage::Vertex);
	setDefines(fragment, defines, ns::Shader::Stage::Fragment);
	setDefines(geometry, defines, ns::Shader::Stage::Geometry);

	//readUniformsNamesFromSource(vertex);
	//readUniformsNamesFromSource(fragment);
	//readUniformsNamesFromSource(geometry);

	compileShader(vertex.c_str(), fragment.c_str(), geometry.c_str());
}

void ns::Shader::loadComputeShaderFrom(const char* computePath, const std::vector<ns::Shader::Define>& defines)
{
	std::string computeText;
	if (!filepathToString(computeText, computePath)) {
		dout << "error : can't find compute shader's file at path :\n" << computePath << "\n";
	}

	setDefines(computeText, defines, ns::Shader::Stage::Compute);

	removeCommentsFromGlslSource(computeText);
	//readUniformsNamesFromSource(computeText);

	const char* text = computeText.c_str();

	uint32_t computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &text, NULL);
	glCompileShader(computeShader);

	int* vertexCompilationSuccess = new int();
	char errorLog[512];
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, vertexCompilationSuccess);
	if (!*vertexCompilationSuccess) {
		glGetShaderInfoLog(computeShader, 512, NULL, errorLog);
		dout << "ERROR in the compute shader \n file : " << computePath << newl << errorLog << '\n';
	}

	id = glCreateProgram();
	glAttachShader(id, computeShader);
	glLinkProgram(id);

	int linkSuccess;
	glGetProgramiv(id, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(id, 512, NULL, errorLog);
		dout << "ERROR while linking compute shader \n file : " << computePath << newl << errorLog << '\n';
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

void ns::Shader::removeCommentsFromGlslSource(std::string& source)
{
	//remove line comments 
	size_t lineCommentBeg = source.find("//");
	while (lineCommentBeg != source.npos) {
		source.erase(lineCommentBeg, source.find('\n', lineCommentBeg) - lineCommentBeg);
		lineCommentBeg = source.find("//", lineCommentBeg + 1);
	}
	//remove paragraph comments
	size_t paragraphCommentBeg = source.find("/*");
	while (paragraphCommentBeg != source.npos) {
		source.erase(paragraphCommentBeg, source.find("*/", paragraphCommentBeg) - paragraphCommentBeg + 2);
		paragraphCommentBeg = source.find("/*", paragraphCommentBeg + 1);
	}
}

void ns::Shader::treatUniformArrays(std::vector<std::string>& names, const std::string& source)
{
	for (const std::string& name : names) {
		size_t bracketBeg = name.find('[');
		if (bracketBeg == name.npos) continue;

		size_t bracketEnd = name.find(']');
		if (bracketBeg == name.npos) {
			dout << "Shader uniform error : in uniform :" << name << " ! can't find ']' \n";
		}
		std::string size = name.substr(bracketBeg + 1, bracketEnd - bracketBeg - 1);

		int value = 0;

		try {
			value = std::stoi(size);
		}
		catch (...) {
			size_t pos = source.find(size);
			if (pos == source.npos) {
				dout << "Shader error in a uniform array : " << name << " can't find it's numeric size !\n";
				return;
			}
			std::string line = source.substr(pos, source.find('\n', pos) - pos - 1);

			std::stringstream sline(line);
			std::string word;

			while (sline >> word) {
				try {
					value = stoi(word);
				}
				catch (...) { continue; }
			}
		}
		if (value == 0) {
			dout << "Shader error : failed to find uniform array size : " << name << std::endl;
		}

		std::string arrayName = name.substr();
	}
}

void ns::Shader::readUniformsNamesFromSource(const std::string& source)
{
	std::stringstream file(source);

	std::vector<std::string> uniforms;

	std::string word;
	while (file >> word) {
		if (word == "uniform") {
			file >> word;
			
			//struct uniform 
			if (word == "struct") {
				std::vector<std::string> names;
				std::vector<std::string> vars;

				size_t bracketPos;

				do
				{
					if (!(file >> word)) break;

					bracketPos = word.find('{');
				} while (bracketPos == word.npos);

				bool bracketFound = false;
				do
				{
					do {
						std::string previousWord = word;
						if (!(file >> word)) return;

						bracketFound = (word.find('}') != word.npos);
						if (bracketFound) break;

						size_t semicolon = word.find(';');
						if (semicolon == word.npos) continue;

						if (semicolon == 0) {
							vars.push_back(previousWord);
							break;
						}
						else {
							word.erase(semicolon);
							vars.push_back(word);
							break;
						}											
					} while (true);
				} while (!bracketFound);

				if (word.size() == 1) {
					file >> word;
				}
				size_t pos = word.find('}');
				if (pos != word.npos) word.erase(pos);

				pos = word.find(';');
				if (pos != word.npos) word.erase(pos);

				names.push_back(word);

				treatUniformArrays(names, source);

				std::vector<std::string> realNames;

				for (const auto& var : vars)
					realNames.push_back(names[0] + '.' + var);

				treatUniformArrays(realNames, source);

				for (const auto& name : realNames)
					dout << name << std::endl;
			}
			else {
				//get the uniform name
				file >> word;

				//remove semicolon
				size_t semiColonPos = word.find(';');
				if (semiColonPos != word.npos)
					word.erase(semiColonPos);

				uniforms.push_back(word);
			}
		}
	}
	treatUniformArrays(uniforms, source);
}

int ns::Shader::getUniformLocation(const std::string& name) const
{
#	ifndef NDEBUG
	if (uniformsNames_.find(name) != uniformsNames_.end())
		return uniformsNames_[name];

	dout << "Shader::getUniformLocation error ! uniform name :" << name << " is not registered !\n";
	return glGetUniformLocation(id, name.c_str());

#	else
	return uniformsNames_[name];
#	endif // !NDEBUG
}

void ns::Shader::use() const
{
	if (Shader::currentlyBindedShader != id) {
		glUseProgram(id);
		Shader::currentlyBindedShader = id;
	}
}

void ns::Shader::forceUse() const
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
		dout << "ERROR in the vertex shader \n" << errorLog << '\n';
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
			dout << "ERROR in the geometry shader \n" << errorLog << '\n';
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
		dout << "ERROR in the fragment shader \n" << errorLog << '\n';
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
		dout << "ERROR while linking shaders \n" << errorLog << '\n';
	}
	use();
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

template<typename T>
inline void ns::Shader::set(const char* name, T const& value) const
{
	static_assert(false, "type is not supported by ns::Shader class");
}
template <>
inline void ns::Shader::set(const char* name, int const& value) const
{
	use();
	glUniform1i(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, unsigned int const& value) const
{
	use();
	glUniform1ui(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, bool const& value) const
{
	use();
	glUniform1i(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, float const& value) const
{
	use();
	glUniform1f(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, glm::vec2 const& value) const
{
	use();
	glUniform2fv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::vec3 const& value) const
{
	use();
	glUniform3fv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::vec4 const& value) const
{
	use();
	glUniform4fv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::mat4 const& value) const
{
	use();
	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, false, &value[0][0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::ivec2 const& value) const
{
	use();
	glUniform2iv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::ivec3 const& value) const
{
	use();
	glUniform3iv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::ivec4 const& value) const
{
	use();
	glUniform4iv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, double const& value) const
{
	use();
	glUniform1d(glGetUniformLocation(id, name), value);
}
template <>
inline void ns::Shader::set(const char* name, glm::dvec2 const& value) const
{
	use();
	glUniform2dv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::dvec3 const& value) const
{
	use();
	glUniform3dv(glGetUniformLocation(id, name), 1, &value[0]);
	//__debugbreak();
}
template <>
inline void ns::Shader::set(const char* name, glm::dvec4 const& value) const
{
	use();
	glUniform4dv(glGetUniformLocation(id, name), 1, &value[0]);
}
template <>
inline void ns::Shader::set(const char* name, glm::dmat4 const& value) const
{
	use();
	glUniformMatrix4dv(glGetUniformLocation(id, name), 1, false, &value[0][0]);
	//__debugbreak();
}

void ShadersetFixLinkFunction(){
	ns::Shader s("");
	s.set<int>("", 0);
	s.set<unsigned>("", 0);
	s.set<bool>("", 0);
	s.set<float>("", 0);
	s.set<double>("", 0);
	s.set<glm::vec2>("", {0, 0});
	s.set<glm::vec3>("", {0, 0, 0});
	s.set<glm::vec4>("", {0, 0, 0, 0});
	s.set<glm::mat4>("", glm::mat4());
	s.set<glm::ivec2>("", { 0, 0 });
	s.set<glm::ivec3>("", { 0, 0, 0 });
	s.set<glm::ivec4>("", { 0, 0, 0, 0 });
	s.set<glm::dvec2>("", { 0, 0 });
	s.set<glm::dvec3>("", { 0, 0, 0 });
	s.set<glm::dvec4>("", { 0, 0, 0, 0 });
	s.set<glm::dmat4>("", glm::dmat4());
}
