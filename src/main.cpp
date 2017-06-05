#include <iostream>

#include "gpu.hpp"
#include "sim.hpp"


void mainloop();

Simulation *TheSimulation = nullptr;

int main(int argc, char **argv)
{
	try {
		initialize_gl();
		initialize_cl();
	}
	catch (std::exception& ex) {
		std::cerr << "Startup Error: \"" << ex.what() << "\"." << std::endl;
		return -1;
	}

	try {
		mainloop();
	}
	catch (std::exception& ex) {
		std::cerr << "Runtime Error: \"" << ex.what() << "\"." << std::endl;
		std::cerr << "Attempting shutdown..." << std::endl;
	}

	try {
		shutdown_cl();
		shutdown_gl();
	}
	catch (std::exception& ex) {
		std::cerr << "Shutdown Error: \"" << ex.what() << "\"." << std::endl;
		return -1;
	}

	return 0;
}

void mainloop()
{
	TheSimulation = new Simulation;

	glPointSize(2);

	float lastTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(g_windowPtr)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float thisTime = (float)glfwGetTime();

		TheSimulation->render(thisTime - lastTime);
		lastTime = thisTime;

		glfwSwapBuffers(g_windowPtr);

		glfwPollEvents();
	}

	delete TheSimulation;
}