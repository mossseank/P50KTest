#pragma once

#include <CL\cl.hpp>
#include <string>
#include <tinythread.h>


class Kernel
{
public:
	enum State : 
		unsigned char 
	{
		IDLE = 0,
		WORKING = 1
	};

private:
	cl_program m_program;
	cl_kernel m_kernel;
	std::string m_fname;
	State m_state;
	mutable tthread::mutex m_mutex;
	tthread::thread *m_thread;

public:
	Kernel(const char *source, const char *fname);
	~Kernel();

	inline const std::string& getFunctionName() const { return m_fname; }
	inline State getState() const { tthread::lock_guard<tthread::mutex> lock(m_mutex); return m_state; }
	inline bool isRunning() const { tthread::lock_guard<tthread::mutex> lock(m_mutex); return (m_state == WORKING); }

	void setKernelArgument(unsigned int pos, size_t size, const void* arg);

	void executeNDRange(unsigned int numdim, const size_t* worksize, bool wait);
};


// For simplicity, just embed the kernel source into the executable
extern const char * const ParticleKernelSource;