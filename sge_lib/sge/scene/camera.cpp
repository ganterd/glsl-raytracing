#include "camera.hpp"

namespace SGE
{
	Camera::Camera()
	{
		this->fov = 45.0f;
		this->ratio = 4.0f/3.0f;
		this->nearPlane = 0.01f;
		this->farPlane = 1000.0f;

		mPosition = glm::vec3(3.0f,2.0f,0.0f);
		mForwardVector = glm::vec3(1, 0, 0);
		mUpVector = glm::vec3(0,1,0);
	}

	void Camera::setLookAt(const glm::vec3& p)
	{
		mForwardVector = glm::normalize(p - mPosition);
	}

	void Camera::update()
	{
		viewMat = glm::lookAt(mPosition,	mPosition + mForwardVector, mUpVector);
		vpMat = projectionMat * viewMat;
	}

	void Camera::updateProjectionMat()
	{
		projectionMat = glm::perspective(fov, ratio, nearPlane, farPlane);
	}

	void Camera::setLookVector(const glm::vec3& l)
	{
		mForwardVector = glm::normalize(l);
		update();
	}

	void Camera::setUpVector(const glm::vec3& u)
	{
		mUpVector = u;
		update();
	}

	void Camera::setPosition(const glm::vec3& p )
	{
		mPosition = p;
		update();
	}

	void Camera::setFoV(float fov)
	{
		this->fov = fov;
		this->updateProjectionMat();
	}

	void Camera::setAspectRatio(float r)
	{
		this->ratio = r;
		this->updateProjectionMat();
	}

	void Camera::setNearPlaneDistance(float p)
	{
		this->nearPlane = p;
		this->updateProjectionMat();
	}

	void Camera::setFarPlaneDistance(float p)
	{
		this->farPlane = p;
		this->updateProjectionMat();
	}

	glm::vec3 Camera::getPosition()
	{
		return mPosition;
	}

	glm::vec3 Camera::getForwardVector()
	{
		return mForwardVector;
	}

	glm::vec3 Camera::getUpVector()
	{
		return mUpVector;
	}

	glm::mat4 Camera::getVPMat()
	{
		return this->vpMat;
	}
}
