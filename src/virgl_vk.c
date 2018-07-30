#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "util/u_double_list.h"
#include "util/u_memory.h"
#include "virgl_vk.h"

struct virgl_vk *vulkan_state;

const char* vkresult_to_string(VkResult res)
{
   switch (res)
   {
#define VK2STR(Value) case Value: return #Value
      VK2STR(VK_SUCCESS);
      VK2STR(VK_NOT_READY);
      VK2STR(VK_TIMEOUT);
      VK2STR(VK_EVENT_SET);
      VK2STR(VK_EVENT_RESET);
      VK2STR(VK_INCOMPLETE);
      VK2STR(VK_ERROR_OUT_OF_HOST_MEMORY);
      VK2STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
      VK2STR(VK_ERROR_INITIALIZATION_FAILED);
      VK2STR(VK_ERROR_DEVICE_LOST);
      VK2STR(VK_ERROR_MEMORY_MAP_FAILED);
      VK2STR(VK_ERROR_LAYER_NOT_PRESENT);
      VK2STR(VK_ERROR_EXTENSION_NOT_PRESENT);
      VK2STR(VK_ERROR_FEATURE_NOT_PRESENT);
      VK2STR(VK_ERROR_INCOMPATIBLE_DRIVER);
      VK2STR(VK_ERROR_TOO_MANY_OBJECTS);
      VK2STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
      VK2STR(VK_ERROR_FRAGMENTED_POOL);
      VK2STR(VK_ERROR_OUT_OF_POOL_MEMORY);
      VK2STR(VK_ERROR_INVALID_EXTERNAL_HANDLE);
      VK2STR(VK_ERROR_SURFACE_LOST_KHR);
      VK2STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
      VK2STR(VK_SUBOPTIMAL_KHR);
      VK2STR(VK_ERROR_OUT_OF_DATE_KHR);
      VK2STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
      VK2STR(VK_ERROR_VALIDATION_FAILED_EXT);
      VK2STR(VK_ERROR_INVALID_SHADER_NV);
      VK2STR(VK_ERROR_FRAGMENTATION_EXT);
      VK2STR(VK_ERROR_NOT_PERMITTED_EXT);
      VK2STR(VK_RESULT_MAX_ENUM);
#undef VK2STR
      default:
      return "VK_UNKNOWN_RETURN_VALUE";
   }
}

static int
init_physical_devices(void)
{
   uint32_t device_count;
   VkResult res;

   vulkan_state->devices = CALLOC_STRUCT(vk_device);
   if (NULL == vulkan_state->devices) {
      return -1;
   }

   LIST_INITHEAD(&vulkan_state->devices->list);

   res = vkEnumeratePhysicalDevices(vulkan_state->vk_instance,
                                    &device_count,
                                    NULL);
   if (VK_SUCCESS != res) {
      fprintf(stderr, "vulkan device enumeration failed (%s)",
                      vkresult_to_string(res));
      return -1;
   }

   if (device_count == 0) {
      fprintf(stderr, "No device supports Vulkan.\n");
      return -1;
   }

   vulkan_state->physical_devices = CALLOC(
      device_count, sizeof(*vulkan_state->physical_devices));

   if (vulkan_state->physical_devices == NULL) {
      return -1;
   }

   res = vkEnumeratePhysicalDevices(vulkan_state->vk_instance,
                                    &device_count,
                                    vulkan_state->physical_devices);
   if (VK_SUCCESS != res) {
      fprintf(stderr, "vulkan device enumeration failed (%s)", vkresult_to_string(res));
      return -1;
   }

   vulkan_state->physical_device_count = device_count;
   return 0;
}

int
virgl_vk_init(void)
{
   VkResult vk_res;
   VkApplicationInfo application_info = { 0 };
   VkInstanceCreateInfo info = { 0 };

   vulkan_state = CALLOC_STRUCT(virgl_vk);
   if (NULL == vulkan_state) {
      return -1;
   }

   application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   application_info.pApplicationName = "virglrenderer";
   application_info.applicationVersion = 1;
   application_info.pEngineName = NULL;
   application_info.engineVersion = 1;
   application_info.apiVersion = VK_MAKE_VERSION(1,1,0);

   const char *validation_layers[] = {
#ifdef DEBUG
      "VK_LAYER_LUNARG_core_validation",
      "VK_LAYER_LUNARG_object_tracker",
      "VK_LAYER_LUNARG_parameter_validation",
      "VK_LAYER_LUNARG_standard_validation",
#endif
   };

   info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   info.pApplicationInfo = &application_info;
   info.enabledLayerCount = ARRAY_SIZE(validation_layers);
   info.ppEnabledLayerNames = validation_layers;

   do {
      vk_res = vkCreateInstance(&info, NULL, &vulkan_state->vk_instance);
      if (VK_SUCCESS != vk_res) {
         fprintf(stderr, "Vk init failed (%s)\n", vkresult_to_string(vk_res));
         break;
      }

      if (0 != init_physical_devices()) {
         break;
      }

      /* success path */
      printf("Vulkan state created with %d devices.\n", vulkan_state->physical_device_count);
      return 0;
   } while (0);

   /* failure branch */
   virgl_vk_destroy();
   return -1;
}

void
virgl_vk_destroy(void)
{
   if (NULL == vulkan_state) {
      return;
   }

   if (VK_NULL_HANDLE != vulkan_state->vk_instance) {
      vkDestroyInstance(vulkan_state->vk_instance, NULL);
      vulkan_state->vk_instance = VK_NULL_HANDLE;
   }

   FREE(vulkan_state->devices);
   FREE(vulkan_state->physical_devices);
   FREE(vulkan_state);
   vulkan_state = NULL;
}
