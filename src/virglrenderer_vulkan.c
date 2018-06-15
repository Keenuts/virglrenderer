#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virgl_vk.h"
#include "util/macros.h"

int virgl_vk_get_device_count(uint32_t *device_count)
{
   uint32_t dev_count;
   VkResult res;

   TRACE_IN();

   *device_count = list_length(&vk_info->physical_devices.list);

   RETURN(0);
}

int virgl_vk_get_sparse_properties(uint32_t device_id,
                                   VkPhysicalDeviceSparseProperties *sparse_props)
{
   struct VkPhysicalDeviceProperties props;
   struct list_physical_device *it = NULL;

   TRACE_IN();

   if (device_id < 0) {
      RETURN(-1);
   }

   LIST_FOR_EACH(it, vk_info->physical_devices.list, list) {
      if (device_id > 0) {
         device_id -= 1;
         continue;
      }

      vkGetPhysicalDeviceProperties(it->vk_device, &props);
      memcpy(sparse_props, &props.sparseProperties, sizeof(*sparse_props));
      break;
   }

   if (device_id > 0) {
      RETURN(-1);
   }

   RETURN(0);
}

static VkPhysicalDevice get_physical_device(uint32_t id)
{
   struct list_physical_device *it = NULL;

   LIST_FOR_EACH(it, vk_info->physical_devices.list, list) {
      if (id > 0) {
         id -= 1;
         continue;
      }

      RETURN(it->vk_device);
   }

   RETURN(VK_NULL_HANDLE);
}

int virgl_vk_get_queue_family_properties(uint32_t device_id,
                                         uint32_t *family_count,
                                         VkQueueFamilyProperties **props)
{
   VkPhysicalDevice dev;

   TRACE_IN();

   dev = get_physical_device(device_id);
   if (dev == VK_NULL_HANDLE) {
      RETURN(-1);
   }

   vkGetPhysicalDeviceQueueFamilyProperties(dev, family_count, NULL);

   *props = malloc(sizeof(VkQueueFamilyProperties) * *family_count);
   if (*props == NULL) {
      *family_count = 0;
      RETURN(-1);
   }

   vkGetPhysicalDeviceQueueFamilyProperties(dev, family_count, *props);

   RETURN(0);
}

static int initialize_vk_device(VkDevice dev,
                                const VkDeviceCreateInfo *info,
                                uint32_t *device_id)
{
   struct list_device *device;
   const VkDeviceQueueCreateInfo *queue_info;
   uint32_t queue_count;

   device = calloc(1, sizeof(*device));
   if (device == NULL) {
      return -1;
   }

   *device_id = list_length(&vk_info->devices.list);

   list_init(&device->list);
   device->vk_device = dev;

   /* initializing device queues */
   device->queue_count = 0; 
   for (uint32_t i = 0; i < info->queueCreateInfoCount; i++) {
      device->queue_count += info->pQueueCreateInfos[i].queueCount;
   }

   device->queues = malloc(sizeof(VkQueue) * device->queue_count);
   if (device->queues == NULL) {
      free(device);
      return -1;
   }

   uint32_t id = 0;
   for (uint32_t i = 0; i < info->queueCreateInfoCount; i++) {
      queue_info = info->pQueueCreateInfos + i;
      
      for (uint32_t j = 0; j < queue_info->queueCount; j++) {
         vkGetDeviceQueue(dev, queue_info->queueFamilyIndex, j, device->queues + id);
         id += 1;
      }
   }

   list_append(&vk_info->devices.list, &device->list);
   return 0;
}

int virgl_vk_create_device(uint32_t phys_device_id,
                           const VkDeviceCreateInfo *info,
                           uint32_t *device_id)
{
   TRACE_IN();

   VkDevice dev;
   VkResult res;
   VkPhysicalDevice physical_dev;
   
   physical_dev = get_physical_device(phys_device_id);
   if (physical_dev == VK_NULL_HANDLE) {
      RETURN(-1);
   }

   res = vkCreateDevice(physical_dev, info, NULL, &dev);
   if (res != VK_SUCCESS) {
      RETURN(-2);
   }

   if (initialize_vk_device(dev, info, device_id) < 0) {
      vkDestroyDevice(dev, NULL);
      RETURN(-3);
   }

   RETURN(0);
}
