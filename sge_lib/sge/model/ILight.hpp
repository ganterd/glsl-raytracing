#ifndef SGE_I_LIGHT_HPP
#define SGE_I_LIGHT_HPP

#include <glm/glm.hpp>

#include <sge/utils/export.hpp>

namespace SGE
{
	class Entity;
	class ILight
	{
	public:
		enum Type{
			Point,
			Area
		};

	private:
		glm::vec3 mPosition;
		glm::vec3 mColor;
		float mIntensity;
		glm::vec2 mSize;
		glm::vec3 mDirection;
		glm::vec3 mUp;
		Type mType;

	public:
		Entity* mParent;

		Export ILight()
		{
			mPosition = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			mColor = glm::vec3(1.0f, 1.0f, 1.0f);
			mIntensity = 0.1f;
			mType = Point;
		};

		Export void setType(Type t){ mType = t; };
		Export void setPosition(glm::vec3 p){ mPosition = p; };
		Export void setColor(glm::vec3 c){ mColor = c; };
		Export void setIntensity(float i){ mIntensity = i; };
		Export void setSize(const glm::vec2& s){ mSize = s; };
		Export void setDirection(const glm::vec3& d){ mDirection = d; };
		Export void setUpVector(const glm::vec3& u){ mUp = u; };

		Export Type getType(){ return mType; };
		Export glm::vec3 getPosition(){ return mPosition; };
		Export glm::vec3 getColor() { return mColor; };
		Export float getIntensity(){ return mIntensity; };
		Export glm::vec2 getSize(){ return mSize; };
		Export glm::vec3 getDirection(){ return mDirection; };
		Export glm::vec3 getUpVector(){ return mUp; };
	};
}

#endif
