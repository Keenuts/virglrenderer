#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "virgl_vk.h"
#include "util/macros.h"

uint32_t virgl_vk_create_device(uint32_t device_id, VkDeviceCreateInfo info)
{
   if (device_id != 0) {
      fprintf(stderr, "PoC case. Only one device is supported\n");
      abort();
   }

   return 0;
}
