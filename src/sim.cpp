#include "sim.hpp"
#include "particle.hpp"
#include <iostream>


// ================================================================================================
Simulation::Simulation() :
	m_camera{nullptr},
	m_buffers{nullptr, nullptr},
	m_particleShader{nullptr}
{
	m_camera = new Camera(-10, 10, 10, -10);

	m_particleShader = new Shader(ParticleVertexShaderSource, nullptr, ParticleFragmentShaderSource);

	const size_t PSIZE = sizeof(Particle) * 1000;
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

	if (m_buffers[0]) {
		delete m_buffers[0];
		delete m_buffers[1];
	}
}

// ================================================================================================
void Simulation::render(float dtime)
{
	static float totalTime = 0.0f;

	m_particleShader->bind();
	m_particleShader->setUniform("Projection", m_camera->projection());
	m_particleShader->setUniform("View", m_camera->view());
	m_particleShader->setUniform("Time", (totalTime += dtime));
	m_buffers[0]->drawBuffer(GL_POINTS, 0, 1000);
	m_particleShader->release();
}

// ================================================================================================
void Simulation::initilizeParticles()
{
	static const auto randflt = [](float low = 0.0f, float high = 1.0f) -> float {
		float flt = rand() / (float)RAND_MAX;
		return (flt * (high - low)) + low;
	};

	Particle *pdata = new Particle[1000];

	for (size_t i = 0; i < 1000; ++i) {
		Particle& part = pdata[i];

		part.mass = 1.0f;
		part.pos = { randflt(-10, 10), randflt(-10, 10) };
		part.vel = { -5, -5 };
		part.acc = { 5, 5 };
		std::cout << "{" << part.x << ", " << part.y << "}" << std::endl;
	}

	m_buffers[0]->setData(pdata);
	delete pdata;
}