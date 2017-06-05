#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

// ================================================================================================
Camera::Camera(float left, float top, float right, float bottom) :
	m_view{ glm::lookAt(glm::vec3{ 0, 0, 1 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 0, 1, 0 }) },
	m_proj{ glm::ortho(left, right, bottom, top, 0.01f, 100.0f) }
{
	
}

// ================================================================================================
Camera::~Camera()
{

}