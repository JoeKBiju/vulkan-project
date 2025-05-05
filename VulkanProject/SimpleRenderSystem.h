#pragma once

#include "Pipeline.h"
#include "Device.h"
#include "Model.h"
#include "Object.h"

#include <memory>
#include <vector>
#include <utility>

class SimpleRenderSystem
{
public:
	SimpleRenderSystem(Device& device, VkRenderPass renderPass);
	~SimpleRenderSystem();

	// Not copyable or movable
	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

	void renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects);

private: 

	Device& m_Device;
	std::unique_ptr<Pipeline> m_Pipeline;
	VkPipelineLayout m_PipelineLayout;

	void m_CreatePipelineLayout();
	void m_CreatePipeline(VkRenderPass& renderPass);
};

