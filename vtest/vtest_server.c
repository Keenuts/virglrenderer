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
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>

#include "util.h"
#include "vtest.h"
#include "vtest_protocol.h"

static int vtest_open_socket(const char *path)
{
    struct sockaddr_un un;
    int sock;

    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;

    snprintf(un.sun_path, sizeof(un.sun_path), "%s", path);

    unlink(un.sun_path);

    if (bind(sock, (struct sockaddr *)&un, sizeof(un)) < 0) {
        goto err;
    }

    if (listen(sock, 1) < 0){
        goto err;
    }

    return sock;
err:
    close(sock);
    return -1;
}

static int wait_for_socket_accept(int sock)
{
    fd_set read_fds;
    int new_fd;
    int ret;
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);

    ret = select(sock + 1, &read_fds, NULL, NULL, NULL);
    if (ret < 0)
        return ret;

    if (FD_ISSET(sock, &read_fds)) {	
        new_fd = accept(sock, NULL, NULL);
        return new_fd;
    }
    return -1;
}

static int (*_commands_ftable[])(uint32_t header) = {
    NULL /* CMD ids starts at 1 */,
    vtest_send_caps,
    vtest_create_resource,
    vtest_resource_unref,
    vtest_transfer_get,
    vtest_transfer_put,
    vtest_submit_cmd,
    vtest_resource_busy_wait,
    NULL /* vtest_create_renderer */,
};
#define COMMAND_COUNT (sizeof(_commands_ftable) / sizeof(*_commands_ftable))

static int run_renderer(int in_fd, int out_fd)
{
   int ret;
   uint32_t header[VTEST_HDR_SIZE];
   bool inited = false;

   do {
again:
      ret = vtest_wait_for_fd_read(in_fd);
      if (ret < 0) {
         break;
      }

      ret = vtest_block_read(in_fd, &header, sizeof(header));
      if (ret != 8) {
         break;
      }

      if (!inited) {
         if (header[1] != VCMD_CREATE_RENDERER) {
            break;
         }

         ret = vtest_create_renderer(in_fd, out_fd, header[0]);
         inited = true;
         vtest_poll();
         continue;
      }

      vtest_poll();

      if (header[1] <= 0 || header[1] > COMMAND_COUNT) {
         break;
      }

      if (header[1] == VCMD_RESOURCE_BUSY_WAIT) { //Only specific case
         vtest_renderer_create_fence();
      }
      ret = _commands_ftable[header[1]](header[0]);

      if (ret < 0) {
         break;
      }
   } while (1);

   fprintf(stderr, "socket failed - closing renderer\n");
   vtest_destroy_renderer();
   close(in_fd);
   return 0;
}

#undef COMMAND_COUNT

int main(int argc, char **argv)
{
    int ret, sock = -1, in_fd, out_fd;
    pid_t pid;
    bool do_fork = true, loop = true;
    struct sigaction sa;

#ifdef __AFL_LOOP
while (__AFL_LOOP(1000)) {
#endif

   if (argc > 1) {
      if (!strcmp(argv[1], "--no-fork"))
	do_fork = false;
      else {
         ret = open(argv[1], O_RDONLY);
         if (ret == -1) {
            perror(0);
            exit(1);
         }
         in_fd = ret;
         ret = open("/dev/null", O_WRONLY);
         if (ret == -1) {
            perror(0);
            exit(1);
         }
         out_fd = ret;
         loop = false;
         do_fork = false;
         goto start;
      }
    }

    if (do_fork) {
      sa.sa_handler = SIG_IGN;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      if (sigaction(SIGCHLD, &sa, 0) == -1) {
	perror(0);
	exit(1);
      }
    }

    sock = vtest_open_socket("/tmp/.virgl_test");
restart:
    in_fd = wait_for_socket_accept(sock);
    out_fd = in_fd;

start:
    if (do_fork) {
      /* fork a renderer process */
      switch ((pid = fork())) {
      case 0:
        run_renderer(in_fd, out_fd);
	exit(0);
	break;
      case -1:
      default:
	close(in_fd);
        if (loop)
           goto restart;
      }
    } else {
      run_renderer(in_fd, out_fd);
      if (loop)
         goto restart;
    }

    if (sock != -1)
       close(sock);
    if (in_fd != out_fd)
       close(out_fd);

#ifdef __AFL_LOOP
}
#endif
}
