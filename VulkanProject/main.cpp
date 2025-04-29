#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "Application.h"

int main() {
	std::cout << "Vulkan Application" << std::endl;

	try {
		Application app;
		app.run();
	}
	catch (const std::string& error)
	{
		std::cout << "An error occurred: " << error << std::endl;
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		std::cout << "An error occurred: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

    return EXIT_SUCCESS;
}