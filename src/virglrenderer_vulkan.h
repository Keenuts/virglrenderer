#ifndef VIRGLRENDERER_VULKAN_H
#define VIRGLRENDERER_VULKAN_H

#include "virglrenderer.h"
#include <vulkan/vulkan.h>

VIRGL_EXPORT int
virgl_vk_get_device_count(uint32_t *device_count);

VIRGL_EXPORT int
virgl_vk_get_sparse_properties(uint32_t device_id,
                               VkPhysicalDeviceSparseProperties *sparse_props);
VIRGL_EXPORT int
virgl_vk_get_queue_family_properties(uint32_t device_id,
                                     uint32_t *family_count,
                                     VkQueueFamilyProperties **props);

VIRGL_EXPORT int
virgl_vk_create_device(uint32_t phys_device_id,
                       const VkDeviceCreateInfo *info,
                       uint32_t *device_id);

VIRGL_EXPORT int
virgl_vk_create_descriptor_set_layout(uint32_t device_id,
												  const VkDescriptorSetLayoutCreateInfo *info,
												  uint32_t *handle);

VIRGL_EXPORT int
virgl_vk_create_buffer(uint32_t device_id,
							  const VkBufferCreateInfo *info,
							  uint32_t *handle);
#endif
