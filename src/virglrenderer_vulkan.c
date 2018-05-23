#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

struct symbol {
   const char *name;
   int hash;
   void *addr;
};

struct symtab {
   void *lib_handle;
   uint64_t used;
   uint64_t capacity;
   struct symbol *symbols;
};

struct symtab _vk_symbols = { 0 };

static unsigned long hash_symbol(const unsigned char *name)
{
   unsigned long h = 0;
   unsigned long g;

   while (*name) {
      h = (h << 4) + *name++;
      if ((g = h & 0xf0000000)) {
         h ^= g >> 24;
      }
      h &= ~g;
   }

   return h;
}

static void* lookup_symbol(const char *name)
{
   unsigned long hash = hash_symbol(name);

   for (uint64_t i = 0; i < _vk_symbols.used; i++) {
      if (_vk_symbols.symbols[i].hash != hash) {
         continue;
      }

      if (strcmp(_vk_symbols.symbols[i].name, name)) {
          continue;
      }
      return _vk_symbols.symbols[i].addr;
   }

   if (_vk_symbols.used >= _vk_symbols.capacity) {
      _vk_symbols.capacity *= 2;
      _vk_symbols.symbols = realloc(_vk_symbols.symbols,
                                    sizeof(*_vk_symbols.symbols) *
                                    _vk_symbols.capacity);
      assert(_vk_symbols.symbols);
   }

   void *addr = dlsym(_vk_symbols.lib_handle, name);
   assert(addr);

   _vk_symbols.symbols[_vk_symbols.used].hash = hash;
   _vk_symbols.symbols[_vk_symbols.used].addr = addr;
   _vk_symbols.symbols[_vk_symbols.used].name = strdup(name);
   _vk_symbols.used++;

   return addr;
}

static void* load_vulkan_function(const char* name)
{
   if (_vk_symbols.capacity == 0) {
      _vk_symbols.lib_handle = dlopen("/usr/lib/libvulkan.so", RTLD_LAZY);
      assert(_vk_symbols.lib_handle);
      _vk_symbols.capacity = 1;
      _vk_symbols.symbols = malloc(sizeof(*_vk_symbols.symbols));
   }

   return lookup_symbol(name);
}

