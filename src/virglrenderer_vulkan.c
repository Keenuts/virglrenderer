#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virgl_vk.h"
#include "util/macros.h"

int virgl_vk_create_device(uint32_t phys_device, VkDeviceCreateInfo info, uint32_t *dev)
{
   if (phys_device != 0) {
      fprintf(stderr, "PoC case. Only one device is supported\n");
      abort();
   }

   return 0;
}

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

int virgl_vk_get_queue_family_properties(uint32_t device_id,
                                         uint32_t *family_count,
                                         VkQueueFamilyProperties **props)
{
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

      vkGetPhysicalDeviceQueueFamilyProperties(it->vk_device,
                                               family_count,
                                               NULL);

      *props = malloc(sizeof(VkQueueFamilyProperties) * *family_count);
      if (*props == NULL) {
         *family_count = 0;
         RETURN(-1);
      }

      vkGetPhysicalDeviceQueueFamilyProperties(it->vk_device,
                                               family_count,
                                               *props);
      break;
   }

   RETURN(0);
}
