#pragma once

#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Model.h"

#include <memory>
#include <vector>
#include <cassert>

class Renderer
{
public:
	Renderer(Window& window, Device& device);
	~Renderer();

	// Not copyable or movable
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	VkCommandBuffer beginFrame();
	void endFrame();
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
	bool isFrameInProgress() const { return m_IsFrameStarted; };
	VkRenderPass getSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); };
	VkCommandBuffer getCurrentCommandBuffer() const { 
		assert(m_IsFrameStarted && "Cannot get current commandbuffer when frame not in progress");
		return m_CommandBuffers[m_CurrentImageIndex]; 
	};

private:
	Window& m_Window;
	Device& m_Device;
	std::unique_ptr <SwapChain> m_SwapChain;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	uint32_t m_CurrentImageIndex{0};
	bool m_IsFrameStarted{ false };

	void m_CreateCommandBuffers();
	void m_FreeCommandBuffers();
	void m_RecreateSwapChain();
};


