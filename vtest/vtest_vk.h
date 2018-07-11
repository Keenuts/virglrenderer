#ifndef VTEST_VK_H
#define VTEST_VK_H

int vtest_vk_enumerate_devices(uint32_t length_dw);
int vtest_vk_get_sparse_properties(uint32_t length_dw);
int vtest_vk_get_queue_family_properties(uint32_t length_dw);
int vtest_vk_get_device_memory_properties(uint32_t length_dw);
int vtest_vk_create_device(uint32_t length_dw);
int vtest_vk_read_memory(uint32_t length_dw);
int vtest_vk_write_memory(uint32_t length_dw);

#define CHECK_IO_RESULT(Done, Expected)                                    \
   if ((Done) < (Expected)) {                                              \
      fprintf(stderr, "%s: failed to write back the answer.\n", __func__); \
      RETURN(-1);                                                          \
   }

#endif
