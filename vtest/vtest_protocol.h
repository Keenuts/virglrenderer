/**************************************************************************
 *
 * Copyright (C) 2015 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
#ifndef VTEST_PROTOCOL
#define VTEST_PROTOCOL

#include <vulkan/vulkan.h>

#define VTEST_DEFAULT_SOCKET_NAME "/tmp/.virgl_test"

/* 32-bit length field */
/* 32-bit cmd field */
#define VTEST_HDR_SIZE 2
#define VTEST_CMD_LEN 0 /* length of data */
#define VTEST_CMD_ID  1
#define VTEST_CMD_DATA_START 2

/* vtest cmds */
#define VCMD_GET_CAPS 1

#define VCMD_RESOURCE_CREATE 2
#define VCMD_RESOURCE_UNREF 3

#define VCMD_TRANSFER_GET 4
#define VCMD_TRANSFER_PUT 5

#define VCMD_SUBMIT_CMD 6

#define VCMD_RESOURCE_BUSY_WAIT 7

/* pass the process cmd line for debugging */
#define VCMD_CREATE_RENDERER 8
/* get caps */
/* 0 length cmd */
/* resp VCMD_GET_CAPS + caps */

#define VCMD_VK_ALLOCATE_COMMAND_BUFFERS              9
#define VCMD_VK_ALLOCATE_DESCRIPTORS                  10
#define VCMD_VK_ALLOCATE_MEMORY                       11
#define VCMD_VK_BIND_BUFFER_MEMORY                    12
#define VCMD_VK_CREATE_BUFFER                         13
#define VCMD_VK_CREATE_COMMAND_POOL                   14
#define VCMD_VK_CREATE_COMPUTE_PIPELINES              15
#define VCMD_VK_CREATE_DESCRIPTOR_POOL                16
#define VCMD_VK_CREATE_DESCRIPTOR_SET_LAYOUT          17
#define VCMD_VK_CREATE_DEVICE                         18
#define VCMD_VK_CREATE_FENCE                          19
#define VCMD_VK_CREATE_PIPELINE_LAYOUT                20
#define VCMD_VK_CREATE_SHADER_MODULE                  21
#define VCMD_VK_ENUMERATE_PHYSICAL_DEVICES            22
#define VCMD_VK_GET_DEVICE_MEMORY                     23
#define VCMD_VK_GET_PHYSICAL_DEVICE_SPARCE_PROPERTIES 24
#define VCMD_VK_GET_QUEUE_FAMILY_PROPS                25
#define VCMD_VK_QUEUE_SUBMIT                          26
#define VCMD_VK_READ_MEMORY                           27
#define VCMD_VK_RECORD_COMMAND                        28
#define VCMD_VK_WAIT_FOR_FENCES                       29
#define VCMD_VK_WRITE_DESCRIPTOR_SET                  30
#define VCMD_VK_WRITE_MEMORY                          31
#define VCMD_VK_DESTROY_OBJECT                        32
#define VCMD_VK_DESTROY_DEVICE                        33

#define VCMD_COMMAND_COUNT                            33

#define VCMD_RES_CREATE_SIZE 10
#define VCMD_RES_CREATE_RES_HANDLE 0
#define VCMD_RES_CREATE_TARGET 1
#define VCMD_RES_CREATE_FORMAT 2
#define VCMD_RES_CREATE_BIND 3
#define VCMD_RES_CREATE_WIDTH 4
#define VCMD_RES_CREATE_HEIGHT 5
#define VCMD_RES_CREATE_DEPTH 6
#define VCMD_RES_CREATE_ARRAY_SIZE 7
#define VCMD_RES_CREATE_LAST_LEVEL 8
#define VCMD_RES_CREATE_NR_SAMPLES 9

#define VCMD_RES_UNREF_SIZE 1
#define VCMD_RES_UNREF_RES_HANDLE 0

#define VCMD_TRANSFER_HDR_SIZE 11
#define VCMD_TRANSFER_RES_HANDLE 0
#define VCMD_TRANSFER_LEVEL 1
#define VCMD_TRANSFER_STRIDE 2
#define VCMD_TRANSFER_LAYER_STRIDE 3
#define VCMD_TRANSFER_X 4
#define VCMD_TRANSFER_Y 5
#define VCMD_TRANSFER_Z 6
#define VCMD_TRANSFER_WIDTH 7
#define VCMD_TRANSFER_HEIGHT 8
#define VCMD_TRANSFER_DEPTH 9
#define VCMD_TRANSFER_DATA_SIZE 10

#define VCMD_BUSY_WAIT_FLAG_WAIT 1

#define VCMD_BUSY_WAIT_SIZE 2
#define VCMD_BUSY_WAIT_HANDLE 0
#define VCMD_BUSY_WAIT_FLAGS 1

struct vtest_hdr {
    union {
        uint32_t raw[2];
        struct {
            uint32_t length;
            uint32_t id;
        };
    };
};

struct vtest_result {
   uint32_t error_code;
   uint32_t result;
};

struct vtest_payload_device_get {
   uint32_t device_id;
};

struct vtest_payload_queue_create {
   VkDeviceQueueCreateFlags flags;
   uint32_t queue_family_index;
   uint32_t queue_count;
   /* float priorities[]; */
};

struct vtest_payload_device_create {
   uint32_t physical_device_id;
   VkDeviceCreateFlags flags;
   VkPhysicalDeviceFeatures features;

   uint32_t queue_info_count;
};

struct vtest_payload_destroy_device {
   uint32_t device_handle;
};

struct vtest_payload_destroy_object {
   uint32_t device_handle;
   uint32_t object_handle;
};

struct vtest_payload_rw_memory {
   uint32_t device_handle;
   uint32_t memory_handle;
   uint64_t offset;
   uint64_t size;
};

#endif
