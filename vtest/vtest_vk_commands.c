#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virglrenderer_vulkan.h"
#include "util/macros.h"
#include "vtest.h"
#include "os/os_misc.h"
#include "vtest_protocol.h"
#include "vtest_vk.h"
#include "vtest_vk_commands.h"

int vtest_vk_create_command_pool(UNUSED uint32_t length_dw)
{
   TRACE_IN();

   int res;
   struct vtest_result result = { 0 };
   VkCommandPoolCreateInfo vk_info;
   struct payload_command_pool_create_info payload;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   vk_info.flags = payload.flags;
   vk_info.queueFamilyIndex = payload.queue_family_index;

   result.error_code = virgl_vk_create_command_pool(payload.device_handle,
                                                    &vk_info,
                                                    &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   return 0;
}

int vtest_vk_allocate_command_buffers(UNUSED uint32_t length_dw)
{
   TRACE_IN();

   int res;
   struct vtest_result result = { 0 };
   VkCommandBufferAllocateInfo vk_info;
   struct payload_command_buffer_allocate_info payload;
   uint32_t *handles = NULL;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   vk_info.level = payload.level;
   vk_info.commandBufferCount = payload.count;

   handles = alloca(sizeof(uint32_t) * payload.count);
   result.error_code = virgl_vk_allocate_command_buffers(payload.device_handle,
                                                         payload.pool_handle,
                                                         &vk_info,
                                                         handles);

   result.result = payload.count;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));
   if (0 != result.error_code) {
      return result.error_code;
   }

   res = vtest_block_write(renderer.out_fd, handles, sizeof(uint32_t) * result.result);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * result.result);

   return 0;
}

int vtest_vk_record_command(UNUSED uint32_t length_dw)
{
   TRACE_IN();

   int res;
   struct vtest_result result = { 0 };
   struct payload_command_record_info payload;
   struct virgl_vk_record_info *info = NULL;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   info = alloca(sizeof(*info));
   info->cmd_handle = payload.cmd_handle;
   info->pool_handle = payload.pool_handle;
   info->pipeline_handle = payload.pipeline_handle;
   info->pipeline_layout_handle = payload.pipeline_layout_handle;
   info->bind_point = payload.bind_point;
   info->descriptor_count = payload.descriptor_count;
   memcpy(&info->dispatch_size, &payload.dispatch_size, sizeof(uint32_t) * 3);

   info->descriptor_handles = alloca(sizeof(uint32_t) * payload.descriptor_count);


   res = vtest_block_read(renderer.in_fd,
                          info->descriptor_handles,
                          sizeof(uint32_t) * payload.descriptor_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.descriptor_count);

   result.error_code = virgl_vk_record_command(payload.device_handle, info);

   result.result = 0;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   return result.error_code;
}
