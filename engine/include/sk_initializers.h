#pragma once
#include <vector>
#include <iostream>
#include "vulkan_init.h"

namespace SketchBook
{
namespace VkInit
{
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkSubmitInfo submit_info(VkCommandBuffer* cmd);

	VkSemaphoreCreateInfo create_semaphore_create_info();

	// pipeline initializers
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode);
	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();
	VkPipelineLayoutCreateInfo pipeline_layout_create_info();
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
}
}

namespace SketchBook
{
namespace Core
{
	class PipelineBuilder {
	public:

		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
		VkViewport _viewport;
		VkRect2D _scissor;
		VkPipelineRasterizationStateCreateInfo _rasterizer;
		VkPipelineColorBlendAttachmentState _colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo _multisampling;
		VkPipelineLayout _pipelineLayout;
		VkPipelineDepthStencilStateCreateInfo _depthStencil;

		VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
	};

	struct UploadContext {
		VkFence _uploadFence;
		VkCommandPool _commandPool;	
		VkCommandBuffer _commandBuffer;
	};
}
}
