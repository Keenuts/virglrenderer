#ifndef VIRGL_VK_H
#define VIRGL_VK_H

#include <vulkan/vulkan.h>
#include "util/list.h"

struct virgl_vk {
   VkInstance vk_instance;

   VkPhysicalDevice *physical_devices;
   uint32_t physical_device_count;

   struct vk_device *devices;
};

struct vk_device {
   struct list list;

   VkDevice vk_device;

   uint32_t queue_count;
   VkQueue *queues;

   uint32_t next_handle;
   struct util_hash_table *objects;
};

struct vk_object {
   uint32_t handle;

   VkDevice vk_device;
   void *vk_handle;

   void (*cleanup_callback)(VkDevice, void*, void*);
};


extern struct virgl_vk *vk_info;

const char* vkresult_to_string(VkResult res);

struct virgl_vk* virgl_vk_init();
void virgl_vk_destroy(struct virgl_vk **state);

#endif
