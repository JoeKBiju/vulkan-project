#pragma once

#include "Window.h"
#include "Pipeline.h"
#include "Device.h"
#include "SwapChain.h"
#include "Model.h"
#include "Object.h"

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
	std::unique_ptr <SwapChain> m_SwapChain;
	std::unique_ptr<Pipeline> m_Pipeline;
	VkPipelineLayout m_PipelineLayout;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	std::vector<Object> m_Objects;

	void m_LoadObjects();
	void m_CreatePipelineLayout();
	void m_CreatePipeline();
	void m_CreateCommandBuffers();
	void m_FreeCommandBuffers();
	void m_DrawFrame();
	void m_RecreateSwapChain();
	void recordCommandBuffer(int imageIndex);
	void m_RenderGameObjects(VkCommandBuffer commandBuffer);

	void m_Sierpinski(std::vector<Model::Vertex>& vertices, int depth, std::pair<glm::vec2, glm::vec3> left, std::pair<glm::vec2, glm::vec3> right, std::pair<glm::vec2, glm::vec3> top);
};

