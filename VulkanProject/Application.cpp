#include "Application.h"

#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

Application::Application()
{
	m_LoadObjects();
}

Application::~Application(){}

void Application::run()
{
	SimpleRenderSystem simpleRenderSystem{ m_Device, m_Renderer.getSwapChainRenderPass() };

	while (!m_Window.shouldClose())
	{
		glfwPollEvents();

		if (VkCommandBuffer commandBuffer = m_Renderer.beginFrame()) {
			m_Renderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderObjects(commandBuffer, m_Objects);
			m_Renderer.endSwapChainRenderPass(commandBuffer);
			m_Renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(m_Device.device());
}

void Application::m_LoadObjects()
{
	std::vector<Model::Vertex> vertices{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }},
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }}
	};
	std::shared_ptr<Model> model = std::make_shared<Model>(m_Device, vertices);

	Object triangle = Object::createObject();
	triangle.model = model;
	triangle.colour = { 0.1f, 0.8f, 0.1f };
	triangle.transfrom2D.translation.x = 0.2f;
	triangle.transfrom2D.scale = { 2.0f, 0.5f };
	triangle.transfrom2D.rotation = 0.25f * glm::two_pi<float>();   // 2pi = 360 degree rotation -> 0.25f * 360 = 90 degrees

	Object triangle2 = Object::createObject();
	triangle2.model = model;
	triangle2.colour = { 0.8f, 0.1f, 0.1f };
	triangle2.transfrom2D.translation.x = -0.2f;
	triangle2.transfrom2D.scale = { 1.1f, 0.5f };
	triangle2.transfrom2D.rotation = 0.5f * glm::two_pi<float>();

	Object triangle3 = Object::createObject();
	triangle3.model = model;
	triangle3.colour = { 0.1f, 0.1f, 0.8f };
	triangle3.transfrom2D.translation.x = -0.7f;
	triangle3.transfrom2D.scale = { 0.5f, 0.5f };
	triangle3.transfrom2D.rotation = 0.75f * glm::two_pi<float>();
	
	m_Objects.push_back(std::move(triangle));
	m_Objects.push_back(std::move(triangle2));
	m_Objects.push_back(std::move(triangle3));
}
