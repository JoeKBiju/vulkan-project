#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

struct PushConstantData {
	glm::mat2 transform{1.0f};
	glm::vec2 offset;
	alignas(16) glm::vec3 colour;		// Device (GPU) memory as 16 byte aligned for vec3, whereas in host (CPU) this isn't the default
};

Application::Application()
{
	m_LoadObjects();
	m_CreatePipelineLayout();
	m_CreatePipeline();
}

Application::~Application()
{
	vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);

}

void Application::run()
{
	while (!m_Window.shouldClose())
	{
		glfwPollEvents();

		if (VkCommandBuffer commandBuffer = m_Renderer.beginFrame()) {
			m_Renderer.beginSwapChainRenderPass(commandBuffer);
			m_RenderGameObjects(commandBuffer);
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

void Application::m_CreatePipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void Application::m_CreatePipeline()
{
	assert(m_PipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = m_Renderer.getSwapChainRenderPass();
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_Pipeline = std::make_unique<Pipeline>(
		m_Device,
		pipelineConfig,
		"C:\\Users\\joebi\\Documents\\Projects\\VulkanProject\\shaders\\simple_shader.vert.spv",
		"C:\\Users\\joebi\\Documents\\Projects\\VulkanProject\\shaders\\simple_shader.frag.spv"
	);
}

void Application::m_RenderGameObjects(VkCommandBuffer commandBuffer)
{
	m_Pipeline->bind(commandBuffer);

	for (auto& object : m_Objects) {
		object.transfrom2D.rotation = glm::mod(object.transfrom2D.rotation + 0.01f, glm::two_pi<float>());

		PushConstantData push{};
		push.offset = object.transfrom2D.translation;
		push.colour = object.colour;
		push.transform = object.transfrom2D.mat2();

		vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
		object.model->bind(commandBuffer);
		object.model->draw(commandBuffer);
	}
}

void Application::m_Sierpinski(std::vector<Model::Vertex>& vertices, int depth, std::pair<glm::vec2, glm::vec3> left, std::pair<glm::vec2, glm::vec3> right, std::pair<glm::vec2, glm::vec3> top) {
	if (depth <= 0) {
		vertices.push_back({ { top.first }, { top.second } });
		vertices.push_back({ { right.first }, { right.second } });
		vertices.push_back({ { left.first }, { left.second } });
	}
	else {
		auto leftTop = 0.5f * (left.first + top.first);
		auto rightTop = 0.5f * (right.first + top.first);
		auto leftRight = 0.5f * (left.first + right.first);
		
		auto leftTopColour = 0.5f * (left.second + top.second);
		auto rightTopColour = 0.5f * (right.second + top.second);
		auto leftRightColour = 0.5f * (left.second + right.second);

		m_Sierpinski(vertices, depth - 1, left, std::make_pair(leftRight, leftRightColour), std::make_pair(leftTop, leftTopColour));
		m_Sierpinski(vertices, depth - 1, std::make_pair(leftRight, leftRightColour), right, std::make_pair(rightTop, rightTopColour));
		m_Sierpinski(vertices, depth - 1, std::make_pair(leftTop, leftTopColour), std::make_pair(rightTop, rightTopColour), top);
	}
}
