#include "shader.hpp"
#include <stdexcept>
#include <glm\gtc\type_ptr.hpp>


// ================================================================================================
Shader::Shader(const char *vert, const char *geom, const char *frag) :
	m_program{0}
{
	// Load the individual shaders
	GLuint vshader = loadVertexSource(vert);
	GLuint gshader = loadGeometrySource(geom);
	GLuint fshader = loadFragmentSource(frag);

	// Create the program handle, and link in the shaders
	m_program = glCreateProgram();
	if (vshader) glAttachShader(m_program, vshader);
	if (gshader) glAttachShader(m_program, gshader);
	if (fshader) glAttachShader(m_program, fshader);
	glLinkProgram(m_program);

	// Check for linking errors
	int success;
	GLchar infoLog[512];
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
		throw std::runtime_error(std::string("Program linking error: '")
			+ infoLog + "'");
	}

	// Release unneeded shaders now that we are linked
	if (vshader) glDeleteShader(vshader);
	if (gshader) glDeleteShader(gshader);
	if (fshader) glDeleteShader(fshader);
}

// ================================================================================================
Shader::~Shader()
{
	release();
	glDeleteProgram(m_program);
}

// ================================================================================================
void Shader::bind()
{
	glUseProgram(m_program);
}

// ================================================================================================
void Shader::release()
{
	glUseProgram(0);
}

// ================================================================================================
void Shader::setUniform(const std::string& name, bool val)
{
	const GLuint loc = glGetUniformLocation(m_program, name.c_str());
	glUniform1i(loc, (int)val);
}

// ================================================================================================
void Shader::setUniform(const std::string& name, int val)
{
	const GLuint loc = glGetUniformLocation(m_program, name.c_str());
	glUniform1i(loc, val);
}

// ================================================================================================
void Shader::setUniform(const std::string& name, float val)
{
	const GLuint loc = glGetUniformLocation(m_program, name.c_str());
	glUniform1f(loc, val);
}

// ================================================================================================
void Shader::setUniform(const std::string& name, const vec2f& val)
{
	const GLuint loc = glGetUniformLocation(m_program, name.c_str());
	glUniform2fv(loc, 1, glm::value_ptr(val));
}

// ================================================================================================
void Shader::setUniform(const std::string& name, const mat4f& val)
{
	const GLuint loc = glGetUniformLocation(m_program, name.c_str());
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

// ================================================================================================
GLuint Shader::loadVertexSource(const char *vert)
{
	if (!vert) return 0;

	// Create shader, assign source, and compile
	const GLchar *source = reinterpret_cast<const GLchar *>(vert);
	GLuint shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	int success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		throw std::runtime_error(std::string("Vertex shader compilation error: '")
			+ infoLog + "'");
	}

	// Return compiled shader
	return shader;
}

// ================================================================================================
GLuint Shader::loadGeometrySource(const char *geom)
{
	if (!geom) return 0;

	// Create shader, assign source, and compile
	const GLchar *source = reinterpret_cast<const GLchar *>(geom);
	GLuint shader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	int success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		throw std::runtime_error(std::string("Geometry shader compilation error: '")
			+ infoLog + "'");
	}

	// Return compiled shader
	return shader;
}

// ================================================================================================
GLuint Shader::loadFragmentSource(const char *frag)
{
	if (!frag) return 0;

	// Create shader, assign source, and compile
	const GLchar *source = reinterpret_cast<const GLchar *>(frag);
	GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	int success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		throw std::runtime_error(std::string("Fragment shader compilation error: '")
			+ infoLog + "'");
	}

	// Return compiled shader
	return shader;
}



const char * const ParticleVertexShaderSource = R"(
	#version 330 core

	layout(location = 0) in float inMass;
	layout(location = 1) in vec2  inPos;
	layout(location = 2) in vec2  inVel;
	layout(location = 3) in vec2  inAcc;

	out vec2 vfPos;

	uniform mat4 View;
	uniform mat4 Projection;
	uniform float Time;

	void main()
	{
		vec4 pos = vec4(inPos, 0, 1);
		vfPos = inPos;
		gl_Position = Projection * View * pos;
	}
)";
const char * const ParticleGeometryShaderSource = R"(
	#version 330 core
)";
const char * const ParticleFragmentShaderSource = R"(
	#version 330 core

	in vec2 vfPos;

	layout(location = 0) out vec4 FragColor;

	void main()
	{
		float dist = length(vfPos);
		FragColor = vec4(dist, 1, 1, 1);
	}
)";