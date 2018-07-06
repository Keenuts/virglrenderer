#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "util/macros.h"
#include "util/u_hash_table.h"
#include "util/u_pointer.h"
#include "virgl_vk.h"

int virgl_vk_get_device_count(uint32_t *device_count)
{
   TRACE_IN();

   *device_count = vk_info->physical_device_count;

   RETURN(0);
}

int virgl_vk_get_sparse_properties(uint32_t device_id,
                                   VkPhysicalDeviceSparseProperties *sparse_props)
{
   struct VkPhysicalDeviceProperties props;

   TRACE_IN();

   if (device_id >= vk_info->physical_device_count) {
      RETURN(-1);
   }

   vkGetPhysicalDeviceProperties(vk_info->physical_devices[device_id], &props);
   memcpy(sparse_props, &props.sparseProperties, sizeof(*sparse_props));

   RETURN(0);
}

static VkPhysicalDevice get_physical_device(uint32_t id)
{
   TRACE_IN();

   if (id >= vk_info->physical_device_count) {
      RETURN(VK_NULL_HANDLE);
   }

   RETURN(vk_info->physical_devices[id]);
}

int virgl_vk_get_queue_family_properties(uint32_t device_id,
                                         uint32_t *family_count,
                                         VkQueueFamilyProperties **props)
{
   VkPhysicalDevice dev;

   TRACE_IN();

   dev = get_physical_device(device_id);
   if (dev == VK_NULL_HANDLE) {
      RETURN(-1);
   }

   vkGetPhysicalDeviceQueueFamilyProperties(dev, family_count, NULL);

   *props = malloc(sizeof(VkQueueFamilyProperties) * *family_count);
   if (*props == NULL) {
      *family_count = 0;
      RETURN(-1);
   }

   vkGetPhysicalDeviceQueueFamilyProperties(dev, family_count, *props);

   RETURN(0);
}

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

   obj->cleanup_callback(obj->vk_device, obj->vk_handle, NULL);
   free(obj);
}

static struct vk_device* get_device_from_handle(uint32_t handle)
{
   uint32_t max_device = list_length(&vk_info->devices->list);
   if (handle >= max_device) {
      return NULL;
   }

   struct vk_device *it = NULL;
   LIST_FOR_EACH(it, vk_info->devices->list, list) {
      if (handle == 0)
         break;
      handle--;
   }

   return it;
}

static uint32_t
device_insert_object(struct vk_device *dev, void *vk_handle, void *callback)
{
   struct vk_object *object = NULL;

   object = malloc(sizeof(*object));
   if (NULL == object) {
      return 0;
   }

   object->vk_device = dev->vk_device;
   object->vk_handle = vk_handle;
   object->handle = dev->next_handle;
   object->cleanup_callback = callback;
   dev->next_handle += 1;

   util_hash_table_set(dev->objects, intptr_to_pointer(object->handle), object);
   return object->handle;
}

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

static void
device_remove_object(struct vk_device *dev, uint32_t handle)
{
   util_hash_table_remove(dev->objects, &handle);
}

static int initialize_vk_device(VkDevice dev,
                                const VkDeviceCreateInfo *info,
                                uint32_t *device_id)
{
   struct vk_device *device;
   const VkDeviceQueueCreateInfo *queue_info;

   TRACE_IN();

   device = calloc(1, sizeof(*device));
   if (device == NULL) {
      return -1;
   }

   *device_id = list_length(&vk_info->devices->list);

   list_init(&device->list);
   device->vk_device = dev;

   /* initializing device queues */
   device->queue_count = 0;
   for (uint32_t i = 0; i < info->queueCreateInfoCount; i++) {
      device->queue_count += info->pQueueCreateInfos[i].queueCount;
   }

   device->queues = malloc(sizeof(VkQueue) * device->queue_count);
   if (device->queues == NULL) {
      free(device);
      return -1;
   }

   uint32_t id = 0;
   for (uint32_t i = 0; i < info->queueCreateInfoCount; i++) {
      queue_info = info->pQueueCreateInfos + i;
      
      for (uint32_t j = 0; j < queue_info->queueCount; j++) {
         vkGetDeviceQueue(dev, queue_info->queueFamilyIndex, j, device->queues + id);
         id += 1;
      }
   }

   /* creating device resource maps */
   device->next_handle = 1;
   device->objects = util_hash_table_create(hash_func, vkobj_compare, vkobj_free);

   /* registering device */
   list_append(&vk_info->devices->list, &device->list);
   return 0;
}

int virgl_vk_create_device(uint32_t phys_device_id,
                           const VkDeviceCreateInfo *info,
                           uint32_t *device_id)
{
   TRACE_IN();

   VkDevice dev;
   VkResult res;
   VkPhysicalDevice physical_dev;
   
   physical_dev = get_physical_device(phys_device_id);
   if (physical_dev == VK_NULL_HANDLE) {
      RETURN(-1);
   }

   res = vkCreateDevice(physical_dev, info, NULL, &dev);
   if (res != VK_SUCCESS) {
      RETURN(-2);
   }

   if (initialize_vk_device(dev, info, device_id) < 0) {
      vkDestroyDevice(dev, NULL);
      RETURN(-3);
   }

   RETURN(0);
}

