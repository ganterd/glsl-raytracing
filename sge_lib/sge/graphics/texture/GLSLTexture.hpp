#ifndef SGE_GLSL_TEXTURE
#define SGE_GLSL_TEXTURE

#include <GL/glew.h>
#include <GL/glu.h>

#include <easylogging++.h>

#include <sge/graphics/texture/ITexture.hpp>

namespace SGE
{
	class Export GLSLTexture : public ITexture
	{
	public:
		GLSLTexture();
		~GLSLTexture();

		virtual void loadFromFile(const std::string& path, Type type);

		virtual void bindTexture(int textureUnit);
		virtual void unbindTexture();

		static GLuint dataTypeToGLDataType(ITexture::DataType dataType);
	};
}

#endif
