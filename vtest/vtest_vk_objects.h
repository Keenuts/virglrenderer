#ifndef VTEST_VK_PROTOCOL
#define VTEST_VK_PROTOCOL

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

struct payload_allocate_descriptor_sets_intro {
   uint32_t handle;
   uint32_t descriptorPool;
   uint32_t descriptorSetCount;
};

struct payload_create_shader_module_intro {
   uint32_t handle;
   uint32_t flags;
   uint32_t codeSize;
};

int
vtest_vk_create_descriptor_set_layout(uint32_t length_dw);
int
vtest_vk_create_buffer(uint32_t length_dw);
int
vtest_vk_allocate_descriptor_sets(uint32_t length_dw);
int
vtest_vk_create_shader_module(uint32_t length_dw);

#endif