#define VK_CALL(Fname, ...) \
   ((PFN_ ## Fname)load_vulkan_function(#Fname))(__VA_ARGS__)

VkResult virgl_vk_create_instance(
      const VkInstanceCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkInstance *pInstance)
{
   return VK_CALL(vkCreateInstance, pCreateInfo, pAllocator, pInstance);
}

VkResult virgl_vk_enumerate_instance_layer_properties(
      uint32_t *pPropertyCount,
      VkLayerProperties *pProperties)
{
   return VK_CALL(vkEnumerateInstanceLayerProperties, pPropertyCount, pProperties);
}

VkResult virgl_vk_enumerate_physical_devices(VkInstance instance,
                                             uint32_t* pPhysicalDeviceCount,
                                             VkPhysicalDevice* pPhysicalDevices)
{
   return VK_CALL(vkEnumeratePhysicalDevices, instance, pPhysicalDeviceCount,
                  pPhysicalDevices);
}

void virgl_vk_get_physical_device_queue_family_properties(
    VkPhysicalDevice physicalDevice,
    uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties* pQueueFamilyProperties)
{
    VK_CALL(vkGetPhysicalDeviceQueueFamilyProperties,
            physicalDevice,
            pQueueFamilyPropertyCount,
            pQueueFamilyProperties
    );
}

VkResult virgl_vk_create_device(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDevice *pDevice)
{
    return VK_CALL(vkCreateDevice,
        physicalDevice,
        pCreateInfo,
        pAllocator,
        pDevice
    );
}

void virgl_vk_get_device_queue(
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    VkQueue *pQueue)
{
    VK_CALL(vkGetDeviceQueue,
        device,
        queueFamilyIndex,
        queueIndex,
        pQueue
    );
}

VkResult virgl_vk_create_descriptor_set_layout(
    VkDevice device,
    const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDescriptorSetLayout *pSetLayout)
{
    return VK_CALL(vkCreateDescriptorSetLayout,
        device,
        pCreateInfo,
        pAllocator,
        pSetLayout
    );
}

VkResult virgl_vk_create_descriptor_pool(
    VkDevice device,
    const VkDescriptorPoolCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDescriptorPool *pDescriptorPool)
{
    return VK_CALL(vkCreateDescriptorPool,
        device,
        pCreateInfo,
        pAllocator,
        pDescriptorPool
    );
}

VkResult virgl_vk_create_command_pool(
    VkDevice device,
    const VkCommandPoolCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkCommandPool *pCommandPool)
{
    return VK_CALL(vkCreateCommandPool,
        device,
        pCreateInfo,
        pAllocator,
        pCommandPool
    );
}

VkResult virgl_vk_allocate_descriptor_sets(
    VkDevice device,
    const VkDescriptorSetAllocateInfo *pAllocateInfo,
    VkDescriptorSet *pDescriptorSets)
{
    return VK_CALL(vkAllocateDescriptorSets,
        device,
        pAllocateInfo,
        pDescriptorSets
    );
}

void virgl_vk_update_descriptor_sets(
    VkDevice device,
    uint32_t descriptorWriteCount,
    const VkWriteDescriptorSet *pDescriptorWrites,
    uint32_t descriptorCopyCount,
    const VkCopyDescriptorSet *pDescriptorCopies)
{
    VK_CALL(vkUpdateDescriptorSets,
        device,
        descriptorWriteCount,
        pDescriptorWrites,
        descriptorCopyCount,
        pDescriptorCopies
    );
}

VkResult virgl_vk_allocate_command_buffers(
    VkDevice device,
    const VkCommandBufferAllocateInfo *pAllocateInfo,
    VkCommandBuffer *pCommandBuffers)
{
    return VK_CALL(vkAllocateCommandBuffers,
        device,
        pAllocateInfo,
        pCommandBuffers
    );
}

VkResult virgl_vk_allocate_memory(
    VkDevice device,
    const VkMemoryAllocateInfo *pAllocateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDeviceMemory *pMemory)
{
    return VK_CALL(vkAllocateMemory,
        device,
        pAllocateInfo,
        pAllocator,
        pMemory
    );
}

VkResult virgl_vk_begin_command_buffer(
    VkCommandBuffer commandBuffer,
    const VkCommandBufferBeginInfo *pBeginInfo)
{
    return VK_CALL(vkBeginCommandBuffer,
        commandBuffer,
        pBeginInfo
    );
}

VkResult virgl_vk_bind_buffer_memory(
    VkDevice device,
    VkBuffer buffer,
    VkDeviceMemory memory,
    VkDeviceSize memoryOffset)
{
    return VK_CALL(vkBindBufferMemory,
        device,
        buffer,
        memory,
        memoryOffset
    );
}

void virgl_vk_bind_descriptor_sets(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    uint32_t descriptorSetCount,
    const VkDescriptorSet *pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t *pDynamicOffsets)
{
    VK_CALL(vkCmdBindDescriptorSets,
        commandBuffer,
        pipelineBindPoint,
        layout,
        firstSet,
        descriptorSetCount,
        pDescriptorSets,
        dynamicOffsetCount,
        pDynamicOffsets
    );
}

void virgl_vk_cmd_bind_pipeline(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline)
{
    VK_CALL(vkCmdBindPipeline,
        commandBuffer,
        pipelineBindPoint,
        pipeline
    );
}

void virgl_vk_cmd_dispatch(
    VkCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ)
{
    VK_CALL(vkCmdDispatch,
        commandBuffer,
        groupCountX,
        groupCountY,
        groupCountZ
    );
}

VkResult virgl_vk_create_buffer(
    VkDevice device,
    const VkBufferCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkBuffer *pBuffer)
{
    return VK_CALL(vkCreateBuffer,
        device,
        pCreateInfo,
        pAllocator,
        pBuffer
    );
}

VkResult virgl_vk_create_compute_pipeline(
    VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkComputePipelineCreateInfo *pCreateInfos,
    const VkAllocationCallbacks *pAllocator,
    VkPipeline *pPipelines)
{
    return VK_CALL(vkCreateComputePipelines,
        device,
        pipelineCache,
        createInfoCount,
        pCreateInfos,
        pAllocator,
        pPipelines
    );
}

VkResult virgl_vk_create_fence(
    VkDevice device,
    const VkFenceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkFence *pFence)
{
    return VK_CALL(vkCreateFence,
        device,
        pCreateInfo,
        pAllocator,
        pFence
    );
}

VkResult virgl_vk_create_pipeline_layout(
    VkDevice device,
    const VkPipelineLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkPipelineLayout *pPipelineLayout)
{
    return VK_CALL(vkCreatePipelineLayout,
        device,
        pCreateInfo,
        pAllocator,
        pPipelineLayout
    );
}

VkResult virgl_vk_create_shader_module(
    VkDevice device,
    const VkShaderModuleCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkShaderModule *pShaderModule)
{
    return VK_CALL(vkCreateShaderModule,
        device,
        pCreateInfo,
        pAllocator,
        pShaderModule
    );
}

void virgl_vk_destroy_buffer(
    VkDevice device,
    VkBuffer buffer,
    const VkAllocationCallbacks *pAllocator)
{
    VK_CALL(vkDestroyBuffer,
        device,
        buffer,
        pAllocator
    );
}

VkResult virgl_vk_end_command_buffer(
    VkCommandBuffer commandBuffer)
{
    return VK_CALL(vkEndCommandBuffer,
        commandBuffer
    );
}

void virgl_vk_free_memory(
     VkDevice device,
     VkDeviceMemory memory,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkFreeMemory,
        device,
        memory,
        pAllocator
    );
}

void virgl_vk_get_physical_device_memory_properties(
     VkPhysicalDevice physicalDevice,
     VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    VK_CALL(vkGetPhysicalDeviceMemoryProperties,
        physicalDevice,
        pMemoryProperties
    );
}

VkResult virgl_vk_queue_submit(
     VkQueue queue,
     uint32_t submitCount,
     const VkSubmitInfo* pSubmits,
     VkFence fence)
{
    return VK_CALL(vkQueueSubmit,
        queue,
        submitCount,
        pSubmits,
        fence
    );
}

void virgl_vk_unmap_memory(
     VkDevice device,
     VkDeviceMemory memory)
{
    VK_CALL(vkUnmapMemory,
        device,
        memory
    );
}

VkResult virgl_vk_wait_for_fences(
     VkDevice device,
     uint32_t fenceCount,
     const VkFence* pFences,
     VkBool32 waitAll,
     uint64_t timeout)
{
    return VK_CALL(vkWaitForFences,
        device,
        fenceCount,
        pFences,
        waitAll,
        timeout
    );
}

void virgl_vk_destroy_fences(
     VkDevice device,
     VkFence fence,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyFence,
        device,
        fence,
        pAllocator
    );
}

