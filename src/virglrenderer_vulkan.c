#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "util/u_hash_table.h"
#include "util/u_pointer.h"
#include "util/u_memory.h"
#include "vrend_vk.h"
#include "virglrenderer_vulkan.h"

/* functions used in the device's hashtable */

/* reusing the same function as vrend does.*/
static unsigned hash_func(void *key)
{
   intptr_t ip = pointer_to_intptr(key);
   return (unsigned)(ip & 0xffffffff);
}

static int vkobj_compare(void *a, void *b)
{
   if (a < b) {
      return -1;
   } else if (a > b) {
      return 1;
   }
   return 0;
}

static void vkobj_free(void *handle)
{
   struct vk_object *obj = handle;

   if (NULL != obj->cleanup_callback) {
      obj->cleanup_callback(obj->vk_device, obj->vk_handle->content, NULL);
   }

   free(obj);
}

/* helper functions */

/* Get a physical device from a VGL-HANDLE */
static VkPhysicalDevice
get_physical_device_from_handle(uint32_t handle)
{
   if (handle >= vulkan_state->physical_device_count) {
      return VK_NULL_HANDLE;
   }

   return vulkan_state->physical_devices[handle];
}

/* Get a logical device from a VGL-HANDLE */
static struct vk_device*
get_device_from_handle(uint32_t handle)
{
   struct vk_device *it = NULL;
   LIST_FOR_EACH_ENTRY(it, &vulkan_state->devices->list, list) {
      if (handle == 0)
         break;
      handle--;
   }

   return it;
}

/* Insert a vgl object into a logical device. Generates the VGL-HANDLE */
static uint32_t
device_insert_object(struct vk_device *dev, void *vk_handle, void *callback)
{
   struct vk_object *object = NULL;

   object = CALLOC_STRUCT(vk_object);
   if (NULL == object) {
      return 0;
   }

   object->vk_device = dev->handle;
   object->vk_handle = vk_handle;
   object->handle = dev->next_handle;
   object->cleanup_callback = callback;
   dev->next_handle += 1;

   util_hash_table_set(dev->objects, intptr_to_pointer(object->handle), object);
   return object->handle;
}

/* get an vgl-object from the hashtable using a VGL-HANDLE */
static void*
device_get_object(struct vk_device *dev, uint32_t handle)
{
   struct vk_object *object = NULL;
   object = util_hash_table_get(dev->objects, intptr_to_pointer(handle));
   if (NULL == object) {
      return NULL;
   }

   return object->vk_handle;
}

/* removes an object from the hashtable */
/* cleanup is done by the hashtable */
static void
device_remove_object(struct vk_device *dev, uint32_t handle)
{
   util_hash_table_remove(dev->objects, &handle);
}


/* Helper to create and insert a simple object
 * object creation has to follow the protptype:
 *    VkResult create_func(VkDevice, const VkXXXInfo *, const VkAlloc...);
 *
 *    The object is inserted and the handle generated on success
 */
typedef VkResult (*PFN_vkCreateFunction)(VkDevice,
                                         const void*,
                                         const VkAllocationCallbacks*,
                                         void*);
typedef void (*PFN_vkDestroyFunction)(VkDevice, void*, const VkAllocationCallbacks*);

static int create_simple_object(uint32_t device_id,
                                const void* create_info,
                                PFN_vkCreateFunction create_func,
                                PFN_vkDestroyFunction destroy_func,
                                size_t vk_handle_size,
                                uint32_t *handle)
{
   struct vk_device *device = NULL;
   struct vk_handle *vk_handle = NULL;
   VkResult res;

   device = get_device_from_handle(device_id);
   if (NULL == device) {
      return -1;
   }

   vk_handle = CALLOC(1, vk_handle_size);
   if (NULL == vk_handle) {
      return -2;
   }

   res = create_func(device->handle, create_info, NULL, &vk_handle->content);
   if (VK_SUCCESS != res) {
      fprintf(stderr, "vk_call_failed %s\n", vkresult_to_string(res));
      free(vk_handle);
      return -3;
   }

   *handle = device_insert_object(device, vk_handle, destroy_func);
   if (0 == *handle) {
      destroy_func(device->handle, vk_handle->content, NULL);
      free(vk_handle);
      return -4;
   }

	return 0;
}

