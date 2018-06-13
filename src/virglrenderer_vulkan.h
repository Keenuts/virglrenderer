#ifndef VIRGLRENDERER_VULKAN_H
#define VIRGLRENDERER_VULKAN_H

#include "virglrenderer.h"
#include <vulkan/vulkan.h>

VIRGL_EXPORT uint32_t
virgl_vk_create_device(uint32_t device_id, VkDeviceCreateInfo info);

#endif
