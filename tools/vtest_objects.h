           
#pragma once                
                            
#include <vulkan/vulkan.h>  

int vtest_create_descriptor_set_layout(uint32_t length_dw);
int vtest_create_buffer(uint32_t length_dw);

struct payload_create_descriptor_set_layout_intro {
   uint32_t handle;
   uint32_t flags;
   uint32_t bindingCount;
};

struct payload_create_descriptor_set_layout_pBindings {
   uint32_t binding;
   uint32_t descriptorType;
   uint32_t descriptorCount;
   uint32_t stageFlags;
};

struct payload_create_buffer_intro {
   uint32_t handle;
   uint32_t flags;
   uint64_t size;
   uint32_t usage;
   uint32_t sharingMode;
   uint32_t queueFamilyIndexCount;
};
