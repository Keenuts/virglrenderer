#ifndef VIRGLRENDERER_VULKAN_H
#define VIRGLRENDERER_VULKAN_H

#include <vulkan/vulkan.h>

VIRGL_EXPORT VkResult
virgl_vk_create_instance(const VkInstanceCreateInfo *pCreateInfo,
                         const VkAllocationCallbacks *pAllocator,
                         VkInstance *pInstance);

VIRGL_EXPORT VkResult
virgl_vk_enumerate_instance_layer_properties(uint32_t *pPropertyCount,
                                             VkLayerProperties *pProperties);

#if 0
VIRGL_EXPORT VkResult
virgl_vk_allocate_descriptor_sets(VkDevice device,
                                  const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                  VkDescriptorSet* pDescriptorSets);

VIRGL_EXPORT VkResult
virgl_vk_allocate_memory(VkDevice device,
                         const VkMemoryAllocateInfo* pAllocateInfo,
                         const VkAllocationCallbacks* pAllocator,
                         VkDeviceMemory* pMemory);


VIRGL_EXPORT VkResult
virgl_vk_create_buffer(VkDevice device,
                       const VkBufferCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator,
                       VkBuffer* pBuffer);

VIRGL_EXPORT VkResult
virgl_vk_bind_buffer_memory(VkDevice device,
                            VkBuffer buffer,
                            VkDeviceMemory memory,
                            VkDeviceSize memoryOffset);

VIRGL_EXPORT VkResult
virgl_vk_unmap_memory(VkDevice device,
                      VkDeviceMemory memory);

VIRGL_EXPORT VkResult
virgl_vk_free_memory(VkDevice device,
                     VkDeviceMemory memory,
                     const VkAllocationCallbacks* pAllocator);

VIRGL_EXPORT VkResult vkDestroyBuffer();
VIRGL_EXPORT VkResult vkAllocateCommandBuffers();
VIRGL_EXPORT VkResult vkBeginCommandBuffer();
VIRGL_EXPORT VkResult vkBindBufferMemory();
VIRGL_EXPORT VkResult vkCmdBindDescriptorSets();
VIRGL_EXPORT VkResult vkCmdBindPipeline();
VIRGL_EXPORT VkResult vkCmdDispatch();
VIRGL_EXPORT VkResult vkCreateComputePipelines();
VIRGL_EXPORT VkResult vkCreateFence();
VIRGL_EXPORT VkResult vkCreatePipelineLayout();
VIRGL_EXPORT VkResult vkCreateShaderModule();
VIRGL_EXPORT VkResult vkDestroyBuffer();
VIRGL_EXPORT VkResult vkDestroyCommandPool();
VIRGL_EXPORT VkResult vkDestroyDescriptorPool();
VIRGL_EXPORT VkResult vkDestroyDescriptorSetLayout();
VIRGL_EXPORT VkResult vkDestroyDevice();
VIRGL_EXPORT VkResult vkDestroyFence();
VIRGL_EXPORT VkResult vkDestroyInstance();
VIRGL_EXPORT VkResult vkDestroyPipeline();
VIRGL_EXPORT VkResult vkDestroyPipelineLayout();
VIRGL_EXPORT VkResult vkDestroyShaderModule();
VIRGL_EXPORT VkResult vkEndCommandBuffer();
VIRGL_EXPORT VkResult vkFreeMemory();
VIRGL_EXPORT VkResult vkMapMemory();
VIRGL_EXPORT VkResult vkQueueSubmit();
VIRGL_EXPORT VkResult vkUnmapMemory();
VIRGL_EXPORT VkResult vkWaitForFences();
VIRGL_EXPORT VkResult vkCreateCommandPool();
VIRGL_EXPORT VkResult vkCreateDescriptorPool();
VIRGL_EXPORT VkResult vkCreateDescriptorSetLayout();
VIRGL_EXPORT VkResult vkCreateDevice();
VIRGL_EXPORT VkResult vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
VIRGL_EXPORT VkResult vkGetDeviceQueue();
VIRGL_EXPORT VkResult vkGetPhysicalDeviceMemoryProperties();
VIRGL_EXPORT VkResult vkGetPhysicalDeviceQueueFamilyProperties();
VIRGL_EXPORT VkResult vkUpdateDescriptorSets();
#endif

#endif