static int
device_initialize_queues(uint32_t phys_device_handle,
                         VkDevice vk_device,
                         const VkDeviceCreateInfo *info,
                         struct vk_device *device)
{
   const VkDeviceQueueCreateInfo *queue_info;

   device->queue_count = 0;
   for (uint32_t i = 0; i < info->queueCreateInfoCount; i++) {
      device->queue_count += info->pQueueCreateInfos[i].queueCount;
   }

   device->queues = CALLOC(device->queue_count, sizeof(VkQueue));
   if (device->queues == NULL) {
      return -1;
   }

   uint32_t id = 0;
   for (uint32_t i = 0; i < info->queueCreateInfoCount; i++) {
      queue_info = info->pQueueCreateInfos + i;

      for (uint32_t j = 0; j < queue_info->queueCount; j++) {
         vkGetDeviceQueue(vk_device,
                          queue_info->queueFamilyIndex,
                          j,
                          device->queues + id);
         id += 1;
      }
   }

   return 0;
};

static int
device_initialize_command_pools(struct vk_device *device)
{
   return 0;
}

static int
initialize_vk_device(uint32_t phys_device_handle,
                     VkDevice dev,
                     const VkDeviceCreateInfo *info,
                     uint32_t *device_id)
{
   struct vk_device *device;
   device = CALLOC_STRUCT(vk_device);
   if (device == NULL) {
      return -1;
   }

   *device_id = vulkan_state->device_count;
   vulkan_state->device_count++;

   LIST_INITHEAD(&device->list);
   device->handle = dev;
   device->physical_device_id = phys_device_handle;

   /* initializing device queues */
   if (0 != device_initialize_queues(phys_device_handle, dev, info, device)) {
      free(device);
      return -2;
   }

   if (0 != device_initialize_command_pools(device)) {
      free(device);
      return -3;
   }

   /* creating device resource maps */
   device->next_handle = 1;
   device->objects = util_hash_table_create(hash_func,
                                            vkobj_compare,
                                            vkobj_free);

   /* registering device */
   LIST_ADDTAIL(&vulkan_state->devices->list, &device->list);
   return 0;
}


int virgl_vk_get_device_count(uint32_t *device_count)
{
   *device_count = vulkan_state->physical_device_count;

   return 0;
}

int virgl_vk_get_sparse_properties(uint32_t device_id,
                                   VkPhysicalDeviceSparseProperties *sparse_props)
{
   VkPhysicalDeviceProperties props;

   if (device_id >= vulkan_state->physical_device_count) {
      return -1;
   }

   vkGetPhysicalDeviceProperties(vulkan_state->physical_devices[device_id], &props);
   memcpy(sparse_props, &props.sparseProperties, sizeof(*sparse_props));

   return 0;
}

int virgl_vk_get_memory_properties(uint32_t phys_device_handle,
                                   VkPhysicalDeviceMemoryProperties *props)
{
   if (phys_device_handle >= vulkan_state->physical_device_count) {
      return -1;
   }

   memset(props, 0, sizeof(*props));
   vkGetPhysicalDeviceMemoryProperties(vulkan_state->physical_devices[phys_device_handle],
                                       props);

   return 0;
}

