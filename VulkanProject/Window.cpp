#include "Window.h"

#include <iostream>
#include <stdexcept>

Window::Window(int width, int height, std::string name) : m_Width(width), m_Height(height), m_Name(name)
{
	m_CreateWindow();
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface");
	}
}

void Window::m_CreateWindow()
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
	}

	if (!glfwVulkanSupported())
	{
		throw std::runtime_error("Vulkan not supported");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto windowPointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	windowPointer->m_FramebufferResized = true;
	windowPointer->m_Width = width;
	windowPointer->m_Height = height;
}
