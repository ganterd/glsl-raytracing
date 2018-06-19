#include "GLSLShader.hpp"

namespace SGE
{
	GLSLShader::GLSLShader()
	{
		this->renderTarget = new GLSLRenderTarget();
		LOG(DEBUG) << "Created render target [" << this->renderTarget << "]";
	}
	
	GLSLShader::~GLSLShader()
	{
		delete this->renderTarget;
	}
	
	bool GLSLShader::loadFromFiles(std::string vFile, std::string fFile)
	{
		this->loadFromFiles(vFile, "", fFile);
	}
	
	bool GLSLShader::loadFromFiles(std::string vFile, std::string gFile, std::string fFile)
	{
		LOG(INFO) << "Loading shaders";
		LOG(INFO) << " |- Vert: " << vFile;
		LOG(INFO) << " |- Geom: " << gFile;
		LOG(INFO) << " |- Frag: " << fFile;

		this->shaderID = glCreateProgram();
		loadShader(readShaderCode(vFile), GL_VERTEX_SHADER, this->shaderID);
		loadShader(readShaderCode(gFile), GL_GEOMETRY_SHADER, this->shaderID);
		loadShader(readShaderCode(fFile), GL_FRAGMENT_SHADER, this->shaderID);
		glLinkProgram(this->shaderID);
		
		
		// TODO: Re-do the whole shader variables thing. Gets a bit messy
		this->locMVP = glGetUniformLocation(this->shaderID, SGE_MVP_SHADER_MAT);
		if(this->locMVP == -1)
		{
			LOG(WARNING) << "Couldn't find shader variable '" << SGE_MVP_SHADER_MAT << "'";
		}
		
		this->locBufferWidth = glGetUniformLocation(this->shaderID, SGE_SHADER_BUFFER_WIDTH);
		if(this->locBufferWidth == -1)
		{
			LOG(WARNING) << "Couldn't find shader variable '" << SGE_SHADER_BUFFER_WIDTH << "'";
		}
		
		this->locBufferHeight = glGetUniformLocation(this->shaderID, SGE_SHADER_BUFFER_HEIGHT);
		if(this->locBufferHeight == -1)
		{
			LOG(WARNING) << "Couldn't find shader variable '" << SGE_SHADER_BUFFER_HEIGHT << "'";
		}
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
			
		// TODO: Add some error sutff in here
		GLuint shaderID = glCreateShader(shaderType);
		
		glShaderSource(shaderID, 1, &shaderCode, 0);
		glCompileShader(shaderID);
		
		glAttachShader(targetProgram, shaderID);
		return shaderID;
	}
	
	void GLSLShader::enable()
	{
		glUseProgram(this->shaderID);
		glUniform1f(this->locBufferWidth, (float)targetBufferWidth);
		glUniform1f(this->locBufferHeight, (float)targetBufferHeight);
		
		if(this->renderTarget == NULL)
			LOG(ERROR) << "Shader has no render target!";
		this->renderTarget->bind();
	}
	
	void GLSLShader::disable()
	{
		this->renderTarget->unbind();
		glUseProgram(0);
	}
	
	void GLSLShader::setMVP(glm::mat4 mvpMat)
	{
		glUniformMatrix4fv(this->locMVP, 1, GL_FALSE, &mvpMat[0][0]);
	}
	
	void GLSLShader::updateTargetBufferDimensions()
	{
	}
}