#include <stdio.h>
#include <stdlib.h>
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

int vtest_vk_create_device(uint32_t length_dw)
{
   const float priorities[] = { 0.f };
   int res;
   struct vtest_result result = { 0 };
   uint32_t id;

   TRACE_IN();
   UNUSED_PARAMETER(length_dw);

   VkDeviceQueueCreateInfo queue_info  = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      NULL,
      0,
      0,
      1,
      priorities
   };

   VkDeviceCreateInfo info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      NULL,
      0,
      1,
      &queue_info,
      0,
      NULL,
      0,
      NULL,
      NULL
   };

   //FIXME: device id
   res = virgl_vk_create_device(0, info, &result.result);
   if (res < 0) {
      result.error_code = -res;
   }

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   RETURN(0);
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
   TRACE_IN();
   UNUSED_PARAMETER(length_dw);

   struct vtest_payload_device_get payload;
   VkPhysicalDeviceSparseProperties sparse_props;
   struct vtest_result result = { 0 };
   int res;

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
