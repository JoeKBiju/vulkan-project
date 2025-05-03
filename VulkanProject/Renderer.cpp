#include "Renderer.h"

#include <stdexcept>
#include <array>

Renderer::Renderer(Window& window, Device& device)
	: m_Window(window), m_Device(device)
{
	m_RecreateSwapChain();
	m_CreateCommandBuffers();
}

Renderer::~Renderer()
{
	m_FreeCommandBuffers();
}

VkCommandBuffer Renderer::beginFrame()
{
	assert(!m_IsFrameStarted && "Cannot call beginFrame function when a frame has already been started");

	VkResult result = m_SwapChain->acquireNextImage(&m_CurrentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_RecreateSwapChain();
		return nullptr;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	m_IsFrameStarted = true;

	VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	return commandBuffer;
}

void Renderer::endFrame()
{
	assert(m_IsFrameStarted && "Cannot call endFrame function when a frame is not in progress");

	VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	VkResult result = m_SwapChain->submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
		m_Window.wasWindowResized()) {
		m_Window.resetWindowResizedFlag();
		m_RecreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_IsFrameStarted = false;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(m_IsFrameStarted && "Cannot call beginSwapChainRenderPass function when a frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin a render pass using a command buffer from another frame");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_SwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(m_CurrentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, m_SwapChain->getSwapChainExtent() };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(m_IsFrameStarted && "Cannot call endSwapChainRenderPass function when a frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end a render pass using a command buffer from another frame");

	vkCmdEndRenderPass(commandBuffer);
}

void Renderer::m_CreateCommandBuffers()
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

void Renderer::m_FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		m_Device.device(),
		m_Device.getCommandPool(),
		static_cast<uint32_t>(m_CommandBuffers.size()),
		m_CommandBuffers.data());
	m_CommandBuffers.clear();
}

void Renderer::m_RecreateSwapChain()
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
	// m_CreatePipeline();
}
