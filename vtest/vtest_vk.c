#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virglrenderer_vulkan.h"
#include "util/macros.h"
#include "vtest.h"
#include "vtest_protocol.h"
#include "vtest_vk.h"

extern struct vtest_renderer renderer;

#define LOAD_UINT64(Buffer, Offset) \
   ((uint64_t)Buffer[Offset + 1] << 32 | (uint64_t)Buffer[Offset])

#define CHECK_IO_RESULT(Done, Expected)                                    \
   if ((Done) < (Expected)) {                                              \
      fprintf(stderr, "%s: failed to write back the answer.\n", __func__); \
      RETURN(-1);                                                          \
   }

int vtest_vk_enumerate_devices(uint32_t length_dw)
{
   TRACE_IN();
   UNUSED_PARAMETER(length_dw);

   uint32_t device_count;
   struct vtest_result result = { 0 };
   int res;

   res = virgl_vk_get_device_count(&device_count);
   if (res < 0) {
      result.error_code = -res;
   }

   result.result = device_count;

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   RETURN(0);
}

int vtest_vk_get_sparse_properties(uint32_t length_dw)
{
   struct vtest_payload_device_get payload;
   VkPhysicalDeviceSparseProperties sparse_props;
   struct vtest_result result = { 0 };
   int res;

   TRACE_IN();
   UNUSED_PARAMETER(length_dw);

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   res = virgl_vk_get_sparse_properties(payload.device_id, &sparse_props);
   if (res < 0) {
      result.error_code = -res;
   }

   result.result = payload.device_id;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   res = vtest_block_write(renderer.out_fd, &sparse_props, sizeof(sparse_props));
   CHECK_IO_RESULT(res, sizeof(sparse_props));

   RETURN(0);
}

int vtest_vk_get_queue_family_properties(uint32_t length_dw)
{
   struct vtest_payload_device_get payload;
   struct vtest_result result = { 0 };
   int res;

   uint32_t family_count;
   VkQueueFamilyProperties *properties = NULL;

   TRACE_IN();
   UNUSED_PARAMETER(length_dw);

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   res = virgl_vk_get_queue_family_properties(payload.device_id,
                                              &family_count,
                                              &properties);
   if (res < 0) {
      result.error_code = -res;
      RETURN(-1);
   }

   result.result = family_count;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   res = vtest_block_write(renderer.out_fd,
                           properties,
                           sizeof(*properties) * family_count);
   CHECK_IO_RESULT(res, sizeof(*properties));

   RETURN(0);
}

int vtest_vk_create_device(uint32_t length_dw)
{
   VkDeviceCreateInfo         vk_device_info = { 0 };
   VkDeviceQueueCreateInfo   *vk_queue_info = NULL;

   struct VkPhysicalDeviceFeatures     features;
   struct vtest_payload_device_create  create_info;
   struct vtest_payload_queue_create   queue_info;
   struct vtest_result result;
   int res;
   uint32_t device_id;

   UNUSED_PARAMETER(length_dw);
   TRACE_IN();

   /* The first payload is a lighter version of the VkDeviceCreationInfo */
   res = vtest_block_read(renderer.in_fd, &create_info, sizeof(create_info));
   CHECK_IO_RESULT(res, sizeof(create_info));

   vk_device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   vk_device_info.flags = create_info.flags;
   vk_device_info.queueCreateInfoCount = create_info.queue_info_count;
   vk_device_info.pEnabledFeatures = &features;
   memcpy(&features, &create_info.features, sizeof(features));

   /* Now, for each queue, we need to extract the informations */
   vk_device_info.pQueueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) *
                                           create_info.queue_info_count);
   if (vk_device_info.pQueueCreateInfos == NULL) {
      RETURN(-1);
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

      vtest_block_read(renderer.in_fd, &vk_queue_info->pQueuePriorities,
                       sizeof(float) * queue_info.queue_count);
      CHECK_IO_RESULT(res, sizeof(float) * queue_info.queue_count);
   }

   result.error_code = virgl_vk_create_device(create_info.physical_device_id,
                                              &vk_device_info,
                                              &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   RETURN(0);
}
