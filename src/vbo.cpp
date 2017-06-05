#include "vbo.hpp"


// ================================================================================================
VertexBuffer::VertexBuffer(size_t size, GLenum usage) :
	m_vao{0},
	m_vbo{0},
	m_isMapped{false},
	m_size{size},
	m_usage{usage}
{
	glGenVertexArrays(1, &m_vao);
	if (!m_vao)
		throw std::runtime_error("Could not allocate VertexArrayObject.");

	glGenBuffers(1, &m_vbo);
	if (!m_vbo)
		throw std::runtime_error("Could not allocate VertexBufferObject.");

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ================================================================================================
VertexBuffer::~VertexBuffer()
{
	if (m_isMapped)
		unmapBuffer();

	if (m_vao)
		glDeleteVertexArrays(1, &m_vao);
	if (m_vbo)
		glDeleteBuffers(1, &m_vbo);
}

// ================================================================================================
void VertexBuffer::setFormat(const vertex_format_specifier_t *fmt, size_t count)
{
	if (m_isMapped)
		throw std::runtime_error("Cannot set the format of a vertex buffer that is mapped in memory.");

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	for (size_t i = 0; i < count; ++i) {
		const vertex_format_specifier_t& cfmt = fmt[i];
		glEnableVertexAttribArray(cfmt.location);
		glVertexAttribPointer(cfmt.location, cfmt.size, cfmt.type, GL_FALSE, cfmt.stride, 
			(GLvoid*)cfmt.offset);
	}

	glBindVertexArray(0);
}

// ================================================================================================
void VertexBuffer::setData(const void * const data)
{
	if (m_isMapped)
		throw std::runtime_error("Cannot set contents of a buffer mapped to host memory.");

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ================================================================================================
void* VertexBuffer::mapBuffer(GLenum flag)
{
	if (m_isMapped)
		throw std::runtime_error("Cannot map a buffer that is already mapped.");

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	void *mapped = glMapBuffer(GL_ARRAY_BUFFER, flag);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	if (!mapped)
		throw std::runtime_error("Could not map the VBO to host memory.");

	m_isMapped = true;
	return mapped;
}

// ================================================================================================
void* VertexBuffer::mapBufferRange(GLenum flag, size_t offset, size_t length)
{
	if (m_isMapped)
		throw std::runtime_error("Cannot map a buffer that is already mapped.");

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	void *mapped = glMapBufferRange(GL_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)length, flag);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (!mapped)
		throw std::runtime_error("Could not map the VBO to host memory.");

	m_isMapped = true;
	return mapped;
}

// ================================================================================================
void VertexBuffer::unmapBuffer()
{
	if (!m_isMapped)
		throw std::runtime_error("Cannot unmap a buffer that is not mapped to host memory.");

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	bool success = (glUnmapBuffer(GL_ARRAY_BUFFER) == GL_TRUE);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (!success)
		throw std::runtime_error("Could not unmap the buffer from host memory.");

	m_isMapped = false;
}

// ================================================================================================
#pragma warning(disable : 4267)
void VertexBuffer::drawBuffer(GLenum primitiveType, size_t start, size_t count)
{
	if (m_isMapped)
		throw std::runtime_error("Cannot draw a VBO that is currently mapped to host memory.");

	glBindVertexArray(m_vao);
	glDrawArrays(primitiveType, start, count);
	glBindVertexArray(0);
}
#pragma warning(default : 4267)