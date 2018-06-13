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

int vtest_vk_create_device(uint32_t length_dw)
{
   const float priorities[] = { 0.f };
   int res;
   struct vtest_result result;
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
   id = virgl_vk_create_device(0, info);

   result.error_code = 0;
   result.result = id;

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   if (res < sizeof(res)) {
      fprintf(stderr, "%s: failed to write back the answer.\n", __func__);
      RETURN(-1);

   }

   RETURN(0);
}
