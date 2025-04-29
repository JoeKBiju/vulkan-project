#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class Window
{
public:
	Window(int width, int height, std::string name);
	~Window();

	// To prevent copying of the window and creating dangling pointers
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	const inline bool shouldClose() { return glfwWindowShouldClose(m_Window); };
	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	bool wasWindowResized() { return m_FramebufferResized; };
	void resetWindowResizedFlag() { m_FramebufferResized = false; }
	VkExtent2D getExtent() { return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; };

private:
	GLFWwindow* m_Window;
	int m_Width;
	int m_Height;
	bool m_FramebufferResized = false;
	std::string m_Name;

	void m_CreateWindow();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

