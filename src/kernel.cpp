#include "kernel.hpp"
#include "gpu.hpp"
#include <iostream>


// ================================================================================================
Kernel::Kernel(const char *source, const char *fname) :
	m_program{nullptr},
	m_kernel{nullptr},
	m_fname{fname},
	m_state{IDLE},
	m_mutex{},
	m_thread{nullptr}
{
	cl_int clerr;
	CL_CHECK_RETURN_FATAL(m_program = clCreateProgramWithSource(g_clContext, 1, &source, 0, &clerr),
		clerr, m_program, "Could not create OpenCL program from source.");

	// Build the program
	if (clerr = clBuildProgram(m_program, 0, nullptr, nullptr, nullptr, nullptr)) {
		std::cerr << "Failed to build OpenCL program (" << clerr << ")." << std::endl;
		char cllog[8192];
		CL_CHECK_FATAL(clGetProgramBuildInfo(m_program, g_clDevice, CL_PROGRAM_BUILD_LOG, 8192, cllog, nullptr),
			"Could not get the program build info log");
		
		throw std::runtime_error(std::string("OpenCL program build error: '") + cllog + "'");
	}

	// Create the kernel
	CL_CHECK_RETURN_FATAL(m_kernel = clCreateKernel(m_program, fname, &clerr), clerr, m_kernel,
		"Could not create OpenCL kernel from program with entry point '%s'", fname);
}

// ================================================================================================
Kernel::~Kernel()
{
	if (m_thread) {
		m_thread->join();
		delete m_thread;
	}
}

// ================================================================================================
void Kernel::setKernelArgument(unsigned int pos, size_t size, const void* arg)
{
	CL_CHECK_FATAL(clSetKernelArg(m_kernel, pos, size, arg),
		"Could not set kernel argument %d for kernel '%s'", pos, m_fname.c_str());
}

// ================================================================================================
void Kernel::executeNDRange(unsigned int numdim, const size_t* worksize, bool wait)
{
	CL_CHECK_FATAL(
		clEnqueueNDRangeKernel(g_clCommandQueue, m_kernel, numdim, nullptr, worksize, nullptr, 0, nullptr, nullptr),
		"Could not queue execution of kernel '%s'", m_fname.c_str());

	{
		tthread::lock_guard<tthread::mutex> lock(m_mutex);
		m_state = WORKING;
	}

	if (wait) {
		CL_CHECK_FATAL(clFinish(g_clCommandQueue),
			"Could not wait for kernel '%s' to finish on main thread", m_fname.c_str());

		{
			tthread::lock_guard<tthread::mutex> lock(m_mutex);
			m_state = IDLE;
		}
	}
	else {
		auto waitfunc = [](void *args) -> void {
			Kernel *kern = static_cast<Kernel*>(args);

			CL_CHECK_FATAL(clFinish(g_clCommandQueue),
				"Could not wait for kernel '%s' to finish on wait thread", kern->m_fname.c_str());

			{
				tthread::lock_guard<tthread::mutex> lock(kern->m_mutex);
				kern->m_state = IDLE;
			}
		};

		if (m_thread) {
			m_thread->join();
			delete m_thread;
		}

		m_thread = new tthread::thread(waitfunc, this);
	}
}



// ================================================================================================
// ================================================================================================
const char * const ParticleKernelSource = R"(
	//#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable

	// Particle struct (mirror of the host Particle type)
	typedef struct __attribute__((packed)) Particle
	{
		float mass;
		float2 pos;
		float2 vel;
		float2 acc;
	} Particle;

	__kernel void Solve(__global __read_only const Particle * src, __global __write_only Particle * dst,
						const float DeltaTime, const float TotalTime) 
	{
		const int IDX = get_global_id(0);
		const float SRCX = src[IDX].pos.x;
		const float SRCY = src[IDX].pos.y;
		float2 force = (float2)(0, 0);

		// Force from central attractor
		float2 diff = src[IDX].pos;
		float difflen = length(diff) + 1;
		float scale = 1.0 / pow(difflen, 3);
		force += (scale * diff);

		// Additional values that are nice to know and are used below (maybe)
		const float angle = atan2(SRCY, SRCX);
		const float distToCenter = length(src[IDX].pos);

		// Add the effects of an additional velocity field
		float afx = SRCY;
		float afy = -SRCX;
		float2 vfield = ((float2)(afx, afy) * 0.1f);

		// And another velocity field
		float pulseamt = sin(TotalTime * 2.0f) * 1.0f;
		vfield.x += (pulseamt * cos(angle));
		vfield.y += (pulseamt * sin(angle));

		// Solve final changes
		float2 dAcc = (-force / src[IDX].mass) * 5.0f;
		float2 dVel = src[IDX].vel + (dAcc * DeltaTime);
		float2 dPos = src[IDX].pos + (dVel * DeltaTime) + (vfield * DeltaTime / (distToCenter + 1));

		// Write solution to output array
		dst[IDX].mass = src[IDX].mass;
		dst[IDX].pos = dPos;
		dst[IDX].vel = dVel;
		dst[IDX].acc = dAcc;
	}
)";