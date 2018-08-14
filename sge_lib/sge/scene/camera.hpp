#ifndef SGE_CAMERA_HPP
#define SGE_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <sge/utils/export.hpp>
#include <sge/graphics/IShader.hpp>
#include <sge/graphics/ShaderManager.hpp>

namespace SGE
{
	class Camera
	{
	private:
		glm::vec3 mPosition;
		glm::vec3 mForwardVector;
		glm::vec3 mUpVector;

		float fov;
		float ratio;
		float nearPlane;
		float farPlane;

		glm::mat4 projectionMat;
		glm::mat4 viewMat;
		glm::mat4 vpMat;

		void updateProjectionMat();
	public:
		Export Camera();

		Export void setLookAt(const glm::vec3& p);
		Export void setLookVector(const glm::vec3& l);
		Export void setUpVector(const glm::vec3& u);
		Export void setPosition(const glm::vec3& p);
		Export void setFoV(float fov);
		Export void setAspectRatio(float r);
		Export void setNearPlaneDistance(float p);
		Export void setFarPlaneDistance(float p);

		Export glm::vec3 getPosition();
		Export glm::vec3 getForwardVector();
		Export glm::vec3 getUpVector();
		Export glm::mat4 getVPMat();

		Export void update();
	};
}

#endif
