#ifndef VIRGL_VK_H
#define VIRGL_VK_H

#include <vulkan/vulkan.h>
#include "util/list.h"

struct list_physical_device {
   struct list list;
   VkPhysicalDevice vk_device;
};

struct list_device {
   struct list list;

   VkDevice vk_device;

   uint32_t queue_count;
   VkQueue *queues;
};

struct virgl_vk {
   VkInstance vk_instance;

   struct list_physical_device physical_devices;
   struct list_device devices;
};

extern struct virgl_vk *vk_info;

struct virgl_vk* virgl_vk_init();
void virgl_vk_destroy(struct virgl_vk **state);

#endif
