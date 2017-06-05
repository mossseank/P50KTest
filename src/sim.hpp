#pragma once

#include "vbo.hpp"
#include "shader.hpp"
#include "camera.hpp"


class Simulation
{
private:
	Camera *m_camera;
	VertexBuffer* m_buffers[2];
	Shader *m_particleShader;

public:
	Simulation();
	~Simulation();

	void render(float dtime);

private:
	void initilizeParticles();
};