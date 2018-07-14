#ifndef VTEST_VK_COMMANDS
#define VTEST_VK_COMMANDS

struct payload_command_pool_create_info {
   uint32_t device_handle;
   uint32_t flags;
   uint32_t queue_family_index;
};

struct payload_command_buffer_allocate_info {
   uint32_t device_handle;
   uint32_t pool_handle;
   uint32_t level;
   uint32_t count;
};

struct payload_command_record_info {
   uint32_t device_handle;
   uint32_t cmd_handle;
   uint32_t pool_handle;
   uint32_t pipeline_handle;
   uint32_t pipeline_layout_handle;
   uint32_t bind_point;
   uint32_t descriptor_count;
   uint32_t dispatch_size[3];
};

int vtest_vk_create_command_pool(uint32_t length_dw);
int vtest_vk_allocate_command_buffers(uint32_t length_dw);
int vtest_vk_record_command(uint32_t length_dw);

#endif
