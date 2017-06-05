#pragma once

#include <CL\cl.hpp>
#include <GL\glew.h>
#include <glfw\glfw3.h>


struct vertex_format_specifier_t
{
	GLuint location;
	GLsizei size;
	unsigned int type;
	GLuint stride;
	size_t offset;
};


class VertexBuffer
{
private:
	GLuint m_vao;
	GLuint m_vbo;
	bool m_isMapped;
	const size_t m_size;
	const GLenum m_usage;

public:
	VertexBuffer(size_t size, GLenum usage);
	~VertexBuffer();

	inline GLuint getVboName() const { return m_vbo; }
	inline GLuint getVaoName() const { return m_vao; }
	inline size_t getSize() const { return m_size; }
	inline GLenum getUsage() const { return m_usage; }

	void setFormat(const vertex_format_specifier_t *fmt, size_t count);
	void setData(const void * const data);

	void* mapBuffer(GLenum flag);
	void* mapBufferRange(GLenum flag, size_t offset, size_t length);
	void unmapBuffer();
	inline bool isMapped() const { return m_isMapped; }

	void drawBuffer(GLenum primitiveType, size_t start, size_t count);
};