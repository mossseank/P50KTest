#pragma once

#include <gl\glew.h>
#include <glm\glm.hpp>
#include <string>


using vec2f = glm::vec2;
using mat4f = glm::mat4;


class Shader
{
private:
	GLuint m_program;

public:
	Shader(const char *vert, const char *geom, const char *frag);
	~Shader();

	void bind();
	void release();

	void setUniform(const std::string& name, bool val);
	void setUniform(const std::string& name, int val);
	void setUniform(const std::string& name, float val);
	void setUniform(const std::string& name, const vec2f& val);
	void setUniform(const std::string& name, const mat4f& val);

private:
	GLuint loadVertexSource(const char *vert);
	GLuint loadGeometrySource(const char *geom);
	GLuint loadFragmentSource(const char *frag);
};


// For simplicity, we are just going to embed the shader source directly into the executable.
extern const char * const ParticleVertexShaderSource;
extern const char * const ParticleGeometryShaderSource;
extern const char * const ParticleFragmentShaderSource;