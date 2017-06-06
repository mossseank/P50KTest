#pragma once

#include "vbo.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "kernel.hpp"


class Simulation
{
private:
	Camera *m_camera;
	VertexBuffer* m_buffers[2];
	Shader *m_particleShader;
	Kernel *m_particleKernel;
	bool m_swapped;
	const size_t m_pCount;
	const float m_xdim;
	const float m_ydim;

public:
	Simulation(size_t pcount, float xdim, float ydim);
	~Simulation();

	void render(float dtime);

private:
	void initilizeParticles();

	inline size_t getSourceIndex() const { return m_swapped ? 1 : 0; }
	inline size_t getDestinationIndex() const { return m_swapped ? 0 : 1; }
	inline cl_mem getSourceMem() const { return m_buffers[m_swapped ? 1 : 0]->getCLMemory(); }
	inline cl_mem getDestinationMem() const { return m_buffers[m_swapped ? 0 : 1]->getCLMemory(); }
	inline VertexBuffer* getSourceBuffer() const { return m_buffers[m_swapped ? 1 : 0]; }
	inline VertexBuffer* getDestinationBuffer() const { return m_buffers[m_swapped ? 0 : 1]; }
};