void virgl_vk_destroy_shader_module(
     VkDevice device,
     VkShaderModule shaderModule,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyShaderModule,
        device,
        shaderModule,
        pAllocator
    );
}

void virgl_vk_descriptor_pool(
     VkDevice device,
     VkDescriptorPool descriptorPool,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyDescriptorPool,
        device,
        descriptorPool,
        pAllocator
    );
}

void virgl_vk_destroy_descriptor_set_layout(
     VkDevice device,
     VkDescriptorSetLayout descriptorSetLayout,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyDescriptorSetLayout,
        device,
        descriptorSetLayout,
        pAllocator
    );
}

void virgl_vk_destroy_pipeline_layout(
     VkDevice device,
     VkPipelineLayout pipelineLayout,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyPipelineLayout,
        device,
        pipelineLayout,
        pAllocator
    );
}

void virgl_vk_destroy_pipeline(
     VkDevice device,
     VkPipeline pipeline,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyPipeline,
        device,
        pipeline,
        pAllocator
    );
}

void virgl_vk_destroy_command_pool(
     VkDevice device,
     VkCommandPool commandPool,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyCommandPool,
        device,
        commandPool,
        pAllocator
    );
}

void virgl_vk_destroy_device(
    VkDevice device,
    const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyDevice,
        device,
        pAllocator
    );
}

void virgl_vk_destroy_instance(
     VkInstance instance,
     const VkAllocationCallbacks* pAllocator)
{
    VK_CALL(vkDestroyInstance,
        instance,
        pAllocator
    );
}

VkResult virgl_vk_map_memory(
     VkDevice device,
     VkDeviceMemory memory,
     VkDeviceSize offset,
     VkDeviceSize size,
     VkMemoryMapFlags flags,
     void** ppData)
{
    return VK_CALL(vkMapMemory,
        device,
        memory,
        offset,
        size,
        flags,
        ppData
    );
}
