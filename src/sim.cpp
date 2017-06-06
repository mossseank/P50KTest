#include "sim.hpp"
#include "particle.hpp"
#include <iostream>


// ================================================================================================
Simulation::Simulation(size_t pcount, float xdim, float ydim) :
	m_camera{nullptr},
	m_buffers{nullptr, nullptr},
	m_particleShader{nullptr},
	m_particleKernel{nullptr},
	m_swapped{false},
	m_pCount{pcount},
	m_xdim{xdim},
	m_ydim{ydim}
{
	m_camera = new Camera(-2.5, 2.5, 2.5, -2.5);

	m_particleShader = new Shader(ParticleVertexShaderSource, nullptr, ParticleFragmentShaderSource);

	m_particleKernel = new Kernel(ParticleKernelSource, "Solve");

	const size_t PSIZE = sizeof(Particle) * m_pCount;
	m_buffers[0] = new VertexBuffer(PSIZE, GL_STATIC_DRAW);
	m_buffers[1] = new VertexBuffer(PSIZE, GL_STATIC_DRAW);
	m_buffers[0]->setFormat(ParticleFormatSpecifier, ParticleFormatSpecifierCount);
	m_buffers[1]->setFormat(ParticleFormatSpecifier, ParticleFormatSpecifierCount);

	initilizeParticles();
}

// ================================================================================================
Simulation::~Simulation()
{
	if (m_camera)
		delete m_camera;

	if (m_particleShader)
		delete m_particleShader;
	
	if (m_particleKernel)
		delete m_particleKernel;

	if (m_buffers[0]) {
		delete m_buffers[0];
		delete m_buffers[1];
	}
}

// ================================================================================================
void Simulation::render(float dtime)
{
	static float totalTime = 0.0f;

	cl_mem src = getSourceMem();
	cl_mem dst = getDestinationMem();
	VertexBuffer *srcbuf = getSourceBuffer();
	VertexBuffer *dstbuf = getDestinationBuffer();

	srcbuf->acquireCLMemory();
	dstbuf->acquireCLMemory();
	m_particleKernel->setKernelArgument(0, sizeof(src), &src);
	m_particleKernel->setKernelArgument(1, sizeof(dst), &dst);
	m_particleKernel->setKernelArgument(2, sizeof(dtime), &dtime);
	m_particleKernel->setKernelArgument(3, sizeof(totalTime), &totalTime);
	size_t global[1] = { m_pCount };
	m_particleKernel->executeNDRange(1, global, true);
	srcbuf->releaseCLMemory();
	dstbuf->releaseCLMemory();

	m_particleShader->bind();
	m_particleShader->setUniform("Projection", m_camera->projection());
	m_particleShader->setUniform("View", m_camera->view());
	m_particleShader->setUniform("Time", (totalTime += dtime));
	srcbuf->drawBuffer(GL_POINTS, 0, m_pCount);
	m_particleShader->release();

	m_swapped = !m_swapped;
}

// ================================================================================================
void Simulation::initilizeParticles()
{
	static const auto randflt = [](float low = 0.0f, float high = 1.0f) -> float {
		float flt = rand() / (float)RAND_MAX;
		return (flt * (high - low)) + low;
	};
	const float halfx = m_xdim / 2.0f;
	const float halfy = m_ydim / 2.0f;

	Particle *pdata = new Particle[m_pCount];

	for (size_t i = 0; i < m_pCount; ++i) {
		Particle& part = pdata[i];

		part.mass = 1.0f;
		part.pos = { randflt(-halfx, halfx), randflt(-halfy, halfy) };
	}

	m_buffers[0]->setData(pdata);
	delete pdata;
}