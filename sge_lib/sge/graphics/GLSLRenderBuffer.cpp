#include "GLSLRenderBuffer.hpp"

namespace SGE
{
	GLSLRenderBuffer::GLSLRenderBuffer(
			int width,
			int height,
			GLuint glFBO,
			IRenderBuffer::BufferType bufferType,
			ITexture::DataType dataType,
			GLuint glColorAttachment
	)
	{
		mTexture = new GLSLTexture();
		mTexture->m_width = width;
		mTexture->m_height = height;
		mTexture->m_dataType = dataType;
		this->m_glFrameBufferID = glFBO;
		this->m_glColorAttachment = glColorAttachment;
		this->m_bufferType = bufferType;

		GLuint glInternalFormat;
		GLuint glFormat;
		GLuint glDataType;
		switch(bufferType)
		{
			case BufferType::Color:
				glInternalFormat = GL_RGB;
				glFormat = GL_RGB;
				glDataType = GL_UNSIGNED_BYTE;
				LOG(DEBUG) << "Adding colour buffer";
				break;
			case BufferType::Normal:
				glInternalFormat = GL_RGB16F;
				glFormat = GL_RGB;
				glDataType = GL_FLOAT;
				LOG(DEBUG) << "Adding normals buffer";
				break;
			case BufferType::Position:
				glInternalFormat = GL_RGB32F;
				glFormat = GL_RGB;
				glDataType = GL_FLOAT;
				LOG(DEBUG) << "Adding positions buffer";
				break;
			case BufferType::Depth:
				glInternalFormat = GL_DEPTH24_STENCIL8;
				glFormat = GL_DEPTH_STENCIL;
				glDataType = GL_UNSIGNED_INT_24_8;
				LOG(DEBUG) << "Adding depth and stencil buffer";
				break;
			default:
				LOG(WARNING) << "Unrecognised buffer type requested";
				glInternalFormat = GL_RGB;
				glFormat = GL_RGB;
				glDataType = GL_UNSIGNED_BYTE;
		}

		glGenTextures(1, (GLuint*)&mTexture->mTextureID);
		glBindTexture(GL_TEXTURE_2D, mTexture->mTextureID);
		glTexImage2D(GL_TEXTURE_2D,	0, glInternalFormat, width, height, 0, glFormat, glDataType, NULL);
		if(this->m_glColorAttachment != GL_DEPTH_ATTACHMENT)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, m_glColorAttachment, GL_TEXTURE_2D, mTexture->mTextureID, 0);
		}
		else
		{
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mTexture->mTextureID, 0);
		}

		LOG(INFO) << "Creating GLSLRenderBuffer (" << width << "x" << height << ")";
	}

	GLSLRenderBuffer::~GLSLRenderBuffer()
	{
		glDeleteTextures(1, (GLuint*)&mTexture->mTextureID);
		glDeleteRenderbuffers(1, &m_glRenderBufferID);
	}

	void GLSLRenderBuffer::setGLTextureType(GLuint glTextureType)
	{
		this->m_glRenderBufferType = glTextureType;
	}

	GLuint GLSLRenderBuffer::getGLTextureType()
	{
		return this->m_glRenderBufferType;
	}

	void GLSLRenderBuffer::setGLColorAttachment(GLuint glColorAttachment)
	{
		this->m_glColorAttachment = glColorAttachment;
	}

	GLuint GLSLRenderBuffer::getGLColorAttachment()
	{
		return this->m_glColorAttachment;
	}

	void GLSLRenderBuffer::setGLDataType(GLuint glDataType)
	{
		this->m_glDataType = glDataType;
	}

	GLuint GLSLRenderBuffer::getGLDataType()
	{
		return this->m_glDataType;
	}

	void GLSLRenderBuffer::bindBuffer()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, this->m_glRenderBufferID);
	}

	void GLSLRenderBuffer::unbindBuffer()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void GLSLRenderBuffer::clear()
	{
		// TODO: Implement clear buffer
	}

	GLuint GLSLRenderBuffer::_bufferTypeToGLInternalFormat(IRenderBuffer::BufferType type)
	{
		GLuint glRenderBufferType;
		switch(type)
		{
			case BufferType::Color:
				glRenderBufferType = GL_RGB;
				break;
			case BufferType::Normal:
				glRenderBufferType = GL_RGB16F;
				break;
			case BufferType::Position:
				glRenderBufferType = GL_RGB32F;
				break;
			case BufferType::Depth:
				glRenderBufferType = GL_DEPTH_COMPONENT24;
				break;
			default:
				LOG(WARNING) << "Unrecognised buffer type requested";
				glRenderBufferType = GL_RGBA;
		}

		return glRenderBufferType;
	}

	bool GLSLRenderBuffer::checkFBOStatus()
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch(status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			return true;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			LOG(ERROR) << "FBO Error: Incomplete Attachment (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			LOG(ERROR) << "FBO Error: Incomplete Missing Attachment (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			LOG(ERROR) << "FBO Error: Incomplete Draw Buffer (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			LOG(ERROR) << "FBO Error: Incomplete Read Buffer (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LOG(ERROR) << "FBO Error: Unsupported (GL_FRAMEBUFFER_UNSUPPORTED)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			LOG(ERROR) << "FBO Error: Incomplete Multisample (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			LOG(ERROR) << "FBO Error: Incomplete Layer Targets (GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)";
			break;
		default:
			LOG(ERROR) << "FBO Error: For some reason, GL decided not to give a known error code...";
		}
		return false;
	}
}
