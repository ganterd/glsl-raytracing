#ifndef SGE_ISHADER_HPP
#define SGE_ISHADER_HPP

#ifndef SGE_MVP_SHADER_MAT
#define SGE_MVP_SHADER_MAT "modelViewProjection"
#endif

#define SGE_SHADER_BUFFER_WIDTH "bufferWidth"
#define SGE_SHADER_BUFFER_HEIGHT "bufferHeight"

#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <glm/glm.hpp>

#include "IRenderTarget.hpp"

namespace SGE
{
	class Export IShader
	{
	protected:
		int targetBufferWidth;
		int targetBufferHeight;

		IRenderTarget* renderTarget;

		virtual void updateTargetBufferDimensions() = 0;

	public:

		virtual bool loadFromFiles(std::string vertFile, std::string fragFile) = 0;
		virtual bool loadFromFiles(std::string vFile, std::string fFile, std::string gFile) = 0;

		virtual void enable() = 0;
		virtual void disable() = 0;

		virtual void setMVP(glm::mat4 mvpMat) = 0;

		void setTargetBufferDimensions(int w, int h);
		const char* readFile(const char* filePath);

		void setRenderTarget(IRenderTarget* rt);
		IRenderTarget* getRenderTarget();

		virtual void setVariable(std::string name, bool value) = 0;
		virtual void setVariable(std::string name, int value) = 0;
		virtual void setVariable(std::string name, float value) = 0;
		virtual void setVariable(std::string name, glm::vec2 value) = 0;
		virtual void setVariable(std::string name, glm::vec3 value) = 0;
		virtual void setVariable(std::string name, glm::vec4 value) = 0;
		virtual void setVariable(std::string name, glm::mat4 value) = 0;
	};
}

#endif
