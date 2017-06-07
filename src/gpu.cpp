#include "gpu.hpp"
#include <GL\wglew.h>
#include <iostream>


// Activate the NVIDIA Optimus gpu settings
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}


Camera *g_camera = nullptr;
GLFWwindow *g_windowPtr = nullptr;
cl_device_id g_clDevice = nullptr;
cl_context g_clContext = nullptr;
cl_command_queue g_clCommandQueue = nullptr;
size_t OPENCL_MAX_WORK_GROUP_SIZE = 0;


void _glfw_error_callback(int err, const char *errstr)
{
	std::cerr << "GLFW Error (" << err << "):  \"" << errstr << "\"." << std::endl;
}

void _glfw_resize_callback(GLFWwindow *win, int width, int height)
{
	const float xscale = width / 1080.0f;
	const float yscale = height / 1080.0f;
	g_camera->onResize(-2.5f * xscale, 2.5f * yscale, 2.5f * xscale, -2.5f * yscale);
	glViewport(0, 0, width, height);
}


const char *clGetErrorString(cl_int);
bool _clCheckError(cl_int err, const char *file, unsigned int line, const std::string& msg, bool fatal)
{
	if (err) {
		std::cerr << "OpenCL error | " << file << "(" << line << ") : (" << err << ")'" 
				<< clGetErrorString(err) << "'." << std::endl;
		if (fatal)
			throw std::runtime_error(msg);
		else
			std::cerr << "  Error message: '" << msg << "'." << std::endl;
		return true;
	}
	return false;
}

void initialize_gl()
{
	glfwSetErrorCallback(_glfw_error_callback);

	if (!glfwInit())
		throw std::runtime_error("GLFW initialization failed");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
	glfwWindowHint(GLFW_REFRESH_RATE, 60);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	g_windowPtr = glfwCreateWindow(1000, 1000, "P50K", nullptr, nullptr);
	if (!g_windowPtr)
		throw std::runtime_error("Could not create GLFW window");

	glfwMakeContextCurrent(g_windowPtr);
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(g_windowPtr, _glfw_resize_callback);

	GLenum glewerror = GLEW_OK;
	if ((glewerror = glewInit()) != GLEW_OK) {
		throw std::runtime_error(std::string("GLEW initialization error: \"") +
				reinterpret_cast<const char*>(glewGetErrorString(glewerror)) + "\"");
	}

	glViewport(0, 0, 1000, 1000);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	g_camera = new Camera(-2.5f, 2.5f, 2.5f, -2.5f);

	const GLubyte *renderer = glGetString(GL_RENDERER);
	std::cout << "Initialized Graphics Device (" << reinterpret_cast<const char*>(renderer) << ")" << std::endl;
}

