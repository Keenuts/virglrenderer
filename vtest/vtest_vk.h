#ifndef VTEST_VK_H
#define VTEST_VK_H

int vtest_vk_enumerate_devices(uint32_t length_dw);
int vtest_vk_get_sparse_properties(uint32_t length_dw);
int vtest_vk_get_queue_family_properties(uint32_t length_dw);
int vtest_vk_create_device(uint32_t length_dw);
int vtest_vk_create_descriptor_set(uint32_t length_dw);
int vtest_vk_create_buffer(uint32_t length_dw);

#endif
