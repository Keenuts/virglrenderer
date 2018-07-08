/*
 * NOTICE:
 * The code and types used here have been partialy generated.
 * Only special cases are then fixed by hand.
 * This will explain weirdly long type-names and some other things.
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
vtest_vk_create_descriptor_pool(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkDescriptorPoolCreateInfo vk_info;

   struct payload_create_descriptor_pool_intro intro;

   memset(&vk_info, 0, sizeof(vk_info));

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));
   vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   vk_info.flags = intro.flags;
   vk_info.maxSets = intro.maxSets;
   vk_info.poolSizeCount = intro.poolSizeCount;

   struct payload_create_descriptor_pool_pPoolSizes tmp_pPoolSizes;
   VkDescriptorPoolSize *pPoolSizes = NULL;
   pPoolSizes = alloca(sizeof(*pPoolSizes) * vk_info.poolSizeCount);

   for (uint32_t i = 0; i < intro.poolSizeCount; i++) {
      res = vtest_block_read(renderer.in_fd, &tmp_pPoolSizes, sizeof(tmp_pPoolSizes));
      CHECK_IO_RESULT(res, sizeof(tmp_pPoolSizes));
      pPoolSizes[i].type = tmp_pPoolSizes.type;
      pPoolSizes[i].descriptorCount = tmp_pPoolSizes.descriptorCount;
   }

   vk_info.pPoolSizes = pPoolSizes;

   result.error_code = virgl_vk_create_descriptor_pool(handle, &vk_info, &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   TRACE_OUT();
   RETURN(0);
}

int
vtest_vk_create_descriptor_set_layout(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkDescriptorSetLayoutCreateInfo vk_info;
   VkDescriptorSetLayoutBinding *pBindings = NULL;

   struct payload_create_descriptor_set_layout_intro intro;
   struct payload_create_descriptor_set_layout_pBindings binding;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   vk_info.flags = intro.flags;
   vk_info.bindingCount = intro.bindingCount;

   /* reading bindings */
   pBindings = alloca(sizeof(*pBindings) * vk_info.bindingCount);

   for (uint32_t i = 0; i < vk_info.bindingCount; i++) {
      res = vtest_block_read(renderer.in_fd, &binding, sizeof(binding));
      CHECK_IO_RESULT(res, sizeof(binding));
      pBindings[i].binding = binding.binding;
      pBindings[i].descriptorType = binding.descriptorType;
      pBindings[i].descriptorCount = binding.descriptorCount;
      pBindings[i].stageFlags = binding.stageFlags;
      pBindings[i].pImmutableSamplers = NULL;
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

   TRACE_IN();

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

int
vtest_vk_create_pipeline_layout(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkPipelineLayoutCreateInfo vk_info;
   uint32_t *set_handles = NULL;
   VkPushConstantRange *vk_push_ranges = NULL;

   struct payload_create_pipeline_layout_intro intro;
   struct payload_create_pipeline_layout_pPushConstantRanges push_range;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   /* generic informations */
   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   vk_info.flags = intro.flags;
   vk_info.setLayoutCount = intro.setLayoutCount;
   vk_info.pushConstantRangeCount = intro.pushConstantRangeCount;

   /* first array: VkDescriptorSetLayout */
   set_handles = alloca(sizeof(*set_handles) * vk_info.setLayoutCount);
   res = vtest_block_read(renderer.in_fd, set_handles,
                          sizeof(*set_handles) * vk_info.setLayoutCount);
   CHECK_IO_RESULT(res, sizeof(*set_handles) * vk_info.setLayoutCount);

   /* second array: VkPushConstantRange */
   vk_push_ranges = alloca(sizeof(*vk_push_ranges) * vk_info.pushConstantRangeCount);

   for (uint32_t i = 0; i < vk_info.pushConstantRangeCount; i++) {
      res = vtest_block_read(renderer.in_fd, &push_range, sizeof(push_range));
      CHECK_IO_RESULT(res, sizeof(push_range));

      vk_push_ranges[i].stageFlags = push_range.stageFlags;
      vk_push_ranges[i].offset = push_range.offset;
      vk_push_ranges[i].size = push_range.size;
   }

   vk_info.pPushConstantRanges = vk_push_ranges;

   /* virgl forwarding */
   result.error_code = virgl_vk_create_pipeline_layout(handle,
                                                       &vk_info,
                                                       set_handles,
                                                       &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   RETURN(0);
}

int
vtest_vk_create_compute_pipelines(uint32_t length_dw)
{
   int res;
   struct vtest_result result;
   VkComputePipelineCreateInfo vk_info;
   char *entrypoint_name = NULL;

   struct payload_create_compute_pipelines_intro intro;

   /* reading intro structure */
   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   /* reading entrypoint name */
   entrypoint_name = alloca(intro.entrypoint_len);
   res = vtest_block_read(renderer.in_fd, entrypoint_name, intro.entrypoint_len);
   CHECK_IO_RESULT(res, intro.entrypoint_len);

   /* setting up vk_info structure */
   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
   vk_info.flags = intro.flags;
   vk_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   vk_info.stage.flags = intro.stage_flags;
   vk_info.stage.stage = intro.stage_stage;
   vk_info.stage.pName = entrypoint_name;

   result.error_code = virgl_vk_create_compute_pipelines(intro.handle,
                                                         &vk_info,
                                                         intro.layout,
                                                         intro.stage_module,
                                                         &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   TRACE_OUT();
   RETURN(0);
}
