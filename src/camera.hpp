#pragma once

#include <GL\glew.h>
#include <glfw\glfw3.h>
#include <glm\vec4.hpp>
#include <glm\matrix.hpp>


class Camera
{
private:
	const glm::mat4 m_view;
	const glm::mat4 m_proj;

public:
	Camera(float left, float top, float right, float bottom);
	~Camera();

	inline const glm::mat4& view() const { return m_view; }
	inline const glm::mat4& projection() const { return m_proj; }
};