#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virglrenderer_vulkan.h"
#include "vtest.h"
#include "vtest_protocol.h"
#include "vtest_vk.h"
#include "vtest_vk_objects.h"

extern struct vtest_renderer renderer;

int
vtest_vk_create_descriptor_pool(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
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

   result.error_code = virgl_vk_create_descriptor_pool(intro.handle,
                                                       &vk_info,
                                                       &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_create_descriptor_set_layout(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
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

   result.error_code = virgl_vk_create_descriptor_set_layout(intro.handle,
                                                             &vk_info,
                                                             &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

/* payload:
 *    - generic pool_handle
 *    - descripor layout handles[]
 */
int
vtest_vk_allocate_descriptor_sets(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   uint32_t pool_handle;
   uint32_t *set_layout_handles = NULL;
   uint32_t *output_handles = NULL;

   struct payload_allocate_descriptor_sets_intro intro;
   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   pool_handle = intro.descriptorPool;

   /* reading all handles sent at the end */
   set_layout_handles = alloca(sizeof(uint32_t) * intro.descriptorSetCount);

   res = vtest_block_read(renderer.in_fd,
                          set_layout_handles,
                          sizeof(uint32_t) * intro.descriptorSetCount);

   output_handles = alloca(sizeof(uint32_t) * intro.descriptorSetCount);
   result.error_code = virgl_vk_allocate_descriptor_set(intro.handle,
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

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_create_shader_module(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   VkShaderModuleCreateInfo vk_info;
   struct payload_create_shader_module_intro intro;
   uint32_t *shader_code = NULL;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));
   memset(&vk_info, 0, sizeof(vk_info));

   vk_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   vk_info.flags = intro.flags;
   vk_info.codeSize = intro.codeSize;
   shader_code = malloc(vk_info.codeSize);
   if (NULL == shader_code) {
      return -1;
   }

   res = vtest_block_read(renderer.in_fd, shader_code, vk_info.codeSize);
   CHECK_IO_RESULT(res, vk_info.codeSize);

   vk_info.pCode = shader_code;

   result.error_code = virgl_vk_create_shader_module(intro.handle,
                                                     &vk_info,
                                                     &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_create_pipeline_layout(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
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
   result.error_code = virgl_vk_create_pipeline_layout(intro.handle,
                                                       &vk_info,
                                                       set_handles,
                                                       &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
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

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_allocate_memory(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   VkMemoryAllocateInfo vk_info;

   struct payload_allocate_memory intro;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   vk_info.allocationSize = intro.device_size;
   vk_info.memoryTypeIndex = intro.memory_index;

   result.error_code = virgl_vk_allocate_memory(intro.handle, &vk_info, &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_create_buffer(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   VkBufferCreateInfo vk_info;

   struct payload_create_buffer intro;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   vk_info.flags = intro.flags;
   vk_info.size = intro.size;
   vk_info.usage = intro.usage;
   vk_info.sharingMode = intro.sharingMode;
   vk_info.queueFamilyIndexCount = intro.queueFamilyIndexCount;
   vk_info.pQueueFamilyIndices = NULL;

   if (0 != vk_info.queueFamilyIndexCount) {
      vk_info.pQueueFamilyIndices = alloca(sizeof(uint32_t)
                                           * vk_info.queueFamilyIndexCount);
      res = vtest_block_read(renderer.in_fd,
                             (void*)vk_info.pQueueFamilyIndices,
                             sizeof(uint32_t) * vk_info.queueFamilyIndexCount);
      CHECK_IO_RESULT(res, sizeof(uint32_t) * vk_info.queueFamilyIndexCount);
   }

   result.error_code = virgl_vk_create_buffer(intro.handle, &vk_info, &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_bind_buffer_memory(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   struct payload_bind_buffer_memory intro;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   result.result = 0;
   result.error_code = virgl_vk_bind_buffer_memory(intro.device_handle,
                                                   intro.buffer_handle,
                                                   intro.memory_handle,
                                                   intro.offset);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_write_descriptor_set(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   VkWriteDescriptorSet vk_info;
   VkDescriptorBufferInfo *pBufferInfo = NULL;

   struct payload_write_descriptor_set_intro intro;
   struct payload_write_descriptor_set_buffer p_buffer;
   uint32_t *buffer_handles;
   uint32_t descriptor_handle;

   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   memset(&vk_info, 0, sizeof(vk_info));
   vk_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   vk_info.dstBinding = intro.dstBinding;
   vk_info.dstArrayElement = intro.dstArrayElement;
   vk_info.descriptorType = intro.descriptorType;
   vk_info.descriptorCount = intro.descriptorCount;

   descriptor_handle = intro.dstSet;

   pBufferInfo = alloca(sizeof(*pBufferInfo) * vk_info.descriptorCount);
   buffer_handles = alloca(sizeof(uint32_t) * vk_info.descriptorCount);

   for (uint32_t i = 0; i < intro.descriptorCount; i++) {
      res = vtest_block_read(renderer.in_fd, &p_buffer, sizeof(p_buffer));
      CHECK_IO_RESULT(res, sizeof(p_buffer));

      buffer_handles[i] = p_buffer.buffer_handle;
      pBufferInfo[i].offset = p_buffer.offset;
      pBufferInfo[i].range = p_buffer.range;
   }

   result.error_code = virgl_vk_write_descriptor_set(intro.device_handle,
                                                     &vk_info,
                                                     pBufferInfo,
                                                     descriptor_handle,
                                                     buffer_handles);
   result.result = 0;
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_create_fence(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   VkFenceCreateInfo vk_info = { 0 };
   struct payload_create_fence payload;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   vk_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   vk_info.flags = payload.flags;

   result.error_code = virgl_vk_create_fence(payload.device_handle,
                                             &vk_info,
                                             &result.result);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_wait_for_fences(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   struct payload_wait_for_fences payload;
   uint32_t *fences = NULL;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   fences = alloca(sizeof(uint32_t) * payload.fence_count);
   res = vtest_block_read(renderer.in_fd, fences, sizeof(uint32_t) * payload.fence_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.fence_count);

   result.error_code = virgl_vk_wait_for_fences(payload.device_handle,
                                                payload.fence_count,
                                                fences,
                                                payload.wait_all,
                                                payload.timeout);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}

int
vtest_vk_queue_submit(uint32_t length_dw)
{
   int res;
   struct vtest_result result = { 0 };
   struct payload_queue_submit payload;
   struct virgl_vk_submit_info info;

   res = vtest_block_read(renderer.in_fd, &payload, sizeof(payload));
   CHECK_IO_RESULT(res, sizeof(payload));

   memcpy(&info, &payload, sizeof(payload));
   info.wait_handles = alloca(sizeof(uint32_t) * info.wait_count);
   info.wait_stage_masks = alloca(sizeof(uint32_t) * info.wait_count);
   info.cmd_handles = alloca(sizeof(uint32_t) * info.cmd_count);
   info.pool_handles = alloca(sizeof(uint32_t) * info.cmd_count);
   info.signal_handles = alloca(sizeof(uint32_t) * info.signal_count);

   res = vtest_block_read(renderer.in_fd,
                          info.wait_handles,
                          sizeof(uint32_t) * payload.wait_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.wait_count);
   res = vtest_block_read(renderer.in_fd,
                          info.wait_stage_masks,
                          sizeof(uint32_t) * payload.wait_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.wait_count);

   res = vtest_block_read(renderer.in_fd,
                          info.pool_handles,
                          sizeof(uint32_t) * payload.cmd_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.cmd_count);
   res = vtest_block_read(renderer.in_fd,
                          info.cmd_handles,
                          sizeof(uint32_t) * payload.cmd_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.cmd_count);

   res = vtest_block_read(renderer.in_fd,
                          info.signal_handles,
                          sizeof(uint32_t) * payload.signal_count);
   CHECK_IO_RESULT(res, sizeof(uint32_t) * payload.signal_count);

   result.error_code = virgl_vk_queue_submit(&info);
   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));

   UNUSED_PARAMETER(length_dw);
   return 0;
}