void initialize_cl()
{
	cl_int clerr;
	char clname[1024];
	char cldname[1024];
	int clmaxmflops = -1;
	cl_platform_id clfastplatid = 0;
	cl_device_id clfastdevid = 0;

	// Get a list of all available OpenCL platforms
	cl_uint numplat;
	cl_platform_id clplatforms[32];
	CL_CHECK_FATAL(clGetPlatformIDs(32, clplatforms, &numplat), "Could not get number of OpenCL platforms");
	if (numplat < 1)
		throw std::runtime_error("No available OpenCL platforms");

	// Loop over all devices from each platform, and get the ID of the fastest one
	for (unsigned int pindex = 0; pindex < numplat; ++pindex) {
		const cl_platform_id currplat = clplatforms[pindex];

		// Get the name of the OpenCL platform
		if (CL_CHECK(clGetPlatformInfo(currplat, CL_PLATFORM_NAME, 1024, clname, nullptr),
				"Could not retrieve name of OpenCL platform, this platform will be ignored")) {
			continue;
		}

		// Retrieve a list of devices on the platform
		cl_uint numdev;
		cl_device_id cldevices[32];
		if (CL_CHECK(clGetDeviceIDs(currplat, CL_DEVICE_TYPE_GPU, 32, cldevices, &numdev),
				"Could not get number of OpenCL devices on platform '%s', ignoring platform", clname)) {
			continue;
		}
		if (numdev < 1) {
			std::cerr << "Platform '" << clname << "' does not have any OpenCL devices, ignoring platform";
			continue;
		}

		// Loop over each device in the platform to find the fastest one
		for (unsigned int dindex = 0; dindex < numdev; ++dindex) {
			const cl_device_id currdev = cldevices[dindex];

			// Extract the device name, compute unit count, and processor speed
			cl_uint dunits, dfreq;
			if (CL_CHECK(clGetDeviceInfo(currdev, CL_DEVICE_NAME, 1024, cldname, nullptr), 
					"Could not get device name on platform '%s', ignoring device", clname) || 
				CL_CHECK(clGetDeviceInfo(currdev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(dunits), &dunits, nullptr),
					"Could not get number of number of compute units for device '%s', ignoring device", cldname) ||
				CL_CHECK(clGetDeviceInfo(currdev, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(dfreq), &dfreq, nullptr),
					"Could not get processor frequency for device '%s', ignoring device", cldname)) {
				continue;
			}

			// Ignore the embedded Intel graphics devices
			std::string devname(cldname);
			if (devname.find("Intel") != std::string::npos) {
				std::cout << "Ignoring device '" << cldname << "', as it is an integrated device." << std::endl;
				continue;
			}

			// Calculate total device speed, and check if it is the fastest available
			int mflops = dunits * dfreq;
			if (mflops > clmaxmflops) {
				clfastplatid = currplat;
				clfastdevid = currdev;
				clmaxmflops = mflops;
			}
		}
	}

	// Get the max work group size
	CL_CHECK_FATAL(clGetDeviceInfo(clfastdevid, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &OPENCL_MAX_WORK_GROUP_SIZE, nullptr),
		"Could not retreive work group size for selected fastest device '%s'", cldname);

	// Report the selected fastest device
	CL_CHECK_FATAL(clGetDeviceInfo(clfastdevid, CL_DEVICE_NAME, 1024, cldname, nullptr),
		"Could not get name of selected fastest device");
	std::cout << "Initialized Compute Device ('" << cldname << "', Mflops: " << clmaxmflops << ", MWGS: " 
		<< OPENCL_MAX_WORK_GROUP_SIZE << ")" << std::endl;
	g_clDevice = clfastdevid;

	// Create the context properties, including memory sharing with OpenGL
	HGLRC wglContext = wglGetCurrentContext();
	if (!wglContext)
		throw std::runtime_error("Could not retreive the current wgl context");
	HDC wglDCContext = wglGetCurrentDC();
	if (!wglDCContext)
		throw std::runtime_error("Could not retreive the current wgl device");
	cl_context_properties clprops[7] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties) clfastplatid,
		CL_GL_CONTEXT_KHR, (cl_context_properties) wglContext,
		CL_WGL_HDC_KHR, (cl_context_properties) wglDCContext,
		0
	};

	// Temporary OpenCL error callback for the next few steps
	const auto clerrcallback = [](const char *errinfo, const void *, size_t, void *) -> void {
		std::cerr << "OpenCL Error: '" << errinfo << "'." << std::endl;
	};

	// Create the context and command queue on the fastest device
	clerr = CL_NONE;
	g_clContext = clCreateContext(clprops, 1, &clfastdevid, /*clerrcallback*/ nullptr, nullptr, &clerr);
	if (!g_clContext || clerr)
		throw std::runtime_error(std::string("Failed to create OpenCL context on selected device (") + clGetErrorString(clerr) + ")");
	g_clCommandQueue = clCreateCommandQueue(g_clContext, clfastdevid, NULL, &clerr);
	if (!g_clCommandQueue || clerr)
		throw std::runtime_error(std::string("Failed to create OpenCL command queue on selected device (") + clGetErrorString(clerr) + ")");

	// Report success
	std::cout << "Initialized OpenCL Context" << std::endl;
}

size_t getMaxWorkGroupSize()
{
	return OPENCL_MAX_WORK_GROUP_SIZE;
}

void shutdown_gl()
{
	if (g_camera)
		delete g_camera;

	if (g_windowPtr)
		glfwDestroyWindow(g_windowPtr);

	glfwTerminate();
}

