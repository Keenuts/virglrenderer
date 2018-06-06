#ifndef VIRGL_VK_H
#define VIRGL_VK_H

#include <vulkan/vulkan.h>

#include "util/vector.h"

struct virgl_vk {
   VkInstance vk_instance;

   struct vector physical_devices;
};

struct virgl_vk* virgl_vk_init();
void virgl_vk_destroy(struct virgl_vk **state);

#endif
