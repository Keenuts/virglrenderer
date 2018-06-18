#ifndef VIRGL_VK_H
#define VIRGL_VK_H

#include <vulkan/vulkan.h>
#include "util/list.h"

struct list_device {
   struct list list;

   VkDevice vk_device;

   uint32_t queue_count;
   VkQueue *queues;
};

struct virgl_vk {
   VkInstance vk_instance;

   VkPhysicalDevice *physical_devices;
   uint32_t physical_device_count;

   struct list_device devices;
};

extern struct virgl_vk *vk_info;

struct virgl_vk* virgl_vk_init();
void virgl_vk_destroy(struct virgl_vk **state);

#endif
