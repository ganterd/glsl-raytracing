#include "DiffuseTexture.hpp"

namespace SGE
{
	DiffuseTexture::DiffuseTexture()
	{
		this->m_type = textureType_DIFFUSE;
	}

	void DiffuseTexture::bindTexture(int textureUnit)
	{
	}

	void DiffuseTexture::unbindTexture()
	{
	}
}
