#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "util/macros.h"
#include "virgl_vk.h"

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

static void check_vkresult(const char* fname, VkResult res)
{
   if (res == VK_SUCCESS) {
      fprintf(stderr, "\033[32m%s\033[0m\n", fname);
      return;
   }

   fprintf(stderr, "\033[31m%s = %s\033[0m\n", fname, vkresult_to_string(res));
   abort();
}

#define CALL_VK(Func, Param) check_vkresult(#Func, Func Param)

static int init_physical_devices(struct virgl_vk *state)
{
   TRACE_IN();

   uint32_t device_count;

   state->devices = malloc(sizeof(*state->devices));
   if (NULL == state->devices) {
      RETURN(-1);
   }

   list_init(&state->devices->list);

   CALL_VK(vkEnumeratePhysicalDevices, (state->vk_instance, &device_count, NULL));
   if (device_count == 0) {
      fprintf(stderr, "No device supports Vulkan.\n");
      RETURN(-1);
   }

   state->physical_devices = malloc(sizeof(*state->physical_devices) * device_count);
   if (state->physical_devices == NULL) {
      RETURN(-1);
   }

   CALL_VK(vkEnumeratePhysicalDevices, (state->vk_instance,
                                        &device_count,
                                        state->physical_devices));

   state->physical_device_count = device_count;
   RETURN(0);
}

struct virgl_vk* virgl_vk_init()
{
   struct virgl_vk *state = NULL;
   VkResult vk_res;
   VkApplicationInfo application_info = { 0 };
   VkInstanceCreateInfo info = { 0 };

   TRACE_IN();

   state = malloc(sizeof(*state));
   if (state == NULL) {
      RETURN(state);
   }

   application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   application_info.pApplicationName = "virglrenderer";
   application_info.applicationVersion = 1;
   application_info.pEngineName = "potatoe";
   application_info.engineVersion = 1;
   application_info.apiVersion = VK_MAKE_VERSION(1,1,0);

   const char *validation_layers[] = {
      //"VK_LAYER_LUNARG_standard_validation",
      //"VK_LAYER_LUNARG_parameter_validation",
      //"VK_LAYER_LUNARG_standard_validation",
      //"VK_LAYER_LUNARG_object_tracker",
      //"VK_LAYER_LUNARG_core_validation",
   };

   info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   info.pApplicationInfo = &application_info;
   info.enabledLayerCount = ARRAY_SIZE(validation_layers);
   info.ppEnabledLayerNames = validation_layers;

   do {
      vk_res = vkCreateInstance(&info, NULL, &state->vk_instance);
      if (vk_res != VK_SUCCESS) {
         fprintf(stderr, "Vk init failed (%s)\n", vkresult_to_string(vk_res));
         break;
      }

      if (init_physical_devices(state) < 0) {
         break;
      }

      /* success path */
      RETURN(state);
   } while (0);

   /* failure branch */
   virgl_vk_destroy(&state);
   RETURN(state);
}

void virgl_vk_destroy(struct virgl_vk **state)
{
   if ((*state)->vk_instance != VK_NULL_HANDLE) {
      vkDestroyInstance((*state)->vk_instance, NULL);
      (*state)->vk_instance = VK_NULL_HANDLE;
   }

   free((*state)->physical_devices);
   free(*state);
   *state = NULL;
}
