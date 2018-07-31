#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virglrenderer_vulkan.h"
#include "vtest.h"
#include "os/os_misc.h"
#include "vtest_protocol.h"
#include "vtest_vk.h"

extern struct vtest_renderer renderer;

int vtest_vk_create_device(UNUSED uint32_t length_dw)
{
   VkDeviceCreateInfo         vk_device_info;;
   VkDeviceQueueCreateInfo   *vk_queue_info = NULL;

   struct VkPhysicalDeviceFeatures     features;
   struct vtest_payload_device_create  create_info;
   struct vtest_payload_queue_create   queue_info;
   struct vtest_result result = { 0 };
   int res;

   /* The first payload is a lighter version of the VkDeviceCreationInfo */
   res = vtest_block_read(renderer.in_fd, &create_info, sizeof(create_info));
   CHECK_IO_RESULT(res, (int)sizeof(create_info));

   memset(&vk_device_info, 0, sizeof(vk_device_info));
   vk_device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   vk_device_info.flags = create_info.flags;
   vk_device_info.queueCreateInfoCount = create_info.queue_info_count;
   vk_device_info.pEnabledFeatures = &features;
   memcpy(&features, &create_info.features, sizeof(features));

   /* Now, for each queue, we need to extract the informations */
   vk_device_info.pQueueCreateInfos = alloca(sizeof(VkDeviceQueueCreateInfo) *
                                           create_info.queue_info_count);
   if (vk_device_info.pQueueCreateInfos == NULL) {
      return -1;
   }

   for (uint32_t i = 0; i < create_info.queue_info_count; i++) {
      // Cast because the pointer is declared as const in the VK struct
      vk_queue_info = (void*)vk_device_info.pQueueCreateInfos + i;

      res = vtest_block_read(renderer.in_fd, &queue_info, sizeof(queue_info));
      CHECK_IO_RESULT(res, sizeof(queue_info));

      vk_queue_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      vk_queue_info->pNext = NULL;
      vk_queue_info->flags = queue_info.flags;
      vk_queue_info->queueFamilyIndex = queue_info.queue_family_index;
      vk_queue_info->queueCount = queue_info.queue_count;
      vk_queue_info->pQueuePriorities = alloca(sizeof(float) * queue_info.queue_count);

      res = vtest_block_read(renderer.in_fd, (float*)vk_queue_info->pQueuePriorities,
                       sizeof(float) * queue_info.queue_count);
      CHECK_IO_RESULT(res, sizeof(float) * queue_info.queue_count);
   }

   res = virgl_vk_create_device(create_info.physical_device_id,
                                 &vk_device_info,
                                 &result.result);
   if (0 > res) {
      return res;
   }

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   return 0;
}

int
vtest_vk_destroy_device(UNUSED uint32_t length_dw)
{
   struct vtest_payload_destroy_device payload;
   struct vtest_result result = { 0 };
   int res;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, (int)sizeof(payload));

   result.result = virgl_vk_destroy_device(payload.device_handle);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));
   return 0;
}

int
vtest_vk_destroy_object(UNUSED uint32_t length_dw)
{
   struct vtest_payload_destroy_object payload;
   struct vtest_result result = { 0 };
   int res;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, (int)sizeof(payload));

   result.result = virgl_vk_destroy_object(payload.device_handle,
                                           payload.object_handle);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));
   return 0;
}

int vtest_vk_enumerate_devices(UNUSED uint32_t length_dw)
{
   uint32_t device_count;
   struct vtest_result result = { 0 };
   int res;

   res = virgl_vk_get_device_count(&device_count);
   if (0 > res) {
      return res;
   }

   result.result = device_count;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   return 0;
}

int vtest_vk_get_device_memory_properties(UNUSED uint32_t length_dw)
{
   struct vtest_payload_device_get payload;
   struct vtest_result result = { 0 };
   int res;

   VkPhysicalDeviceMemoryProperties properties;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   res = virgl_vk_get_memory_properties(payload.device_id, &properties);
   if (0 > res) {
      return res;
   }

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   res = vtest_block_write(renderer.out_fd, &properties, sizeof(properties));
   CHECK_IO_RESULT(res, sizeof(properties));

   return 0;
}