int virgl_vk_get_queue_family_properties(uint32_t device_id,
                                         uint32_t *family_count,
                                         VkQueueFamilyProperties **props)
{
   VkPhysicalDevice dev;

   dev = get_physical_device_from_handle(device_id);
   if (dev == VK_NULL_HANDLE) {
      return -1;
   }

   vkGetPhysicalDeviceQueueFamilyProperties(dev, family_count, NULL);

   *props = CALLOC(*family_count, sizeof(VkQueueFamilyProperties));
   if (*props == NULL) {
      *family_count = 0;
      return -1;
   }

   vkGetPhysicalDeviceQueueFamilyProperties(dev, family_count, *props);

   return 0;
}

int virgl_vk_create_device(uint32_t phys_device_id,
                           const VkDeviceCreateInfo *info,
                           uint32_t *device_id)
{
   VkDevice dev;
   VkResult res;
   VkPhysicalDevice physical_dev;

   physical_dev = get_physical_device_from_handle(phys_device_id);
   if (physical_dev == VK_NULL_HANDLE) {
      return -1;
   }

   res = vkCreateDevice(physical_dev, info, NULL, &dev);
   if (res != VK_SUCCESS) {
      return -2;
   }

   if (initialize_vk_device(phys_device_id, dev, info, device_id) < 0) {
      vkDestroyDevice(dev, NULL);
      return -3;
   }

   return 0;
}

int virgl_vk_allocate_descriptor_set(uint32_t device_handle,
                                     uint32_t pool_handle,
                                     uint32_t descriptor_count,
                                     uint32_t *desc_layout_ids,
                                     uint32_t *handles)
{
   vk_device_t *device = NULL;
   vk_descriptor_pool_t *pool = NULL;
   vk_descriptor_set_layout_t *layout = NULL;
   vk_descriptor_set_t *sets= NULL;
   VkDescriptorSet *vk_sets = NULL;
   VkDescriptorSetLayout *vk_layouts = NULL;

   VkDescriptorSetAllocateInfo vk_info;
   VkResult res;

   device = get_device_from_handle(device_handle);
   pool = device_get_object(device, pool_handle);
   if (NULL == device || NULL == pool) {
      return -1;
   }

   vk_layouts = alloca(sizeof(*vk_layouts) * descriptor_count);
   vk_sets = alloca(sizeof(*vk_sets) * descriptor_count);

   sets = CALLOC(descriptor_count, sizeof(*sets));
   if (NULL == sets) {
      free(sets);
      return -2;
   }

   for (uint32_t i = 0; i < descriptor_count; i++) {
      layout = device_get_object(device, desc_layout_ids[i]);
      if (NULL != layout) {
         vk_layouts[i] = layout->handle;
         continue;
      }

      free(sets);
      return -3;
   }

   vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   vk_info.pNext = NULL;
   vk_info.descriptorPool = pool->handle;
   vk_info.descriptorSetCount = descriptor_count;
   vk_info.pSetLayouts = vk_layouts;

   res = vkAllocateDescriptorSets(device->handle, &vk_info, vk_sets);
   if (VK_SUCCESS != res) {
      free(sets);
      return -4;
   }

   for (uint32_t i = 0; i < descriptor_count; i++) {
      sets[i].handle = vk_sets[i];
      handles[i] = device_insert_object(device, sets + i, NULL);
      if (0 != handles[i]) {
         /* success path */
         continue;
      }

      /* If an error occured, clean-up old handles, and exit */
      for (uint32_t j = i; j > 0; j++) {
         vkFreeDescriptorSets(device->handle, pool->handle, 1, &sets[i].handle);
         device_remove_object(device, handles[j - 1]);
      }
      return -5;
   }

   return 0;
}

int virgl_vk_create_descriptor_pool(uint32_t device_handle,
                                    const VkDescriptorPoolCreateInfo *info,
                                    uint32_t *handle)
{
   int res = create_simple_object(device_handle,
                                  info,
                                  (PFN_vkCreateFunction)vkCreateDescriptorPool,
                                  (PFN_vkDestroyFunction)vkDestroyDescriptorPool,
                                  sizeof(vk_descriptor_pool_t),
                                  handle);
   return res;
}

