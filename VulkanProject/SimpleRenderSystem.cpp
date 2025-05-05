#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

struct PushConstantData {
	glm::mat2 transform{ 1.0f };
	glm::vec2 offset;
	alignas(16) glm::vec3 colour;		// Device (GPU) memory as 16 byte aligned for vec3, whereas in host (CPU) this isn't the default
};

SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass)
	: m_Device(device)
{
	m_CreatePipelineLayout();
	m_CreatePipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem()
{
	vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);

} 

void SimpleRenderSystem::m_CreatePipelineLayout()
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

void SimpleRenderSystem::m_CreatePipeline(VkRenderPass& renderPass)
{
	assert(m_PipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_Pipeline = std::make_unique<Pipeline>(
		m_Device,
		pipelineConfig,
		"C:\\Users\\joebi\\Documents\\Projects\\VulkanProject\\shaders\\simple_shader.vert.spv",
		"C:\\Users\\joebi\\Documents\\Projects\\VulkanProject\\shaders\\simple_shader.frag.spv"
	);
}

void SimpleRenderSystem::renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects)
{
	m_Pipeline->bind(commandBuffer);

	for (auto& object : objects) {
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