int vtest_vk_get_queue_family_properties(UNUSED uint32_t length_dw)
{
   struct vtest_payload_device_get payload;
   struct vtest_result result = { 0 };
   int res;

   uint32_t family_count;
   VkQueueFamilyProperties *properties = NULL;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   res = virgl_vk_get_queue_family_properties(payload.device_id,
                                               &family_count,
                                               &properties);
   if (0 > res) {
      return res;
   }

   result.result = family_count;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   res = vtest_block_write(renderer.out_fd,
                           properties,
                           sizeof(*properties) * family_count);
   CHECK_IO_RESULT(res, sizeof(*properties));

   return 0;
}

int vtest_vk_get_sparse_properties(UNUSED uint32_t length_dw)
{
   struct vtest_payload_device_get payload;
   VkPhysicalDeviceSparseProperties sparse_props;
   struct vtest_result result = { 0 };
   int res;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   res = virgl_vk_get_sparse_properties(payload.device_id, &sparse_props);
   if (0 > res) {
      return res;
   }

   result.result = payload.device_id;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   res = vtest_block_write(renderer.out_fd, &sparse_props, sizeof(sparse_props));
   CHECK_IO_RESULT(res, sizeof(sparse_props));

   return 0;
}

int vtest_vk_read_memory(UNUSED uint32_t length_dw)
{
   uint8_t cached;
   struct vtest_result result = { 0 };
   struct vtest_payload_rw_memory info;
   void *data = NULL;
   int res;

   res = vtest_block_read(renderer.in_fd, &info, sizeof(info));
   CHECK_IO_RESULT(res, sizeof(info));

   res = virgl_vk_map_memory(info.device_handle,
                             info.memory_handle,
                             info.offset,
                             info.size,
                             &data);
   if (0 > res) {
      return res;
   }

   do {
      res = virgl_vk_is_memory_cached(info.device_handle, info.memory_handle, &cached);
      if (0 > res) {
         break;
      }

      if (cached) {
         res = virgl_vk_invalidate_memory(info.device_handle, info.memory_handle);
         if (0 > res) {
            break;
         }
      }

      res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
      if (res < (int)sizeof(result)) {
         break;
      }

      res = vtest_block_write(renderer.out_fd, data, info.size);
      if (res != (int)info.size) {
         break;
      }

      res = 0;
   } while (0);

   if (virgl_vk_unmap_memory(info.device_handle, info.memory_handle) < 0) {
      fprintf(stderr, "%s: unmap failed\n", __func__);
   }

   return res;
}

int vtest_vk_write_memory(UNUSED uint32_t length_dw)
{
   int res = 0;
   uint8_t cached;
   struct vtest_result result = { 0 };
   struct vtest_payload_rw_memory info;
   void *data = NULL;

   res = vtest_block_read(renderer.in_fd, &info, sizeof(info));
   CHECK_IO_RESULT(res, sizeof(info));

   res = virgl_vk_map_memory(info.device_handle,
                             info.memory_handle,
                             info.offset,
                             info.size,
                             &data);
   if (0 > res) {
      return res;
   }

   do {
      res = virgl_vk_is_memory_cached(info.device_handle, info.memory_handle, &cached);
      if (0 > res) {
         break;
      }

      res = vtest_block_read(renderer.out_fd, data, info.size);
      if (0 > res) {
         break;
      }

      if (cached) {
         res = virgl_vk_flush_memory(info.device_handle, info.memory_handle);
         if (0 > res) {
            virgl_vk_unmap_memory(info.device_handle, info.memory_handle);
            break;
         }
      }
   } while (0);

   if (virgl_vk_unmap_memory(info.device_handle, info.memory_handle) < 0) {
      fprintf(stderr, "%s: unmap failed\n", __func__);
   }

   result.error_code = res;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   return 0;
}