int virgl_vk_create_descriptor_set_layout(uint32_t device_handle,
													   VkDescriptorSetLayoutCreateInfo *info,
													   uint32_t *handle)
{
   int res = create_simple_object(device_handle,
                                  info,
                                  (PFN_vkCreateFunction)vkCreateDescriptorSetLayout,
                                  (PFN_vkDestroyFunction)vkDestroyDescriptorSetLayout,
                                  sizeof(vk_descriptor_set_layout_t),
                                  handle);
   return res;
}

int virgl_vk_create_shader_module(uint32_t device_handle,
                                  const VkShaderModuleCreateInfo *info,
                                  uint32_t *handle)
{
   int res = create_simple_object(device_handle,
                                  info,
                                  (PFN_vkCreateFunction)vkCreateShaderModule,
                                  (PFN_vkDestroyFunction)vkDestroyShaderModule,
                                  sizeof(vk_shader_module_t),
                                  handle);
   return res;
}

int virgl_vk_create_pipeline_layout(uint32_t device_handle,
                                    VkPipelineLayoutCreateInfo *info,
                                    uint32_t *set_handles,
                                    uint32_t *handle)
{
   VkDescriptorSetLayout *vk_layouts = NULL;
   vk_descriptor_set_layout_t *layout = NULL;
   struct vk_device *device = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   vk_layouts = alloca(sizeof(*vk_layouts) * info->setLayoutCount);
   for (uint32_t i = 0; i < info->setLayoutCount; i++) {
      layout = device_get_object(device, set_handles[i]);
      if (NULL == layout) {
         return -1;
      }
      vk_layouts[i] = layout->handle;
   }

   info->pSetLayouts = vk_layouts;

   int res = create_simple_object(device_handle,
                                  info,
                                  (PFN_vkCreateFunction)vkCreatePipelineLayout,
                                  (PFN_vkDestroyFunction)vkDestroyPipelineLayout,
                                  sizeof(vk_pipeline_layout_t),
                                  handle);
   return res;
}

int virgl_vk_create_compute_pipelines(uint32_t device_handle,
                                      VkComputePipelineCreateInfo *info,
                                      uint32_t layout_handle,
                                      uint32_t module_handle,
                                      uint32_t *handle)
{
   struct vk_device *device = NULL;
   vk_pipeline_layout_t *pipeline_layout = NULL;
   vk_shader_module_t *shader_module = NULL;

   vk_pipeline_t *pipeline = NULL;
   VkResult res;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   pipeline_layout = device_get_object(device, layout_handle);
   shader_module = device_get_object(device, module_handle);
   if (NULL == pipeline_layout || NULL == shader_module) {
      return -1;
   }

   pipeline = CALLOC(1, sizeof(*pipeline));
   if (NULL == pipeline) {
      return -2;
   }

   info->layout = pipeline_layout->handle;
   info->stage.module = shader_module->handle;

   res = vkCreateComputePipelines(device->handle,
                                  VK_NULL_HANDLE,
                                  1,
                                  info,
                                  NULL,
                                  &pipeline->handle);
   if (VK_SUCCESS != res) {
      free(pipeline);
      return -3;
   }

   *handle = device_insert_object(device, pipeline, vkDestroyPipeline);
   if (0 == *handle) {
      vkDestroyPipeline(device->handle, pipeline->handle, NULL);
      free(pipeline);
   }

   return 0;
}

int virgl_vk_allocate_memory(uint32_t device_handle,
                             const VkMemoryAllocateInfo *info,
                             uint32_t *output)
{
   int res;
   vk_device_t *device;
   vk_device_memory_t *memory = NULL;
   VkPhysicalDeviceMemoryProperties props;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   res = virgl_vk_get_memory_properties(device->physical_device_id, &props);
   if (0 > res) {
      return res;
   }

   res = create_simple_object(device_handle,
                              info,
                              (PFN_vkCreateFunction)vkAllocateMemory,
                              (PFN_vkDestroyFunction)vkFreeMemory,
                              sizeof(vk_device_memory_t),
                              output);
   if (0 > res) {
      return res;
   }

   memory = device_get_object(device, *output);
   if (unlikely(NULL == memory)) {
      return -1;
   }

   memory->flags = props.memoryTypes[info->memoryTypeIndex].propertyFlags;

   return res;
}

