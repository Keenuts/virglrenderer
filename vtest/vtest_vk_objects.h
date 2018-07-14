#ifndef VTEST_VK_OBJECTS
#define VTEST_VK_OBJECTS

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

struct payload_create_descriptor_pool_intro {
   uint32_t handle;
   uint32_t flags;
   uint32_t maxSets;
   uint32_t poolSizeCount;
};

struct payload_create_descriptor_pool_pPoolSizes {
   uint32_t type;
   uint32_t descriptorCount;
};

struct payload_create_pipeline_layout_intro {
   uint32_t handle;
   uint32_t flags;
   uint32_t setLayoutCount;
   uint32_t pushConstantRangeCount;
};

struct payload_create_pipeline_layout_pPushConstantRanges {
   uint32_t stageFlags;
   uint32_t offset;
   uint32_t size;
};

struct payload_create_compute_pipelines_intro {
   uint32_t handle;
   uint32_t flags;
   uint32_t layout;
   uint32_t stage_flags;
   uint32_t stage_stage;
   uint32_t stage_module;
   uint32_t entrypoint_len;
};

struct payload_allocate_memory {
   uint32_t handle;
   uint32_t memory_index;
   uint64_t device_size;
};

struct payload_create_buffer {
   uint32_t handle;
   uint32_t flags;
   uint64_t size;
   uint32_t usage;
   uint32_t sharingMode;
   uint32_t queueFamilyIndexCount;
};

struct payload_bind_buffer_memory {
   uint32_t device_handle;
   uint32_t buffer_handle;
   uint32_t memory_handle;
   uint64_t offset;
};

struct payload_write_descriptor_set_intro {
   uint32_t device_handle;
   uint32_t dstSet;
   uint32_t dstBinding;
   uint32_t dstArrayElement;
   uint32_t descriptorType;
   uint32_t descriptorCount;
};

struct payload_write_descriptor_set_buffer {
   uint32_t buffer_handle;
   uint64_t offset;
   uint64_t range;
};

struct payload_create_fence {
   uint32_t device_handle;
   uint32_t flags;
};

struct payload_wait_for_fences {
   uint32_t device_handle;
   uint32_t fence_count;
   uint32_t wait_all;
   uint32_t timeout;
   /* uint32_t fence_handles[] */;
};

struct payload_queue_submit {
   uint32_t device_handle;
   uint32_t queue_handle;
   uint32_t fence_handle;
   uint32_t wait_count;
   uint32_t cmd_count;
   uint32_t signal_count;
   /* uint32_t wait_handles[]; */
   /* uint32_t cmd_handles[]; */
   /* uint32_t signal_handles[]; */
};

int
vtest_vk_create_descriptor_set_layout(uint32_t length_dw);
int
vtest_vk_allocate_descriptor_sets(uint32_t length_dw);
int
vtest_vk_create_shader_module(uint32_t length_dw);
int
vtest_vk_create_descriptor_pool(uint32_t length_dw);
int
vtest_vk_create_pipeline_layout(uint32_t length_dw);
int
vtest_vk_create_compute_pipelines(uint32_t length_dw);
int
vtest_vk_allocate_memory(uint32_t length_dw);
int
vtest_vk_create_buffer(uint32_t length_dw);
int
vtest_vk_bind_buffer_memory(uint32_t length_dw);
int
vtest_vk_write_descriptor_set(uint32_t length_dw);
int
vtest_vk_create_fence(uint32_t length_dw);
int
vtest_vk_wait_for_fences(uint32_t length_dw);
int
vtest_vk_queue_submit(uint32_t length_dw);

#endif
