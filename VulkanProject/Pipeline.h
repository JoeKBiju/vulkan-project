#pragma once

#include <string>
#include <vector>

#include "Device.h"

struct PipelineConfigInfo {
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline
{
public:
	Pipeline(Device& device,
		const PipelineConfigInfo& configInfo,
		const std::string& vertexFilePath,
		const std::string& fragFilePath);

	~Pipeline();

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	void bind(VkCommandBuffer commandBuffer);

	// Not copyable or movable
	Pipeline(const Pipeline&) = delete;
	Pipeline operator=(const Pipeline&) = delete;

private:
	Device& m_Device;
	VkPipeline m_GraphicsPipeline;
	VkShaderModule m_VertexShaderModule;
	VkShaderModule m_FragmentShaderModule;

	std::vector<char> m_readFile(const std::string& filePath);
	void m_createGraphicsPipeline(const std::string& vertexFilePath,
		const std::string& fragFilePath,
		const PipelineConfigInfo& configInfo);

	void m_createShaderModules(const std::vector<char>& code, VkShaderModule& shaderModule);
};

