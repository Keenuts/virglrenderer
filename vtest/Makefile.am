AM_CFLAGS = \
        -I$(top_srcdir)/src/gallium/drivers/virgl \
	-I$(top_srcdir)/src/gallium/include \
	-I$(top_srcdir)/src/gallium/auxiliary \
	-I$(top_srcdir)/src/gallium/drivers \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	$(DEFINES) \
	$(PIC_FLAGS) \
	$(LIBDRM_CFLAGS) \
	$(EPOXY_CFLAGS) \
	$(VISIBILITY_CFLAGS) \
	$(CODE_COVERAGE_CFLAGS)

bin_PROGRAMS = virgl_test_server

virgl_test_server_SOURCES =			\
	util.c					\
	util.h					\
	vtest_server.c				\
	vtest_renderer.c			\
	vtest_protocol.h			\
	vtest.h

if WITH_VULKAN
virgl_test_server_SOURCES +=	\
	vtest_vk.c				\
	vtest_vk_commands.c	\
	vtest_vk_commands.h	\
	vtest_vk_objects.c	\
	vtest_vk_objects.h
endif

virgl_test_server_LDADD = $(top_builddir)/src/libvirglrenderer.la
