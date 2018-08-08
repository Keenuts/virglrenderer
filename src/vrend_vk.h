#ifndef VIRGL_VK_H
#define VIRGL_VK_H

#include <vulkan/vulkan.h>
#include "util/u_double_list.h"

/* This struct contains the state of our Vulkan module
*
* vk_instance: one instance per virglrenderer process.
* physical_devices: contiguous array of VkPhysicalDevice*.
*                   Devices are enumerated on instance creation.
*
* devices: list of VkDevice wrappers. Each item in this list reprensents
*          a vulkan application using virglrenderer.
*/
struct vrend_vk {
   VkInstance vk_instance;

   VkPhysicalDevice *physical_devices;
   uint32_t physical_device_count;

   struct vk_device *devices;
   uint32_t device_count;
};

extern struct vrend_vk *vulkan_state;

/* This struct contains the state of ONE Vulkan application running using vgl
*
* physical_device_id: this is the index of the physical device in use
*                     (in the vrend_vk.physical_devices array)
* handle: The VkDevice handle
*
* Queue creation if forwarded. Thus, we store the VkQueue handles here.
* queue_count: number of queues created by the application
* queues: array of VkQueue handles
*
* next_handle: next handle the device will use when creating an object
* objects: this hashtable stores every Vulkan objects.
*          All objects are wrapped in a struct defined bellow.
*/
typedef struct vk_device {
   struct list_head list;

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

   /* For now, commanbuffer arrays are contiguous. It's not
    * needed, and having a large continguous array might cause issues.
    * SHOULD be canged
    */
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

/* converts a VkResult to the readable string */
const char* vkresult_to_string(VkResult res);

/* vulkan state management functions */
int vrend_vk_init(void);
void vrend_vk_destroy(void);

#endif
