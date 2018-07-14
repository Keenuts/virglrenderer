#ifndef VIRGL_VK_H
#define VIRGL_VK_H

#include <vulkan/vulkan.h>
#include "util/list.h"

struct virgl_vk {
   VkInstance vk_instance;

   VkPhysicalDevice *physical_devices;
   uint32_t physical_device_count;

   struct vk_device *devices;
};

typedef struct vk_device {
   struct list list;

   uint32_t physical_device_id;
   VkDevice handle;

   uint32_t queue_count;
   VkQueue *queues;

   uint32_t next_handle;
   struct util_hash_table *objects;
} vk_device_t;


/* Vulkan objects are stored in these handles to keep some state
 * This default handle should never be used.
 * Concider using vk_XXX_t handles instead.
 */
struct vk_handle {
   void *content;
};

#define DECLARE_VK_HANDLE(Type, Name)  \
   typedef struct {                    \
      Type handle;                     \
   } vk_ ## Name ## _t;

DECLARE_VK_HANDLE(VkDescriptorPool, descriptor_pool);
DECLARE_VK_HANDLE(VkDescriptorSetLayout, descriptor_set_layout);
DECLARE_VK_HANDLE(VkDescriptorSet, descriptor_set);
DECLARE_VK_HANDLE(VkPipelineLayout, pipeline_layout);
DECLARE_VK_HANDLE(VkShaderModule, shader_module);
DECLARE_VK_HANDLE(VkPipeline, pipeline);
DECLARE_VK_HANDLE(VkBuffer, buffer);
DECLARE_VK_HANDLE(VkFence, fence);
DECLARE_VK_HANDLE(VkSemaphore, semaphore);

typedef struct {
   VkDeviceMemory handle;
   VkMemoryPropertyFlags flags;

   void *map_ptr;
   uint64_t map_size;
   uint64_t map_offset;
} vk_device_memory_t;

typedef struct {
   VkCommandPool handle;

   //FIXME: this array does not have to be contiguous
   VkCommandBuffer *cmds;
   uint32_t usage;
   uint32_t capacity;
} vk_command_pool_t;

/* An object stores in the hashtable */
struct vk_object {
   uint32_t handle;

   VkDevice vk_device;
   struct vk_handle *vk_handle;

   void (*cleanup_callback)(VkDevice, void*, void*);
};

extern struct virgl_vk *vk_info;
const char* vkresult_to_string(VkResult res);

struct virgl_vk* virgl_vk_init();
void virgl_vk_destroy(struct virgl_vk **state);

#endif