int virgl_vk_create_descriptor_set_layout(uint32_t device_id,
													   VkDescriptorSetLayoutCreateInfo *info,
													   uint32_t *handle)
{
   struct vk_device *device = NULL;
   VkDescriptorSetLayout *vk_handle = NULL;
   VkResult res;

	TRACE_IN();

   info->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

   device = get_device_from_handle(device_id);
   if (NULL == device) {
      RETURN(-1);
   }

   vk_handle = malloc(sizeof(*vk_handle));
   if (NULL == vk_handle) {
      RETURN(-2);
   }

   res = vkCreateDescriptorSetLayout(device->vk_device, info, NULL, vk_handle);
   if (VK_SUCCESS != res) {
      free(vk_handle);
      RETURN(-3);
   }

   *handle = device_insert_object(device, vk_handle, vkDestroyDescriptorSetLayout);
   if (*handle == 0) {
      free(vk_handle);
      RETURN(-4);
   }

   printf("%s: handle=%d\n", __func__, *handle);
   RETURN(0);
}

int virgl_vk_create_descriptor_pool(uint32_t device_id,
                                    VkDescriptorPoolCreateInfo *info,
                                    uint32_t *handle)
{
	TRACE_IN();

   struct vk_device *device = NULL;
   VkDescriptorPool *vk_handle = NULL;
   VkResult res;

   info->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   device = get_device_from_handle(device_id);
   if (NULL == device) {
      RETURN(-1);
   }

   vk_handle = malloc(sizeof(*vk_handle));
   if (NULL == vk_handle) {
      RETURN(-2);
   }

   res = vkCreateDescriptorPool(device->vk_device, info, NULL, vk_handle);
   if (VK_SUCCESS != res) {
      free(vk_handle);
      RETURN(-3);
   }

   *handle = device_insert_object(device, vk_handle, vkDestroyDescriptorPool);
   if (0 == *handle) {
      free(vk_handle);
      RETURN(-4);
   }

   printf("%s: handle=%d\n", __func__, *handle);
	RETURN(0);
}

int virgl_vk_allocate_descriptor_set(uint32_t device_handle,
                                     uint32_t pool_handle,
                                     uint32_t descriptor_count,
                                     uint32_t *desc_layout_ids,
                                     uint32_t *handles)
{
   TRACE_IN();
   struct vk_device *device = NULL;
   VkDescriptorPool *vk_pool;

   VkDescriptorSetLayout *vk_layouts = NULL;
   VkDescriptorSet *vk_sets = NULL;
   VkDescriptorSetAllocateInfo vk_info;
   VkResult res;

   device = get_device_from_handle(device_handle);
   vk_pool = device_get_object(device, pool_handle);
   if (NULL == device || NULL == vk_pool) {
      RETURN(-1);
   }

   vk_sets = malloc(sizeof(*vk_sets) * descriptor_count);
   vk_layouts = malloc(sizeof(*vk_layouts) * descriptor_count);
   if (NULL == vk_sets || NULL == vk_layouts) {
      free(vk_sets);
      free(vk_layouts);
      RETURN(-2);
   }

   for (uint32_t i = 0; i < descriptor_count; i++) {
      VkDescriptorSetLayout *ptr = device_get_object(device, desc_layout_ids[i]);
      if (NULL != ptr) {
         vk_layouts[i] = *ptr;
         continue;
      }

      free(vk_sets);
      free(vk_layouts);
      RETURN(-9);
   }

   vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   vk_info.pNext = NULL;
   vk_info.descriptorPool = *vk_pool;
   vk_info.descriptorSetCount = descriptor_count;
   vk_info.pSetLayouts = vk_layouts;

   res = vkAllocateDescriptorSets(device->vk_device,
                                  &vk_info,
                                  vk_sets);
   if (VK_SUCCESS != res) {
      free(vk_sets);
      RETURN(-3);
   }

   for (uint32_t i = 0; i < descriptor_count; i++) {
      handles[i] = device_insert_object(device, vk_sets + i, vkDestroyDescriptorPool);
      if (0 != handles[i]) {
         /* success path */
         continue;
      }

      /* If an error occured, clean-up old handles, and exit */
      for (uint32_t j = i; j > 0; j++) {
         device_remove_object(device, handles[j - 1]);
      }
      RETURN(-4);
   }

   RETURN(0);
}

int virgl_vk_create_buffer(uint32_t device_id,
								   VkBufferCreateInfo *info,
								   uint32_t *handle)
{
	TRACE_IN();

   UNUSED_PARAMETER(device_id);
   UNUSED_PARAMETER(info);
   UNUSED_PARAMETER(handle);

	puts("CREATING BUFFER");
	*handle = 1;
	RETURN(-1);
}

int virgl_vk_create_shader_module(uint32_t device_id,
                                  const VkShaderModuleCreateInfo *info,
                                  uint32_t *handle)
{
   TRACE_IN();

   UNUSED_PARAMETER(device_id);
   UNUSED_PARAMETER(info);
   UNUSED_PARAMETER(handle);

   puts("CREATING SHADER");

   RETURN(-1);
}