void shutdown_cl()
{

}




const char* clGetErrorString(cl_int error)
{
	static const char* strings[] =
	{
		// Error Codes
		"CL_SUCCESS"                                  //   0
		, "CL_DEVICE_NOT_FOUND"                         //  -1
		, "CL_DEVICE_NOT_AVAILABLE"                     //  -2
		, "CL_COMPILER_NOT_AVAILABLE"                   //  -3
		, "CL_MEM_OBJECT_ALLOCATION_FAILURE"            //  -4
		, "CL_OUT_OF_RESOURCES"                         //  -5
		, "CL_OUT_OF_HOST_MEMORY"                       //  -6
		, "CL_PROFILING_INFO_NOT_AVAILABLE"             //  -7
		, "CL_MEM_COPY_OVERLAP"                         //  -8
		, "CL_IMAGE_FORMAT_MISMATCH"                    //  -9
		, "CL_IMAGE_FORMAT_NOT_SUPPORTED"               //  -10
		, "CL_BUILD_PROGRAM_FAILURE"                    //  -11
		, "CL_MAP_FAILURE"                              //  -12

		, ""    //  -13
		, ""    //  -14
		, ""    //  -15
		, ""    //  -16
		, ""    //  -17
		, ""    //  -18
		, ""    //  -19

		, ""    //  -20
		, ""    //  -21
		, ""    //  -22
		, ""    //  -23
		, ""    //  -24
		, ""    //  -25
		, ""    //  -26
		, ""    //  -27
		, ""    //  -28
		, ""    //  -29

		, "CL_INVALID_VALUE"                            //  -30
		, "CL_INVALID_DEVICE_TYPE"                      //  -31
		, "CL_INVALID_PLATFORM"                         //  -32
		, "CL_INVALID_DEVICE"                           //  -33
		, "CL_INVALID_CONTEXT"                          //  -34
		, "CL_INVALID_QUEUE_PROPERTIES"                 //  -35
		, "CL_INVALID_COMMAND_QUEUE"                    //  -36
		, "CL_INVALID_HOST_PTR"                         //  -37
		, "CL_INVALID_MEM_OBJECT"                       //  -38
		, "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"          //  -39
		, "CL_INVALID_IMAGE_SIZE"                       //  -40
		, "CL_INVALID_SAMPLER"                          //  -41
		, "CL_INVALID_BINARY"                           //  -42
		, "CL_INVALID_BUILD_OPTIONS"                    //  -43
		, "CL_INVALID_PROGRAM"                          //  -44
		, "CL_INVALID_PROGRAM_EXECUTABLE"               //  -45
		, "CL_INVALID_KERNEL_NAME"                      //  -46
		, "CL_INVALID_KERNEL_DEFINITION"                //  -47
		, "CL_INVALID_KERNEL"                           //  -48
		, "CL_INVALID_ARG_INDEX"                        //  -49
		, "CL_INVALID_ARG_VALUE"                        //  -50
		, "CL_INVALID_ARG_SIZE"                         //  -51
		, "CL_INVALID_KERNEL_ARGS"                      //  -52
		, "CL_INVALID_WORK_DIMENSION"                   //  -53
		, "CL_INVALID_WORK_GROUP_SIZE"                  //  -54
		, "CL_INVALID_WORK_ITEM_SIZE"                   //  -55
		, "CL_INVALID_GLOBAL_OFFSET"                    //  -56
		, "CL_INVALID_EVENT_WAIT_LIST"                  //  -57
		, "CL_INVALID_EVENT"                            //  -58
		, "CL_INVALID_OPERATION"                        //  -59
		, "CL_INVALID_GL_OBJECT"                        //  -60
		, "CL_INVALID_BUFFER_SIZE"                      //  -61
		, "CL_INVALID_MIP_LEVEL"                        //  -62
		, "CL_INVALID_GLOBAL_WORK_SIZE"                 //  -63
		, "CL_UNKNOWN_ERROR_CODE"
	};

	if (error >= -63 && error <= 0)
		return strings[-error];
	else if (error <= -1000)
		return "CL_EXTENSION_ERROR_CODE";
	else
		return strings[64];
}