#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

struct PushConstantData {
	glm::vec2 offset;
	alignas(16) glm::vec3 colour;		// Device (GPU) memory as 16 byte aligned for vec3, whereas in host (CPU) this isn't the default
};

Application::Application()
{
	m_LoadModel();
	m_CreatePipelineLayout();
	m_RecreateSwapChain();
	m_CreateCommandBuffers();
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
		m_DrawFrame();
	}

	vkDeviceWaitIdle(m_Device.device());
}

void Application::m_LoadModel()
{
	std::vector<Model::Vertex> vertices{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }},
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }}
	};
	m_Model = std::make_unique<Model>(m_Device, vertices);
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
	assert(m_SwapChain != nullptr);
	assert(m_PipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = m_SwapChain->getRenderPass();
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_Pipeline = std::make_unique<Pipeline>(
		m_Device,
		pipelineConfig,
		"C:\\Users\\joebi\\Documents\\Projects\\VulkanProject\\shaders\\simple_shader.vert.spv",
		"C:\\Users\\joebi\\Documents\\Projects\\VulkanProject\\shaders\\simple_shader.frag.spv"
	);
}

void Application::m_CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_SwapChain->imageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_Device.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_Device.device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Application::m_FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		m_Device.device(),
		m_Device.getCommandPool(),
		static_cast<uint32_t>(m_CommandBuffers.size()),
		m_CommandBuffers.data());
	m_CommandBuffers.clear();
}

void Application::m_DrawFrame()
{
	uint32_t imageIndex;
	VkResult result = m_SwapChain->acquireNextImage(&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_RecreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	recordCommandBuffer(imageIndex);
	result = m_SwapChain->submitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUBOPTIMAL_KHR || m_Window.wasWindowResized()) {
		m_Window.resetWindowResizedFlag();
		m_RecreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
}

void Application::m_RecreateSwapChain()
{
	VkExtent2D extent = m_Window.getExtent();

	while (extent.width == 0 || extent.height == 0) {
		extent = m_Window.getExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Device.device());
	if (m_SwapChain == nullptr) {
		m_SwapChain = std::make_unique<SwapChain>(m_Device, extent);
	}
	else {
		m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, std::move(m_SwapChain));
		if (m_SwapChain->imageCount() != m_CommandBuffers.size()) {
			m_FreeCommandBuffers();
			m_CreateCommandBuffers();
		}
	}

	// TODO: check if new render pass is compatible with previous pipeline
	//			if it is, do not create a new pipleine
	m_CreatePipeline();
}

void Application::recordCommandBuffer(int imageIndex)
{
	static int frame = 0;
	frame = (frame + 1) % 100;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_SwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(imageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, m_SwapChain->getSwapChainExtent() };
	vkCmdSetViewport(m_CommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(m_CommandBuffers[imageIndex], 0, 1, &scissor);

	m_Pipeline->bind(m_CommandBuffers[imageIndex]);
	m_Model->bind(m_CommandBuffers[imageIndex]);

	// Initialising push constants
	for (int i = 0; i < 4; i++) {
		PushConstantData push{};
		push.offset = { -0.5f + frame * 0.02f, -0.4f + i * 0.25f };
		push.colour = { 0.0f, 0.0f, 0.2f + 0.2f * i };

		vkCmdPushConstants(m_CommandBuffers[imageIndex], m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
		m_Model->draw(m_CommandBuffers[imageIndex]);
	}

	vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
	if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
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
