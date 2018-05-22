#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

struct symbol {
   const char *sym_name;
   int hash;
   void *sym_addr;
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

      return _vk_symbols.symbols[i].sym_addr;
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
   _vk_symbols.symbols[_vk_symbols.used].sym_addr = addr;
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