int virgl_vk_create_buffer(uint32_t device_handle,
                           VkBufferCreateInfo *info,
                           uint32_t *handle)
{
   int res = create_simple_object(device_handle,
                                  info,
                                  (PFN_vkCreateFunction)vkCreateBuffer,
                                  (PFN_vkDestroyFunction)vkDestroyBuffer,
                                  sizeof(vk_buffer_t),
                                  handle);
   return res;
}

int virgl_vk_bind_buffer_memory(uint32_t device_handle,
                                uint32_t buffer_handle,
                                uint32_t memory_handle,
                                uint64_t offset)
{
   VkResult res;
   struct vk_device *device = NULL;
   vk_buffer_t *buffer = NULL;
   vk_device_memory_t *memory = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   buffer = device_get_object(device, buffer_handle);
   memory = device_get_object(device, memory_handle);
   if (NULL == buffer || NULL == memory) {
      return -2;
   }

   res = vkBindBufferMemory(device->handle, buffer->handle, memory->handle, offset);
   if (VK_SUCCESS != res) {
      return -3;
   }

   return 0;
}

int virgl_vk_write_descriptor_set(uint32_t device_handle,
                                  VkWriteDescriptorSet *write_info,
                                  VkDescriptorBufferInfo *buffer_info,
                                  uint32_t descriptor_handle,
                                  uint32_t *buffer_handles)
{
   struct vk_device *device = NULL;
   vk_descriptor_set_t *set = NULL;
   vk_buffer_t *buffer = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   set = device_get_object(device, descriptor_handle);
   if (NULL == set) {
      return -2;
   }

   for (uint32_t i = 0; i < write_info->descriptorCount; i++) {
      buffer = device_get_object(device, buffer_handles[i]);
      if (NULL == buffer) {
         return -2;
      }

      buffer_info[i].buffer = buffer->handle;
   }

   write_info->dstSet = set->handle;
   write_info->pBufferInfo = buffer_info;

   vkUpdateDescriptorSets(device->handle, 1, write_info, 0, NULL);
   return 0;
}

