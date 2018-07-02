#include <string.h>         
#include <vulkan/vulkan.h>  
                            
#include "common/macros.h" 
#include "virgl_vtest.h" 
#include "vtest_protocol.h" 
#include "vtest_objects.h"  

int vtest_create_descriptor_set_layout(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkDescriptorSetLayoutCreateInfo vk_info;
   struct payload_create_descriptor_set_layout_intro intro;
   struct payload_create_descriptor_set_layout_pBindings pBindings;

   handle = intro.handle;
   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   vk_info.flags = intro.flags;
   vk_info.bindingCount = intro.bindingCount;

   for (uint32_t i = 0; i < vk_info.bindingCount; i++) {
      res = vtest_block_read(renderer.in_fd, &pBindings, sizeof(pBindings));
      CHECK_IO_RESULT(res, sizeof(pBindings));

      vk_info.pBindings[i].binding = pBindings.binding;
      vk_info.pBindings[i].descriptorType = pBindings.descriptorType;
      vk_info.pBindings[i].descriptorCount = pBindings.descriptorCount;
      vk_info.pBindings[i].stageFlags = pBindings.stageFlags;
   }

   result.error_code = vgl_vk_create_descriptor_set_layout(
      handle, &vk_info, &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));
   *output = result.result;
   RETURN(result.error_code);
}

int vtest_create_buffer(uint32_t length_dw)
{
   int res;
   uint32_t handle;
   struct vtest_result result;
   VkBufferCreateInfo vk_info;
   struct payload_create_buffer_intro intro;

   handle = intro.handle;
   res = vtest_block_read(renderer.in_fd, &intro, sizeof(intro));
   CHECK_IO_RESULT(res, sizeof(intro));

   vk_info.flags = intro.flags;
   vk_info.size = intro.size;
   vk_info.usage = intro.usage;
   vk_info.sharingMode = intro.sharingMode;
   vk_info.queueFamilyIndexCount = intro.queueFamilyIndexCount;

   result.error_code = vgl_vk_create_buffer(
      handle, &vk_info, &result.result);

   res = vtest_block_write(renderer.out_fd, &result, sizeof(result));
   CHECK_IO_RESULT(res, sizeof(result));
   *output = result.result;
   RETURN(result.error_code);
}
