/*
 This file has been generated. Please do not modify it.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virglrenderer_vulkan.h"
#include "util/macros.h"
#include "vtest.h"
#include "vtest_protocol.h"
#include "vtest_vk.h"
#include "vtest_vk_objects.h"

extern struct vtest_renderer renderer;

int
vtest_vk_create_descriptor_set_layout(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkDescriptorSetLayoutCreateInfo vk_info;

   struct payload_create_descriptor_set_layout_intro intro;

   memset(&vk_info, 0, sizeof(vk_info));

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));
   vk_info.flags = intro.flags;
   vk_info.bindingCount = intro.bindingCount;

   struct payload_create_descriptor_set_layout_pBindings tmp_pBindings;
   VkDescriptorSetLayoutBinding *pBindings = NULL;
   pBindings = alloca(sizeof(*pBindings) * vk_info.bindingCount);

   for (uint32_t i = 0; i < intro.bindingCount; i++) {
      res = vtest_block_read(renderer.in_fd, &tmp_pBindings, sizeof(tmp_pBindings));
      CHECK_IO_RESULT(res, sizeof(tmp_pBindings));
      pBindings[i].binding = tmp_pBindings.binding;
      pBindings[i].descriptorType = tmp_pBindings.descriptorType;
      pBindings[i].descriptorCount = tmp_pBindings.descriptorCount;
      pBindings[i].stageFlags = tmp_pBindings.stageFlags;
   }

   vk_info.pBindings = pBindings;

   result.error_code = virgl_vk_create_descriptor_set_layout(handle, &vk_info, &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   TRACE_OUT();
   RETURN(0);
}

int
vtest_vk_create_buffer(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkBufferCreateInfo vk_info;

   struct payload_create_buffer_intro intro;

   memset(&vk_info, 0, sizeof(vk_info));

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));
   vk_info.flags = intro.flags;
   vk_info.size = intro.size;
   vk_info.usage = intro.usage;
   vk_info.sharingMode = intro.sharingMode;
   vk_info.queueFamilyIndexCount = intro.queueFamilyIndexCount;

   result.error_code = virgl_vk_create_buffer(handle, &vk_info, &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   TRACE_OUT();
   RETURN(0);
}

/* payload:
 *    - generic pool_handle
 *    - descripor layout handles[]
 */
int
vtest_vk_allocate_descriptor_sets(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   uint32_t pool_handle;
   uint32_t *set_layout_handles = NULL;
   uint32_t *output_handles = NULL;

   struct payload_allocate_descriptor_sets_intro intro;
   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   pool_handle = intro.descriptorPool;

   /* reading all handles sent at the end */
   set_layout_handles = alloca(sizeof(uint32_t) * intro.descriptorSetCount);

   printf("Reading %d handles\n", intro.descriptorSetCount);
   res = vtest_block_read(renderer.in_fd,
                          set_layout_handles,
                          sizeof(uint32_t) * intro.descriptorSetCount);

   output_handles = alloca(sizeof(uint32_t) * intro.descriptorSetCount);
   result.error_code = virgl_vk_allocate_descriptor_set(handle,
                                                        pool_handle,
                                                        intro.descriptorSetCount,
                                                        set_layout_handles,
                                                        output_handles);

   result.result = intro.descriptorSetCount;
   /* Writting back the results */
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));
   res = vtest_block_write(renderer.out_fd,
                           output_handles,
                           sizeof(uint32_t) * result.result);
   CHECK_IO_RESULT(res, result.result * sizeof(uint32_t));

   TRACE_OUT();
   RETURN(0);
}

int
vtest_vk_create_shader_module(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkShaderModuleCreateInfo vk_info;
   struct payload_create_shader_module_intro intro;
   uint32_t *shader_code = NULL;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));
   memset(&vk_info, 0, sizeof(vk_info));

   vk_info.flags = intro.flags;
   vk_info.codeSize = intro.codeSize;
   shader_code = malloc(vk_info.codeSize);
   if (NULL == shader_code) {
      RETURN(-1);
   }

   res = vtest_block_read(renderer.in_fd, shader_code, vk_info.codeSize);
   CHECK_IO_RESULT(res, vk_info.codeSize);

   vk_info.pCode = shader_code;

   result.error_code = virgl_vk_create_shader_module(handle, &vk_info, &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   TRACE_OUT();
   RETURN(0);
}
