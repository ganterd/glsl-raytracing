#include "GLSLShader.hpp"

namespace SGE
{
	GLSLShader::GLSLShader()
	{
		//this->renderTarget = new GLSLRenderTarget();
		//LOG(DEBUG) << "Created render target [" << this->renderTarget << "]";
	}

	GLSLShader::~GLSLShader()
	{
		//delete this->renderTarget;
	}

	bool GLSLShader::loadFromFiles(std::string vFile, std::string fFile)
	{
		return this->loadFromFiles(vFile, "", fFile);
	}

	bool GLSLShader::loadFromFiles(std::string vFile, std::string gFile, std::string fFile)
	{

		const char* vShaderCode = readShaderCode(vFile);
		const char* gShaderCode = readShaderCode(gFile);
		const char* fShaderCode = readShaderCode(fFile);

		LOG(INFO) << "Loading shaders";
		this->shaderID = glCreateProgram();
		if(vShaderCode)
		{
			LOG(INFO) << " |- Vert: " << vFile;
			loadShader(vShaderCode, GL_VERTEX_SHADER, shaderID);
		}
		if(gShaderCode)
		{
			LOG(INFO) << " |- Geom: " << gFile;
			loadShader(gShaderCode, GL_GEOMETRY_SHADER, shaderID);
		}
		if(fShaderCode)
		{
			LOG(INFO) << " |- Frag: " << fFile;
			loadShader(fShaderCode, GL_FRAGMENT_SHADER, this->shaderID);
		}

		glLinkProgram(this->shaderID);

		return true;
	}

	GLuint GLSLShader::getUniformLocation(std::string name)
	{
		std::map<std::string, GLuint>::iterator entry = mUniformsMap.find(name);
		if(entry != mUniformsMap.end())
		{
			return entry->second;
		}
		else
		{
			GLuint loc = glGetUniformLocation(shaderID, name.c_str());
			mUniformsMap[name] = loc;

			if(loc == -1)
				LOG(WARNING) << "Shader has no uniform \"" << name << "\"";
		}
	}

	void GLSLShader::setVariable(std::string name, bool value)
	{
		glUniform1i(getUniformLocation(name), value ? 1 : 0);
	}

	void GLSLShader::setVariable(std::string name, int value)
	{
		glUniform1i(getUniformLocation(name), value);
	}

	void GLSLShader::setVariable(std::string name, float value)
	{
		glUniform1f(getUniformLocation(name), value);
	}

	void GLSLShader::setVariable(std::string name, glm::vec2 value)
	{
		glUniform2f(getUniformLocation(name), value.x, value.y);
	}

	void GLSLShader::setVariable(std::string name, glm::vec3 value)
	{
		glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
	}

	void GLSLShader::setVariable(std::string name, glm::vec4 value)
	{
		glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
	}

	void GLSLShader::setVariable(std::string name, glm::mat4 value)
	{
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
	}

	const char* GLSLShader::readShaderCode(std::string file)
	{
		if(file.empty())
		{
			return NULL;
		}

		const char* shaderCode = readFile(file.c_str());
		if(!shaderCode){
			LOG(WARNING) << "Could not read shader file: " << file;
			return NULL;
		}

		return shaderCode;
	}

	GLuint GLSLShader::loadShader(const char* shaderCode, GLuint shaderType, GLuint targetProgram)
	{
		if(shaderCode == NULL)
			return 0;

		GLuint shaderID = glCreateShader(shaderType);

		glShaderSource(shaderID, 1, &shaderCode, 0);
		glCompileShader(shaderID);

		GLint compiled;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
		if (compiled == GL_FALSE)
		{
			GLint logLength = 0;
			glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
			GLchar* errorString = new GLchar[logLength];
			glGetShaderInfoLog(shaderID, logLength, &logLength, errorString);

			const char* sType = shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment";
			LOG(ERROR) << "Couldn't compile " << sType << " shader:" << std::endl << errorString << std::endl;
			glDeleteShader(shaderID);
			delete[] errorString;
			return false;
		}


		glAttachShader(targetProgram, shaderID);
		return shaderID;
	}

	void GLSLShader::enable()
	{
		glUseProgram(this->shaderID);
		glUniform1f(this->locBufferWidth, (float)targetBufferWidth);
		glUniform1f(this->locBufferHeight, (float)targetBufferHeight);
	}

	void GLSLShader::disable()
	{
		glUseProgram(0);
	}

	void GLSLShader::updateTargetBufferDimensions()
	{
	}
}
