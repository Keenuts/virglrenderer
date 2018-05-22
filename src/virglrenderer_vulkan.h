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

#endif
