#pragma once

#include "Window.h"
#include "Device.h"
#include "Model.h"
#include "Object.h"
#include "Renderer.h"

#include <memory>
#include <vector>
#include <utility>

class Application
{
public:
	Application();
	~Application();

	void run();

	// Not copyable or movable
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

private:
	int WIDTH = 1920;
	int HEIGHT = 1080;
	static constexpr const char* NAME = "Vulkan Application";

	Window m_Window{ WIDTH, HEIGHT, NAME };
	Device m_Device{ m_Window };
	Renderer m_Renderer{ m_Window, m_Device };
	std::vector<Object> m_Objects;

	void m_LoadObjects();
};

