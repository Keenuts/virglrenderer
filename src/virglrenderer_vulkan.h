#ifndef VIRGLRENDERER_VULKAN_H
#define VIRGLRENDERER_VULKAN_H

#include "virglrenderer.h"
#include <vulkan/vulkan.h>

VIRGL_EXPORT VkResult
virgl_vk_create_instance(
    const VkInstanceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkInstance *pInstance
);

VIRGL_EXPORT VkResult
virgl_vk_enumerate_instance_layer_properties(
    uint32_t *pPropertyCount,
    VkLayerProperties *pProperties
);

VIRGL_EXPORT VkResult
virgl_vk_enumerate_physical_devices(
    VkInstance instance,
    uint32_t* pPhysicalDeviceCount,
    VkPhysicalDevice* pPhysicalDevices
);

VIRGL_EXPORT void
virgl_vk_get_physical_device_queue_family_properties(
    VkPhysicalDevice physicalDevice,
    uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties* pQueueFamilyProperties
);

VIRGL_EXPORT VkResult
virgl_vk_create_device(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDevice *pDevice
);

VIRGL_EXPORT void
virgl_vk_get_device_queue(
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    VkQueue *pQueue
);

VIRGL_EXPORT VkResult
virgl_vk_create_descriptor_set_layout(
    VkDevice device,
    const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDescriptorSetLayout *pSetLayout
);

VIRGL_EXPORT VkResult
virgl_vk_create_descriptor_pool(
    VkDevice device,
    const VkDescriptorPoolCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDescriptorPool *pDescriptorPool
);

VIRGL_EXPORT VkResult
virgl_vk_create_command_pool(
    VkDevice device,
    const VkCommandPoolCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkCommandPool *pCommandPool
);

VIRGL_EXPORT VkResult
virgl_vk_allocate_descriptor_sets(
    VkDevice device,
    const VkDescriptorSetAllocateInfo *pAllocateInfo,
    VkDescriptorSet *pDescriptorSets
);

VIRGL_EXPORT void
virgl_vk_update_descriptor_sets(
    VkDevice device,
    uint32_t descriptorWriteCount,
    const VkWriteDescriptorSet *pDescriptorWrites,
    uint32_t descriptorCopyCount,
    const VkCopyDescriptorSet *pDescriptorCopies
);

VIRGL_EXPORT VkResult virgl_vk_allocate_command_buffers(
    VkDevice device,
    const VkCommandBufferAllocateInfo *pAllocateInfo,
    VkCommandBuffer *pCommandBuffers);

VIRGL_EXPORT VkResult virgl_vk_allocate_memory(
    VkDevice device,
    const VkMemoryAllocateInfo *pAllocateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDeviceMemory *pMemory);

VIRGL_EXPORT VkResult virgl_vk_begin_command_buffer(
    VkCommandBuffer commandBuffer,
    const VkCommandBufferBeginInfo *pBeginInfo);

VIRGL_EXPORT VkResult virgl_vk_bind_buffer_memory(
    VkDevice device,
    VkBuffer buffer,
    VkDeviceMemory memory,
    VkDeviceSize memoryOffset);

VIRGL_EXPORT void virgl_vk_bind_descriptor_sets(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    uint32_t descriptorSetCount,
    const VkDescriptorSet *pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t *pDynamicOffsets);

VIRGL_EXPORT void virgl_vk_cmd_bind_pipeline(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline);

VIRGL_EXPORT void virgl_vk_cmd_dispatch(
    VkCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ);

VIRGL_EXPORT VkResult virgl_vk_create_buffer(
    VkDevice device,
    const VkBufferCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkBuffer *pBuffer);

VIRGL_EXPORT VkResult virgl_vk_create_compute_pipeline(
    VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkComputePipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator,
    VkPipeline *pPipelines);

VIRGL_EXPORT VkResult virgl_vk_create_fence(
    VkDevice device,
    const VkFenceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkFence *pFence);

VIRGL_EXPORT VkResult virgl_vk_create_pipeline_layout(
    VkDevice device,
    const VkPipelineLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkPipelineLayout *pPipelineLayout);

VIRGL_EXPORT VkResult virgl_vk_create_shader_module(
    VkDevice device,
    const VkShaderModuleCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkShaderModule *pShaderModule);

VIRGL_EXPORT void virgl_vk_destroy_buffer(
    VkDevice device,
    VkBuffer buffer,
    const VkAllocationCallbacks *pAllocator);

VIRGL_EXPORT VkResult virgl_vk_end_command_buffer(
    VkCommandBuffer commandBuffer);

VIRGL_EXPORT void virgl_vk_free_memory(
     VkDevice device,
     VkDeviceMemory memory,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_get_physical_device_memory_properties(
     VkPhysicalDevice physicalDevice,
     VkPhysicalDeviceMemoryProperties* pMemoryProperties);

VIRGL_EXPORT VkResult virgl_vk_queue_submit(
     VkQueue queue,
     uint32_t submitCount,
     const VkSubmitInfo* pSubmits,
     VkFence fence);

VIRGL_EXPORT void virgl_vk_unmap_memory(
     VkDevice device,
     VkDeviceMemory memory);

VIRGL_EXPORT VkResult virgl_vk_wait_for_fences(
     VkDevice device,
     uint32_t fenceCount,
     const VkFence* pFences,
     VkBool32 waitAll,
     uint64_t timeout);

VIRGL_EXPORT void virgl_vk_destroy_fences(
     VkDevice device,
     VkFence fence,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_shader_module(
     VkDevice device,
     VkShaderModule shaderModule,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_descriptor_pool(
     VkDevice device,
     VkDescriptorPool descriptorPool,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_descriptor_set_layout(
     VkDevice device,
     VkDescriptorSetLayout descriptorSetLayout,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_pipeline_layout(
     VkDevice device,
     VkPipelineLayout pipelineLayout,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_pipeline(
     VkDevice device,
     VkPipeline pipeline,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_command_pool(
     VkDevice device,
     VkCommandPool commandPool,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_device(
    VkDevice device,
    const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT void virgl_vk_destroy_instance(
     VkInstance instance,
     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT VkResult virgl_vk_map_memory(
     VkDevice device,
     VkDeviceMemory memory,
     VkDeviceSize offset,
     VkDeviceSize size,
     VkMemoryMapFlags flags,
     void** ppData);
#endif