int virgl_vk_is_memory_cached(uint32_t device_handle,
                              uint32_t memory_handle,
                              uint8_t *output)
{
   vk_device_t *device = NULL;
   vk_device_memory_t *memory = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   memory = device_get_object(device, memory_handle);
   if (NULL == memory) {
      return -2;
   }

   *output = (memory->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0;
   return 0;
}

int virgl_vk_invalidate_memory(uint32_t device_handle,
                               uint32_t memory_handle)
{
   VkMappedMemoryRange range;
   VkResult res;
   vk_device_t *device = NULL;
   vk_device_memory_t *memory = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   memory = device_get_object(device, memory_handle);
   if (NULL == memory || memory->map_ptr == NULL) {
      return -2;
   }

   range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
   range.pNext = NULL;
   range.memory = memory->handle;
   range.offset = memory->map_offset;
   range.size = memory->map_size;

   res = vkInvalidateMappedMemoryRanges(device->handle, 1, &range);
   if (VK_SUCCESS != res) {
      return -3;
   }

   return 0;
}

int virgl_vk_flush_memory(uint32_t device_handle,
                          uint32_t memory_handle)
{
   VkMappedMemoryRange range;
   VkResult res;
   vk_device_t *device = NULL;
   vk_device_memory_t *memory = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   memory = device_get_object(device, memory_handle);
   if (NULL == memory || memory->map_ptr == NULL) {
      return -2;
   }

   range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
   range.pNext = NULL;
   range.memory = memory->handle;
   range.offset = memory->map_offset;
   range.size = memory->map_size;

   res = vkFlushMappedMemoryRanges(device->handle, 1, &range);
   if (VK_SUCCESS != res) {
      return -3;
   }

   return 0;
}

int virgl_vk_map_memory(uint32_t device_handle,
                        uint32_t memory_handle,
                        uint32_t offset,
                        uint32_t size,
                        void **ptr)
{
   VkResult res;
   vk_device_t *device = NULL;
   vk_device_memory_t *memory = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   memory = device_get_object(device, memory_handle);
   if (NULL == memory) {
      return -2;
   }

   res = vkMapMemory(device->handle, memory->handle, offset, size, 0, ptr);
   if (VK_SUCCESS != res) {
      return -3;
   }

   memory->map_size = size;
   memory->map_offset = offset;
   memory->map_ptr = ptr;

   return 0;
}

int virgl_vk_unmap_memory(uint32_t device_handle,
                          uint32_t memory_handle)
{
   vk_device_t *device = NULL;
   vk_device_memory_t *memory = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   memory = device_get_object(device, memory_handle);
   if (NULL == memory) {
      return -2;
   }

   vkUnmapMemory(device->handle, memory->handle);

   memory->map_ptr = NULL;
   return 0;
}

int virgl_vk_create_command_pool(uint32_t device_handle,
                                 const VkCommandPoolCreateInfo *info,
                                 uint32_t *handle)
{
   VkResult res;
   vk_device_t *device = NULL;
   vk_command_pool_t *pool = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   pool = CALLOC(1, sizeof(*pool));
   if (NULL == pool) {
      return -3;
   }

   res = vkCreateCommandPool(device->handle,
                             info,
                             NULL,
                             &pool->handle);
   if (VK_SUCCESS != res) {
      free(pool);
      return -4;
   }

   *handle = device_insert_object(device, pool, vkDestroyCommandPool);
   if (0 == *handle) {
      free(pool);
      return -5;
   }

   return res;
}

static int
virgl_vk_command_pool_allocate_buffers(vk_device_t *device,
                                       vk_command_pool_t *pool,
                                       VkCommandBufferAllocateInfo *info,
                                       uint32_t *handles)
{
   VkResult res;
   uint32_t count = info->commandBufferCount;

   if (pool->capacity - pool->usage < count) {
      // we don't allocate exactly what we need, no particular reason */
      pool->cmds = REALLOC(pool->cmds,
                           sizeof(*pool->cmds) * pool->capacity,
                           sizeof(*pool->cmds) * (pool->capacity + count));
      if (NULL == pool->cmds) {
         fprintf(stderr, "cmd pool reallocation failed. good luck.\n");
         return -1;
      }

      pool->capacity += count;
      memset(pool->cmds + pool->usage, 0, sizeof(*pool->cmds) * count);
   }

   res = vkAllocateCommandBuffers(device->handle, info, pool->cmds + pool->usage);
   if (VK_SUCCESS != res) {
      return -2;
   }

   for (uint32_t i = 0; i < count; i++) {
      /* 0 is an invalid handle */
      handles[i] = pool->usage + i + 1;
   }
   pool->usage += count;

   return 0;
}

int virgl_vk_allocate_command_buffers(uint32_t device_handle,
                                      uint32_t pool_handle,
                                      VkCommandBufferAllocateInfo *info,
                                      uint32_t *handles)
{
   int res;
   vk_device_t *device = NULL;
   vk_command_pool_t *pool = NULL;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   pool = device_get_object(device, pool_handle);
   if (NULL == pool) {
      return -2;
   }

   info->commandPool = pool->handle;
   res = virgl_vk_command_pool_allocate_buffers(device, pool, info, handles);
   if (0 > res) {
      return -3;
   }

   return 0;
}

/* NOTE:
 *  for now, this function only handles compute operations.
 */
int virgl_vk_record_command(uint32_t device_handle,
                            const struct virgl_vk_record_info *info)
{
   vk_device_t *device = NULL;
   vk_command_pool_t *pool = NULL;
   vk_pipeline_t *pipeline = NULL;
   vk_pipeline_layout_t *pipeline_layout = NULL;
   vk_descriptor_set_t *set = NULL;

   VkCommandBuffer cmd = VK_NULL_HANDLE;
   VkDescriptorSet *descriptors = NULL;
   VkResult res;
   VkCommandBufferBeginInfo begin_info = { 0 };


   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   pool = device_get_object(device, info->pool_handle);
   pipeline = device_get_object(device, info->pipeline_handle);
   pipeline_layout = device_get_object(device, info->pipeline_layout_handle);
   if (NULL == pipeline || NULL == pipeline_layout || NULL == pool) {
      return -2;
   }

   descriptors = alloca(sizeof(*descriptors) * info->descriptor_count);
   for (uint32_t i = 0; i < info->descriptor_count; i++) {
      set = device_get_object(device, info->descriptor_handles[i]);
      if (NULL == set) {
         return -2;
      }
      descriptors[i] = set->handle;
   }

   if (pool->usage < info->cmd_handle) {
      return -2;
   }
   /* 0 is an invalid handles. entry = handle - 1 */
   cmd = pool->cmds[info->cmd_handle - 1];
   if (VK_NULL_HANDLE == cmd) {
      return -2;
   }

   begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   //FIXME: get type from guest
   begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   res = vkBeginCommandBuffer(cmd, &begin_info);
   if (VK_SUCCESS != res) {
      return -4;
   }

   vkCmdBindPipeline(cmd, info->bind_point, pipeline->handle);
   vkCmdBindDescriptorSets(cmd,
                           info->bind_point,
                           pipeline_layout->handle,
                           0,
                           info->descriptor_count,
                           descriptors,
                           0,
                           NULL);
   vkCmdDispatch(cmd,
                 info->dispatch_size[0],
                 info->dispatch_size[1],
                 info->dispatch_size[2]);
   res = vkEndCommandBuffer(cmd);
   if (VK_SUCCESS != res) {
      return -4;
   }

   return 0;
}

int virgl_vk_create_fence(uint32_t device_handle,
                          VkFenceCreateInfo *info,
                          uint32_t *handle)
{
   int res = create_simple_object(device_handle,
                                  info,
                                  (PFN_vkCreateFunction)vkCreateFence,
                                  (PFN_vkDestroyFunction)vkDestroyFence,
                                  sizeof(vk_fence_t),
                                  handle);
   return res;
}

int virgl_vk_wait_for_fences(uint32_t device_handle,
                             uint32_t fence_count,
                             uint32_t *handles,
                             uint32_t wait_all,
                             uint64_t timeout)
{
   vk_device_t *device = NULL;
   vk_fence_t *fence = NULL;
   VkFence *fences = NULL;
   VkResult res;

   device = get_device_from_handle(device_handle);
   if (NULL == device) {
      return -1;
   }

   fences = alloca(sizeof(*fences) * fence_count);
   for (uint32_t i = 0; i < fence_count; i++) {
      fence = device_get_object(device, handles[i]);
      if (NULL == fence) {
         return -2;
      }

      fences[i] = fence->handle;
   }

   res = vkWaitForFences(device->handle, fence_count, fences, wait_all, timeout);
   return res;
}

int virgl_vk_queue_submit(const struct virgl_vk_submit_info *info)
{
   vk_device_t *vk_device = NULL;
   vk_fence_t *vk_fence = NULL;
   vk_command_pool_t *vk_pool = NULL;
   vk_semaphore_t *vk_semaphore = NULL;
   VkFence fence = VK_NULL_HANDLE;
   VkSubmitInfo vk_info = { 0 };
   VkQueue queue;

   VkSemaphore *wait_s = NULL;
   VkSemaphore *signal_s = NULL;
   VkCommandBuffer *cmds = NULL;
   VkPipelineStageFlags *flags = NULL;

   vk_device = get_device_from_handle(info->device_handle);
   if (NULL == vk_device) {
      return -1;
   }

   if (info->queue_handle >= vk_device->queue_count) {
      return -2;
   }
   queue = vk_device->queues[info->queue_handle];

   if (info->fence_handle != 0) {
      vk_fence = device_get_object(vk_device, info->fence_handle);
      if (NULL == vk_fence) {
         return -2;
      }
      fence = vk_fence->handle;
   }

   vk_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   vk_info.waitSemaphoreCount = info->wait_count;
   wait_s = alloca(sizeof(VkSemaphore) * info->wait_count);
   flags = alloca(sizeof(VkPipelineStageFlags) * info->wait_count);

   for (uint32_t i = 0; i < info->wait_count; i++) {
      vk_semaphore = device_get_object(vk_device, info->wait_handles[i]);
      if (NULL == vk_semaphore) {
         return -2;
      }
      wait_s[i] = vk_semaphore->handle;
      flags[i] = info->wait_stage_masks[i];
   }

   vk_info.commandBufferCount = info->cmd_count;
   cmds = alloca(sizeof(VkCommandBuffer) * info->cmd_count);
   for (uint32_t i = 0; i < info->cmd_count; i++) {
      vk_pool = device_get_object(vk_device, info->pool_handles[i]);
      if (NULL == vk_pool) {
         return -2;
      }

      /* remember: handle 0 is invalid. Thus we need to substract one */
      if (0 == info->cmd_handles[i] || info->cmd_handles[i] - 1 >= vk_pool->usage) {
         return -2;
      }
      cmds[i] = vk_pool->cmds[info->cmd_handles[i] - 1];
   }

   vk_info.signalSemaphoreCount = info->signal_count;
   signal_s = alloca(sizeof(VkSemaphore) * info->signal_count);
   for (uint32_t i = 0; i < info->signal_count; i++) {
      vk_semaphore = device_get_object(vk_device, info->signal_handles[i]);
      if (NULL == vk_semaphore) {
         return -2;
      }
      signal_s[i] = vk_semaphore->handle;
   }

   vk_info.pWaitSemaphores = wait_s;
   vk_info.pWaitDstStageMask = flags;
   vk_info.pCommandBuffers = cmds;
   vk_info.pSignalSemaphores = signal_s;

   return vkQueueSubmit(queue, 1, &vk_info, fence);
}

int
virgl_vk_destroy_object(uint32_t device_handle,
                        uint32_t handle)
{
   vk_device_t *vk_device = NULL;
   vk_device = get_device_from_handle(device_handle);
   if (NULL == vk_device) {
      return -1;
   }

   util_hash_table_remove(vk_device->objects, intptr_to_pointer(handle));
   return 0;
}

static void
deinit_device(vk_device_t *device)
{
   util_hash_table_destroy(device->objects);

   /* About descriptor set layouts:
    *   This issue is pretty much the same as the commands one.
    *   A descriptor is linked to a pool.
    *   Right now, the object allocation mechanism is too rudimentary.
    *   I didn't took these kind of objects into concideration.
    *
    *   This should be taken into account for the next iteration
    */
   fprintf(stderr, "FIXME: descriptor layouts have not been cleaned-up.\n");
}

int
virgl_vk_destroy_device(uint32_t device_handle)
{
   vk_device_t *it = NULL;
   vk_device_t *storage = NULL;

   LIST_FOR_EACH_ENTRY_SAFE(it, storage, &vulkan_state->devices->list, list) {
      if (0 == device_handle) {

         LIST_DEL(&it->list);
         deinit_device(it);
         free(it);

         return 0;
      }
   }

   return -1;
}
