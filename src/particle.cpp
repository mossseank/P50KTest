#include "particle.hpp"

const size_t ParticleFormatSpecifierCount = 4;
const vertex_format_specifier_t ParticleFormatSpecifier[4] = {
	{ 0, 1, GL_FLOAT, 7 * sizeof(GLfloat), 0 },					// Mass
	{ 1, 2, GL_FLOAT, 7 * sizeof(GLfloat), 1 * sizeof(GLfloat) },	// Position
	{ 2, 2, GL_FLOAT, 7 * sizeof(GLfloat), 3 * sizeof(GLfloat) },	// Velocity
	{ 3, 2, GL_FLOAT, 7 * sizeof(GLfloat), 5 * sizeof(GLfloat) }	// Acceleration
};