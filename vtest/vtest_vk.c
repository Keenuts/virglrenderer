#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "vtest_protocol.h"
#include "vtest.h"
#include "vtest_vk.h"

extern struct vtest_renderer renderer;

#define LOAD_UINT64(Buffer, Offset) \
   ((uint64_t)Buffer[Offset + 1] << 32 | (uint64_t)Buffer[Offset])

int vtest_vk_allocate(uint32_t header_len)
{
   int ret;
   uint32_t buffer[VCMD_VK_ALLOCATE_SIZE];
   VkResult res;

   ret = vtest_block_read(renderer.in_fd, buffer, sizeof(buffer));

   if (ret != sizeof(buffer)) {
      return -1;
   }

   VkDevice device = (VkDevice)LOAD_UINT64(buffer, VCMD_VK_ALLOCATE_P_DEVICE_ID);
   uint64_t size = LOAD_UINT64(buffer, VCMD_VK_ALLOCATE_P_SIZE);

   VkMemoryAllocateInfo info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      NULL,
      size,
      buffer[VCMD_VK_ALLOCATE_P_MEM_INDEX],
   };

   printf("calling VK with:\t%p\n \t0x%x\n \t0x%x\n", device,
          info.allocationSize, info.memoryTypeIndex);

   abort();

   VkDeviceMemory vk_memory;
   res = vkAllocateMemory(device, &info, NULL, &vk_memory);
   printf("Vk returned %d\n", res);

   uint32_t result[] = {
      res,
      (uint64_t)vk_memory & 0xFFFFFFFFUL,
      (uint64_t)vk_memory >> 32,
   };

   vtest_block_write(renderer.in_fd, result, sizeof(result));

   return 0;
}
