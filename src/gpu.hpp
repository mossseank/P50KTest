#pragma once

#include <CL\cl.hpp>
#include <GL\glew.h>
#include <glfw\glfw3.h>

#include <stdexcept>
#include <string>
#include <sstream>
#include "camera.hpp"


extern Camera *g_camera;
extern GLFWwindow *g_windowPtr;
extern cl_device_id g_clDevice;
extern cl_context g_clContext;
extern cl_command_queue g_clCommandQueue;

#define CL_CHECK(stmt, msg, ...) \
	_clCheckError((stmt), __FILE__, __LINE__, ([&]() -> std::string { \
		char outstr[1024]; \
		snprintf(outstr, 1024, msg, __VA_ARGS__); \
		return std::string(outstr); \
	})(), false)
#define CL_CHECK_FATAL(stmt, msg, ...) \
	_clCheckError((stmt), __FILE__, __LINE__, ([&]() -> std::string { \
		char outstr[1024]; \
		snprintf(outstr, 1024, msg, __VA_ARGS__); \
		return std::string(outstr); \
	})(), true)
#define CL_CHECK_RETURN_FATAL(stmt, errval, stmtval, msg, ...) \
	_clCheckError(([&]() -> cl_int { (stmt); return (errval) ? (errval) : !(stmtval); })(), \
		__FILE__, __LINE__, ([&]() -> std::string { \
			char outstr[1024]; \
			snprintf(outstr, 1024, msg, __VA_ARGS__); \
			return std::string(outstr); \
		})(), true)
bool _clCheckError(cl_int err, const char *file, unsigned int line, const std::string& msg, bool fatal);

void initialize_gl();
void initialize_cl();

size_t getMaxWorkGroupSize();

void shutdown_gl();
void shutdown_cl();