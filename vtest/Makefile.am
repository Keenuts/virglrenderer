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
tools_dir = $(top_srcdir)/tools
vulkan_api_xml = $(tools_dir)/vk.xml

virgl_test_server_SOURCES +=	\
	vtest_vk.c		\
	vtest_vk_objects.c

vtest_vk_objects.c: $(tools_dir)/codegen_protocol.py \
	$(tools_dir)/codegen_utils.py \
	$(vulkan_api_xml)	\
	$(tools_dir)/vtest_protocol_objects.json
	$(AM_V_GEN)$(PYTHON3) $(tools_dir)/codegen_protocol.py \
		--xml $(vulkan_api_xml) \
		--outdir .

vtest_vk_objects.h: vtest_vk_objects.c
endif
virgl_test_server_LDADD = $(top_builddir)/src/libvirglrenderer.la
