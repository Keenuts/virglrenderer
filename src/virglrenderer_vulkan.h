#ifndef VIRGLRENDERER_VULKAN_H
#define VIRGLRENDERER_VULKAN_H

#include "virglrenderer.h"
#include <vulkan/vulkan.h>

VIRGL_EXPORT int
virgl_vk_get_device_count(uint32_t *device_count);

VIRGL_EXPORT int
virgl_vk_get_sparse_properties(uint32_t device_handle,
                               VkPhysicalDeviceSparseProperties *sparse_props);
VIRGL_EXPORT int
virgl_vk_get_memory_properties(uint32_t device_id, VkPhysicalDeviceMemoryProperties *out);
VIRGL_EXPORT int
virgl_vk_get_queue_family_properties(uint32_t device_handle,
                                     uint32_t *family_count,
                                     VkQueueFamilyProperties **props);

VIRGL_EXPORT int
virgl_vk_create_device(uint32_t phys_device_id,
                       const VkDeviceCreateInfo *info,
                       uint32_t *device_handle);

VIRGL_EXPORT int
virgl_vk_allocate_descriptor_set(uint32_t device_handle,
                                 uint32_t pool_id,
                                 uint32_t descriptor_count,
                                 uint32_t *desc_layout_ids,
                                 uint32_t *handles);

VIRGL_EXPORT int
virgl_vk_create_descriptor_pool(uint32_t device_handle,
                                const VkDescriptorPoolCreateInfo *vk_info,
                                uint32_t *handle);
VIRGL_EXPORT int
virgl_vk_create_descriptor_set_layout(uint32_t device_handle,
												  VkDescriptorSetLayoutCreateInfo *info,
												  uint32_t *handle);
VIRGL_EXPORT int
virgl_vk_create_shader_module(uint32_t device_handle,
                              const VkShaderModuleCreateInfo *info,
                              uint32_t *handle);
VIRGL_EXPORT int
virgl_vk_create_pipeline_layout(uint32_t device_handle,
                                VkPipelineLayoutCreateInfo *info,
                                uint32_t *set_handles,
                                uint32_t *handle);
VIRGL_EXPORT int
virgl_vk_create_compute_pipelines(uint32_t device_handle,
                                  VkComputePipelineCreateInfo *info,
                                  uint32_t layout_handle,
                                  uint32_t module_handle,
                                  uint32_t *handle);
VIRGL_EXPORT int
virgl_vk_allocate_memory(uint32_t device_handle,
                         const VkMemoryAllocateInfo *info,
                         uint32_t *output);
VIRGL_EXPORT int
virgl_vk_create_buffer(uint32_t device_handle,
                       VkBufferCreateInfo *info,
                       uint32_t *handle);
VIRGL_EXPORT int
virgl_vk_bind_buffer_memory(uint32_t device_handle,
                            uint32_t buffer_handle,
                            uint32_t memory_handle,
                            uint64_t offset);

VIRGL_EXPORT int
virgl_vk_write_descriptor_set(uint32_t device_handle,
                              VkWriteDescriptorSet *write_info,
                              VkDescriptorBufferInfo *buffer_info,
                              uint32_t descriptor_handle,
                              uint32_t *buffer_handles);

VIRGL_EXPORT int
virgl_vk_is_memory_cached(uint32_t device_handle,
                          uint32_t memory_handle,
                          uint8_t *output);

VIRGL_EXPORT int
virgl_vk_invalidate_memory(uint32_t device_handle,
                           uint32_t memory_handle);

VIRGL_EXPORT int
virgl_vk_map_memory(uint32_t device_handle,
                    uint32_t memory_handle,
                    uint32_t offset,
                    uint32_t size,
                    void **ptr);

VIRGL_EXPORT int
virgl_vk_unmap_memory(uint32_t device_handle,
                      uint32_t memory_handle);
#endif
