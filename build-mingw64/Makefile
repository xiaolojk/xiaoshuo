# Makefile to build and install the SDL library

top_builddir = .
srcdir  = ../SDL2-2.28.5
objects = build
gen = gen
prefix = /usr/local/cross-tools/x86_64-w64-mingw32
exec_prefix = ${prefix}
bindir	= ${exec_prefix}/bin
libdir  = ${exec_prefix}/lib
includedir = ${prefix}/include
datarootdir = ${prefix}/share
datadir	= ${datarootdir}
auxdir	= ../SDL2-2.28.5/build-scripts/
distpath = $(srcdir)/..
distdir = SDL2-2.28.5
distfile = $(distdir).tar.gz


SHELL	= /bin/bash
CC      = x86_64-w64-mingw32-gcc
CXX     = g++
INCLUDE = -Iinclude -I../SDL2-2.28.5/include -idirafter ../SDL2-2.28.5/src/video/khronos
CFLAGS  = -g -O3 -DUSING_GENERATED_CONFIG_H
EXTRA_CFLAGS = -Iinclude -I../SDL2-2.28.5/include -idirafter ../SDL2-2.28.5/src/video/khronos  -DSDL_BUILD_MAJOR_VERSION=2 -DSDL_BUILD_MINOR_VERSION=28 -DSDL_BUILD_MICRO_VERSION=5 -mmmx -m3dnow -msse -msse2 -msse3 -Wall -Wno-unused-local-typedefs -fno-strict-aliasing -gdwarf-4 -Wdeclaration-after-statement -Werror=declaration-after-statement
LDFLAGS =  -Wl,--no-undefined -Wc,-static-libgcc
EXTRA_LDFLAGS =  -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INSTALL = /usr/bin/install -c
FGREP	= /usr/bin/grep -F
AR	= x86_64-w64-mingw32-ar
RANLIB	= x86_64-w64-mingw32-ranlib
RC	= x86_64-w64-mingw32-windres
LINKER = $(CC)
LIBTOOLLINKERTAG = CC
SDL_VENDOR_INFO = 

TARGET  = libSDL2.la
OBJECTS = $(objects)/SDL.lo $(objects)/SDL_assert.lo $(objects)/SDL_dataqueue.lo $(objects)/SDL_error.lo $(objects)/SDL_guid.lo $(objects)/SDL_hints.lo $(objects)/SDL_list.lo $(objects)/SDL_log.lo $(objects)/SDL_utils.lo $(objects)/SDL_atomic.lo $(objects)/SDL_spinlock.lo $(objects)/SDL_audio.lo $(objects)/SDL_audiocvt.lo $(objects)/SDL_audiodev.lo $(objects)/SDL_audiotypecvt.lo $(objects)/SDL_mixer.lo $(objects)/SDL_wave.lo $(objects)/SDL_cpuinfo.lo $(objects)/SDL_dynapi.lo $(objects)/SDL_clipboardevents.lo $(objects)/SDL_displayevents.lo $(objects)/SDL_dropevents.lo $(objects)/SDL_events.lo $(objects)/SDL_gesture.lo $(objects)/SDL_keyboard.lo $(objects)/SDL_keysym_to_scancode.lo $(objects)/SDL_mouse.lo $(objects)/SDL_quit.lo $(objects)/SDL_scancode_tables.lo $(objects)/SDL_touch.lo $(objects)/SDL_windowevents.lo $(objects)/imKStoUCS.lo $(objects)/SDL_rwops.lo $(objects)/SDL_haptic.lo $(objects)/SDL_hidapi.lo $(objects)/SDL_gamecontroller.lo $(objects)/SDL_joystick.lo $(objects)/controller_type.lo $(objects)/e_atan2.lo $(objects)/e_exp.lo $(objects)/e_fmod.lo $(objects)/e_log.lo $(objects)/e_log10.lo $(objects)/e_pow.lo $(objects)/e_rem_pio2.lo $(objects)/e_sqrt.lo $(objects)/k_cos.lo $(objects)/k_rem_pio2.lo $(objects)/k_sin.lo $(objects)/k_tan.lo $(objects)/s_atan.lo $(objects)/s_copysign.lo $(objects)/s_cos.lo $(objects)/s_fabs.lo $(objects)/s_floor.lo $(objects)/s_scalbn.lo $(objects)/s_sin.lo $(objects)/s_tan.lo $(objects)/SDL_url.lo $(objects)/SDL_power.lo $(objects)/SDL_d3dmath.lo $(objects)/SDL_render.lo $(objects)/SDL_yuv_sw.lo $(objects)/SDL_render_d3d.lo $(objects)/SDL_shaders_d3d.lo $(objects)/SDL_render_d3d11.lo $(objects)/SDL_shaders_d3d11.lo $(objects)/SDL_render_d3d12.lo $(objects)/SDL_shaders_d3d12.lo $(objects)/SDL_render_gl.lo $(objects)/SDL_shaders_gl.lo $(objects)/SDL_render_gles.lo $(objects)/SDL_render_gles2.lo $(objects)/SDL_shaders_gles2.lo $(objects)/SDL_render_ps2.lo $(objects)/SDL_render_psp.lo $(objects)/SDL_blendfillrect.lo $(objects)/SDL_blendline.lo $(objects)/SDL_blendpoint.lo $(objects)/SDL_drawline.lo $(objects)/SDL_drawpoint.lo $(objects)/SDL_render_sw.lo $(objects)/SDL_rotate.lo $(objects)/SDL_triangle.lo $(objects)/SDL_render_vita_gxm.lo $(objects)/SDL_render_vita_gxm_memory.lo $(objects)/SDL_render_vita_gxm_tools.lo $(objects)/SDL_sensor.lo $(objects)/SDL_crc16.lo $(objects)/SDL_crc32.lo $(objects)/SDL_getenv.lo $(objects)/SDL_iconv.lo $(objects)/SDL_malloc.lo $(objects)/SDL_mslibc.lo $(objects)/SDL_qsort.lo $(objects)/SDL_stdlib.lo $(objects)/SDL_string.lo $(objects)/SDL_strtokr.lo $(objects)/SDL_thread.lo $(objects)/SDL_timer.lo $(objects)/SDL_RLEaccel.lo $(objects)/SDL_blit.lo $(objects)/SDL_blit_0.lo $(objects)/SDL_blit_1.lo $(objects)/SDL_blit_A.lo $(objects)/SDL_blit_N.lo $(objects)/SDL_blit_auto.lo $(objects)/SDL_blit_copy.lo $(objects)/SDL_blit_slow.lo $(objects)/SDL_bmp.lo $(objects)/SDL_clipboard.lo $(objects)/SDL_egl.lo $(objects)/SDL_fillrect.lo $(objects)/SDL_pixels.lo $(objects)/SDL_rect.lo $(objects)/SDL_shape.lo $(objects)/SDL_stretch.lo $(objects)/SDL_surface.lo $(objects)/SDL_video.lo $(objects)/SDL_vulkan_utils.lo $(objects)/SDL_yuv.lo $(objects)/yuv_rgb.lo $(objects)/SDL_locale.lo $(objects)/SDL_nullevents.lo $(objects)/SDL_nullframebuffer.lo $(objects)/SDL_nullvideo.lo $(objects)/SDL_offscreenevents.lo $(objects)/SDL_offscreenframebuffer.lo $(objects)/SDL_offscreenopengles.lo $(objects)/SDL_offscreenvideo.lo $(objects)/SDL_offscreenwindow.lo $(objects)/SDL_hidapi_combined.lo $(objects)/SDL_hidapi_gamecube.lo $(objects)/SDL_hidapi_luna.lo $(objects)/SDL_hidapi_ps3.lo $(objects)/SDL_hidapi_ps4.lo $(objects)/SDL_hidapi_ps5.lo $(objects)/SDL_hidapi_rumble.lo $(objects)/SDL_hidapi_shield.lo $(objects)/SDL_hidapi_stadia.lo $(objects)/SDL_hidapi_steam.lo $(objects)/SDL_hidapi_switch.lo $(objects)/SDL_hidapi_wii.lo $(objects)/SDL_hidapi_xbox360.lo $(objects)/SDL_hidapi_xbox360w.lo $(objects)/SDL_hidapi_xboxone.lo $(objects)/SDL_hidapijoystick.lo $(objects)/SDL_hid.lo $(objects)/SDL_immdevice.lo $(objects)/SDL_windows.lo $(objects)/SDL_xinput.lo $(objects)/SDL_sysurl.lo $(objects)/SDL_syslocale.lo $(objects)/SDL_windowsclipboard.lo $(objects)/SDL_windowsevents.lo $(objects)/SDL_windowsframebuffer.lo $(objects)/SDL_windowskeyboard.lo $(objects)/SDL_windowsmessagebox.lo $(objects)/SDL_windowsmodes.lo $(objects)/SDL_windowsmouse.lo $(objects)/SDL_windowsopengl.lo $(objects)/SDL_windowsopengles.lo $(objects)/SDL_windowsshape.lo $(objects)/SDL_windowsvideo.lo $(objects)/SDL_windowsvulkan.lo $(objects)/SDL_windowswindow.lo $(objects)/SDL_dinputjoystick.lo $(objects)/SDL_rawinputjoystick.lo $(objects)/SDL_windows_gaming_input.lo $(objects)/SDL_windowsjoystick.lo $(objects)/SDL_xinputjoystick.lo $(objects)/SDL_dinputhaptic.lo $(objects)/SDL_windowshaptic.lo $(objects)/SDL_xinputhaptic.lo $(objects)/SDL_windowssensor.lo $(objects)/SDL_syspower.lo $(objects)/SDL_sysfilesystem.lo $(objects)/SDL_syscond_cv.lo $(objects)/SDL_sysmutex.lo $(objects)/SDL_syssem.lo $(objects)/SDL_systhread.lo $(objects)/SDL_systls.lo $(objects)/SDL_syscond.lo $(objects)/SDL_systimer.lo $(objects)/SDL_sysloadso.lo $(objects)/SDL_virtualjoystick.lo $(objects)/SDL_dummyaudio.lo
GEN_HEADERS = 
GEN_OBJECTS = 
VERSION_OBJECTS = $(objects)/version.lo

SDLMAIN_TARGET = libSDL2main.la
SDLMAIN_OBJECTS = $(objects)/SDL_windows_main.lo

SDLTEST_TARGET = libSDL2_test.la
SDLTEST_OBJECTS = $(objects)/SDL_test_assert.lo $(objects)/SDL_test_common.lo $(objects)/SDL_test_compare.lo $(objects)/SDL_test_crc32.lo $(objects)/SDL_test_font.lo $(objects)/SDL_test_fuzzer.lo $(objects)/SDL_test_harness.lo $(objects)/SDL_test_imageBlit.lo $(objects)/SDL_test_imageBlitBlend.lo $(objects)/SDL_test_imageFace.lo $(objects)/SDL_test_imagePrimitives.lo $(objects)/SDL_test_imagePrimitivesBlend.lo $(objects)/SDL_test_log.lo $(objects)/SDL_test_md5.lo $(objects)/SDL_test_memory.lo $(objects)/SDL_test_random.lo

WAYLAND_SCANNER = 
WAYLAND_SCANNER_CODE_MODE = 

INSTALL_SDL2_CONFIG = TRUE

SRC_DIST = *.md *.txt acinclude Android.mk autogen.sh android-project build-scripts cmake cmake_uninstall.cmake.in configure configure.ac docs include Makefile.* mingw sdl2-config.cmake.in sdl2-config-version.cmake.in sdl2-config.in sdl2.m4 sdl2.pc.in SDL2.spec.in SDL2Config.cmake.in src test VisualC VisualC-WinRT Xcode Xcode-iOS wayland-protocols
GEN_DIST = SDL2.spec

ifneq ($V,1)
RUN_CMD_AR     = @echo "  AR    " $@;
RUN_CMD_CC     = @echo "  CC    " $@;
RUN_CMD_CXX    = @echo "  CXX   " $@;
RUN_CMD_LTLINK = @echo "  LTLINK" $@;
RUN_CMD_RANLIB = @echo "  RANLIB" $@;
RUN_CMD_RC     = @echo "  RC    " $@;
RUN_CMD_GEN    = @echo "  GEN   " $@;
LIBTOOL += --quiet
endif

HDRS = \
	SDL.h \
	SDL_assert.h \
	SDL_atomic.h \
	SDL_audio.h \
	SDL_bits.h \
	SDL_blendmode.h \
	SDL_clipboard.h \
	SDL_cpuinfo.h \
	SDL_egl.h \
	SDL_endian.h \
	SDL_error.h \
	SDL_events.h \
	SDL_filesystem.h \
	SDL_gamecontroller.h \
	SDL_gesture.h \
	SDL_guid.h \
	SDL_haptic.h \
	SDL_hidapi.h \
	SDL_hints.h \
	SDL_joystick.h \
	SDL_keyboard.h \
	SDL_keycode.h \
	SDL_loadso.h \
	SDL_locale.h \
	SDL_log.h \
	SDL_main.h \
	SDL_messagebox.h \
	SDL_metal.h \
	SDL_misc.h \
	SDL_mouse.h \
	SDL_mutex.h \
	SDL_name.h \
	SDL_opengl.h \
	SDL_opengl_glext.h \
	SDL_opengles.h \
	SDL_opengles2_gl2ext.h \
	SDL_opengles2_gl2.h \
	SDL_opengles2_gl2platform.h \
	SDL_opengles2.h \
	SDL_opengles2_khrplatform.h \
	SDL_pixels.h \
	SDL_platform.h \
	SDL_power.h \
	SDL_quit.h \
	SDL_rect.h \
	SDL_render.h \
	SDL_rwops.h \
	SDL_scancode.h \
	SDL_sensor.h \
	SDL_shape.h \
	SDL_stdinc.h \
	SDL_surface.h \
	SDL_system.h \
	SDL_syswm.h \
	SDL_thread.h \
	SDL_timer.h \
	SDL_touch.h \
	SDL_types.h \
	SDL_version.h \
	SDL_video.h \
	SDL_vulkan.h \
	begin_code.h \
	close_code.h

SDLTEST_HDRS = $(shell ls $(srcdir)/include | $(FGREP) SDL_test)

LT_AGE      = 2800
LT_CURRENT  = 2800
LT_RELEASE  = 2.0
LT_REVISION = 5
LT_LDFLAGS  = -no-undefined -rpath $(libdir) -release $(LT_RELEASE) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

all: $(srcdir)/configure Makefile $(objects)/$(TARGET) $(objects)/$(SDLMAIN_TARGET) $(objects)/$(SDLTEST_TARGET)

$(srcdir)/configure: $(srcdir)/configure.ac
	@echo "Warning, configure is out of date, please re-run autogen.sh"

Makefile: $(srcdir)/Makefile.in
	$(SHELL) config.status $@

Makefile.in:;

$(objects)/.created:
	$(SHELL) $(auxdir)/mkinstalldirs $(objects)
	touch $@

update-revision:
	$(SHELL) $(auxdir)/updaterev.sh --vendor "$(SDL_VENDOR_INFO)"

.PHONY: all update-revision install install-bin install-hdrs install-lib install-data uninstall uninstall-bin uninstall-hdrs uninstall-lib uninstall-data clean distclean dist $(OBJECTS:.lo=.d)

$(objects)/$(TARGET): $(GEN_HEADERS) $(GEN_OBJECTS) $(OBJECTS) $(VERSION_OBJECTS)
	$(RUN_CMD_LTLINK)$(LIBTOOL) --tag=$(LIBTOOLLINKERTAG) --mode=link $(LINKER) -o $@ $(OBJECTS) $(GEN_OBJECTS) $(VERSION_OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LT_LDFLAGS)

$(objects)/$(SDLMAIN_TARGET): $(SDLMAIN_OBJECTS)
	$(RUN_CMD_LTLINK)$(LIBTOOL) --tag=$(LIBTOOLLINKERTAG) --mode=link $(LINKER) -static -o $@ $(SDLMAIN_OBJECTS) -rpath $(libdir)

$(objects)/$(SDLTEST_TARGET): $(SDLTEST_OBJECTS)
	$(RUN_CMD_LTLINK)$(LIBTOOL) --tag=$(LIBTOOLLINKERTAG) --mode=link $(LINKER) -static -o $@ $(SDLTEST_OBJECTS) -rpath $(libdir)

install: all install-bin install-hdrs install-lib install-data
install-bin:
ifeq ($(INSTALL_SDL2_CONFIG),TRUE)
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 sdl2-config $(DESTDIR)$(bindir)/sdl2-config
endif

install-hdrs: update-revision
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(includedir)/SDL2
	for file in $(HDRS) $(SDLTEST_HDRS); do \
	    $(INSTALL) -m 644 $(srcdir)/include/$$file $(DESTDIR)$(includedir)/SDL2/$$file; \
	done
	$(INSTALL) -m 644 include/SDL_config.h $(DESTDIR)$(includedir)/SDL2/SDL_config.h
	if test -f include/SDL_revision.h; then \
	    $(INSTALL) -m 644 include/SDL_revision.h $(DESTDIR)$(includedir)/SDL2/SDL_revision.h; \
	else \
	    $(INSTALL) -m 644 $(srcdir)/include/SDL_revision.h $(DESTDIR)$(includedir)/SDL2/SDL_revision.h; \
	fi

install-lib: $(objects)/$(TARGET) $(objects)/$(SDLMAIN_TARGET) $(objects)/$(SDLTEST_TARGET)
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(TARGET) $(DESTDIR)$(libdir)/$(TARGET)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(SDLMAIN_TARGET) $(DESTDIR)$(libdir)/$(SDLMAIN_TARGET)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(SDLTEST_TARGET) $(DESTDIR)$(libdir)/$(SDLTEST_TARGET)
install-data:
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(datadir)/aclocal
	$(INSTALL) -m 644 $(srcdir)/sdl2.m4 $(DESTDIR)$(datadir)/aclocal/sdl2.m4
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL) -m 644 sdl2.pc $(DESTDIR)$(libdir)/pkgconfig
ifeq ($(INSTALL_SDL2_CONFIG),TRUE)
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(libdir)/cmake/SDL2
	$(INSTALL) -m 644 sdl2-config.cmake $(DESTDIR)$(libdir)/cmake/SDL2
	$(INSTALL) -m 644 sdl2-config-version.cmake $(DESTDIR)$(libdir)/cmake/SDL2
endif

uninstall: uninstall-bin uninstall-hdrs uninstall-lib uninstall-data
uninstall-bin:
	rm -f $(DESTDIR)$(bindir)/sdl2-config
uninstall-hdrs:
	for file in $(HDRS) $(SDLTEST_HDRS); do \
	    rm -f $(DESTDIR)$(includedir)/SDL2/$$file; \
	done
	rm -f $(DESTDIR)$(includedir)/SDL2/SDL_config.h
	rm -f $(DESTDIR)$(includedir)/SDL2/SDL_revision.h
	-rmdir $(DESTDIR)$(includedir)/SDL2
uninstall-lib:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(libdir)/$(TARGET)
	rm -f $(DESTDIR)$(libdir)/$(SDLMAIN_TARGET)
	rm -f $(DESTDIR)$(libdir)/$(SDLTEST_TARGET)
uninstall-data:
	rm -f $(DESTDIR)$(datadir)/aclocal/sdl2.m4
	rm -f $(DESTDIR)$(libdir)/pkgconfig/sdl2.pc
	rm -f $(DESTDIR)$(libdir)/cmake/SDL2/sdl2-config.cmake
	rm -f $(DESTDIR)$(libdir)/cmake/SDL2/sdl2-config-version.cmake

clean:
	rm -rf $(objects)
	rm -rf $(gen)
	if test -f test/Makefile; then (cd test; $(MAKE) $@); fi

distclean: clean
	rm -f Makefile Makefile.rules sdl2-config
	rm -f config.status config.cache config.log libtool
	rm -rf $(srcdir)/autom4te*
	find $(srcdir) \( \
	    -name '*~' -o \
	    -name '*.bak' -o \
	    -name '*.old' -o \
	    -name '*.rej' -o \
	    -name '*.orig' -o \
	    -name '.#*' \) \
	    -exec rm -f {} \;
	if test -f test/Makefile; then (cd test; $(MAKE) $@); fi

dist $(distfile):
	$(SHELL) $(auxdir)/mkinstalldirs $(distdir)
	(cd $(srcdir); tar cf - $(SRC_DIST)) | (cd $(distdir); tar xf -)
	tar cf - $(GEN_DIST) | (cd $(distdir); tar xf -)
	find $(distdir) \( \
	    -name '*~' -o \
	    -name '*.bak' -o \
	    -name '*.old' -o \
	    -name '*.rej' -o \
	    -name '*.orig' -o \
	    -name '.#*' \) \
	    -exec rm -f {} \;
	if test -f $(distdir)/test/Makefile; then (cd $(distdir)/test && make distclean); fi
	# Intentionally no vendor suffix: that's a property of the build, not the source
	(cd $(distdir); $(srcdir)/build-scripts/updaterev.sh --dist)
	tar cvf - $(distdir) | gzip --best >$(distfile)
	rm -rf $(distdir)

rpm: $(distfile)
	rpmbuild -ta $?

# Build rules for objects
-include $(OBJECTS:.lo=.d)

# Special dependency for SDL.c, since it depends on SDL_revision.h
../SDL2-2.28.5/src/SDL.c: update-revision

$(objects)/SDL.lo: ../SDL2-2.28.5/src/SDL.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_assert.lo: ../SDL2-2.28.5/src/SDL_assert.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dataqueue.lo: ../SDL2-2.28.5/src/SDL_dataqueue.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_error.lo: ../SDL2-2.28.5/src/SDL_error.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_guid.lo: ../SDL2-2.28.5/src/SDL_guid.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hints.lo: ../SDL2-2.28.5/src/SDL_hints.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_list.lo: ../SDL2-2.28.5/src/SDL_list.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_log.lo: ../SDL2-2.28.5/src/SDL_log.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_utils.lo: ../SDL2-2.28.5/src/SDL_utils.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_atomic.lo: ../SDL2-2.28.5/src/atomic/SDL_atomic.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_spinlock.lo: ../SDL2-2.28.5/src/atomic/SDL_spinlock.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audio.lo: ../SDL2-2.28.5/src/audio/SDL_audio.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audiocvt.lo: ../SDL2-2.28.5/src/audio/SDL_audiocvt.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audiodev.lo: ../SDL2-2.28.5/src/audio/SDL_audiodev.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audiotypecvt.lo: ../SDL2-2.28.5/src/audio/SDL_audiotypecvt.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_mixer.lo: ../SDL2-2.28.5/src/audio/SDL_mixer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_wave.lo: ../SDL2-2.28.5/src/audio/SDL_wave.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_cpuinfo.lo: ../SDL2-2.28.5/src/cpuinfo/SDL_cpuinfo.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dynapi.lo: ../SDL2-2.28.5/src/dynapi/SDL_dynapi.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_clipboardevents.lo: ../SDL2-2.28.5/src/events/SDL_clipboardevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_displayevents.lo: ../SDL2-2.28.5/src/events/SDL_displayevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dropevents.lo: ../SDL2-2.28.5/src/events/SDL_dropevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_events.lo: ../SDL2-2.28.5/src/events/SDL_events.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_gesture.lo: ../SDL2-2.28.5/src/events/SDL_gesture.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_keyboard.lo: ../SDL2-2.28.5/src/events/SDL_keyboard.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_keysym_to_scancode.lo: ../SDL2-2.28.5/src/events/SDL_keysym_to_scancode.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_mouse.lo: ../SDL2-2.28.5/src/events/SDL_mouse.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_quit.lo: ../SDL2-2.28.5/src/events/SDL_quit.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_scancode_tables.lo: ../SDL2-2.28.5/src/events/SDL_scancode_tables.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_touch.lo: ../SDL2-2.28.5/src/events/SDL_touch.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowevents.lo: ../SDL2-2.28.5/src/events/SDL_windowevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/imKStoUCS.lo: ../SDL2-2.28.5/src/events/imKStoUCS.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rwops.lo: ../SDL2-2.28.5/src/file/SDL_rwops.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_haptic.lo: ../SDL2-2.28.5/src/haptic/SDL_haptic.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi.lo: ../SDL2-2.28.5/src/hidapi/SDL_hidapi.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_gamecontroller.lo: ../SDL2-2.28.5/src/joystick/SDL_gamecontroller.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_joystick.lo: ../SDL2-2.28.5/src/joystick/SDL_joystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/controller_type.lo: ../SDL2-2.28.5/src/joystick/controller_type.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_atan2.lo: ../SDL2-2.28.5/src/libm/e_atan2.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_exp.lo: ../SDL2-2.28.5/src/libm/e_exp.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_fmod.lo: ../SDL2-2.28.5/src/libm/e_fmod.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_log.lo: ../SDL2-2.28.5/src/libm/e_log.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_log10.lo: ../SDL2-2.28.5/src/libm/e_log10.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_pow.lo: ../SDL2-2.28.5/src/libm/e_pow.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_rem_pio2.lo: ../SDL2-2.28.5/src/libm/e_rem_pio2.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_sqrt.lo: ../SDL2-2.28.5/src/libm/e_sqrt.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_cos.lo: ../SDL2-2.28.5/src/libm/k_cos.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_rem_pio2.lo: ../SDL2-2.28.5/src/libm/k_rem_pio2.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_sin.lo: ../SDL2-2.28.5/src/libm/k_sin.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_tan.lo: ../SDL2-2.28.5/src/libm/k_tan.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_atan.lo: ../SDL2-2.28.5/src/libm/s_atan.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_copysign.lo: ../SDL2-2.28.5/src/libm/s_copysign.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_cos.lo: ../SDL2-2.28.5/src/libm/s_cos.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_fabs.lo: ../SDL2-2.28.5/src/libm/s_fabs.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_floor.lo: ../SDL2-2.28.5/src/libm/s_floor.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_scalbn.lo: ../SDL2-2.28.5/src/libm/s_scalbn.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_sin.lo: ../SDL2-2.28.5/src/libm/s_sin.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_tan.lo: ../SDL2-2.28.5/src/libm/s_tan.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_url.lo: ../SDL2-2.28.5/src/misc/SDL_url.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_power.lo: ../SDL2-2.28.5/src/power/SDL_power.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_d3dmath.lo: ../SDL2-2.28.5/src/render/SDL_d3dmath.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render.lo: ../SDL2-2.28.5/src/render/SDL_render.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_yuv_sw.lo: ../SDL2-2.28.5/src/render/SDL_yuv_sw.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_d3d.lo: ../SDL2-2.28.5/src/render/direct3d/SDL_render_d3d.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_d3d.lo: ../SDL2-2.28.5/src/render/direct3d/SDL_shaders_d3d.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_d3d11.lo: ../SDL2-2.28.5/src/render/direct3d11/SDL_render_d3d11.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_d3d11.lo: ../SDL2-2.28.5/src/render/direct3d11/SDL_shaders_d3d11.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_d3d12.lo: ../SDL2-2.28.5/src/render/direct3d12/SDL_render_d3d12.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_d3d12.lo: ../SDL2-2.28.5/src/render/direct3d12/SDL_shaders_d3d12.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_gl.lo: ../SDL2-2.28.5/src/render/opengl/SDL_render_gl.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_gl.lo: ../SDL2-2.28.5/src/render/opengl/SDL_shaders_gl.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_gles.lo: ../SDL2-2.28.5/src/render/opengles/SDL_render_gles.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_gles2.lo: ../SDL2-2.28.5/src/render/opengles2/SDL_render_gles2.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_gles2.lo: ../SDL2-2.28.5/src/render/opengles2/SDL_shaders_gles2.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_ps2.lo: ../SDL2-2.28.5/src/render/ps2/SDL_render_ps2.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_psp.lo: ../SDL2-2.28.5/src/render/psp/SDL_render_psp.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blendfillrect.lo: ../SDL2-2.28.5/src/render/software/SDL_blendfillrect.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blendline.lo: ../SDL2-2.28.5/src/render/software/SDL_blendline.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blendpoint.lo: ../SDL2-2.28.5/src/render/software/SDL_blendpoint.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_drawline.lo: ../SDL2-2.28.5/src/render/software/SDL_drawline.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_drawpoint.lo: ../SDL2-2.28.5/src/render/software/SDL_drawpoint.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_sw.lo: ../SDL2-2.28.5/src/render/software/SDL_render_sw.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rotate.lo: ../SDL2-2.28.5/src/render/software/SDL_rotate.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_triangle.lo: ../SDL2-2.28.5/src/render/software/SDL_triangle.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_vita_gxm.lo: ../SDL2-2.28.5/src/render/vitagxm/SDL_render_vita_gxm.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_vita_gxm_memory.lo: ../SDL2-2.28.5/src/render/vitagxm/SDL_render_vita_gxm_memory.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_vita_gxm_tools.lo: ../SDL2-2.28.5/src/render/vitagxm/SDL_render_vita_gxm_tools.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sensor.lo: ../SDL2-2.28.5/src/sensor/SDL_sensor.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_crc16.lo: ../SDL2-2.28.5/src/stdlib/SDL_crc16.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_crc32.lo: ../SDL2-2.28.5/src/stdlib/SDL_crc32.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_getenv.lo: ../SDL2-2.28.5/src/stdlib/SDL_getenv.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_iconv.lo: ../SDL2-2.28.5/src/stdlib/SDL_iconv.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_malloc.lo: ../SDL2-2.28.5/src/stdlib/SDL_malloc.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_mslibc.lo: ../SDL2-2.28.5/src/stdlib/SDL_mslibc.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_qsort.lo: ../SDL2-2.28.5/src/stdlib/SDL_qsort.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_stdlib.lo: ../SDL2-2.28.5/src/stdlib/SDL_stdlib.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_string.lo: ../SDL2-2.28.5/src/stdlib/SDL_string.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_strtokr.lo: ../SDL2-2.28.5/src/stdlib/SDL_strtokr.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_thread.lo: ../SDL2-2.28.5/src/thread/SDL_thread.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_timer.lo: ../SDL2-2.28.5/src/timer/SDL_timer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_RLEaccel.lo: ../SDL2-2.28.5/src/video/SDL_RLEaccel.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit.lo: ../SDL2-2.28.5/src/video/SDL_blit.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_0.lo: ../SDL2-2.28.5/src/video/SDL_blit_0.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_1.lo: ../SDL2-2.28.5/src/video/SDL_blit_1.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_A.lo: ../SDL2-2.28.5/src/video/SDL_blit_A.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_N.lo: ../SDL2-2.28.5/src/video/SDL_blit_N.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_auto.lo: ../SDL2-2.28.5/src/video/SDL_blit_auto.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_copy.lo: ../SDL2-2.28.5/src/video/SDL_blit_copy.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_slow.lo: ../SDL2-2.28.5/src/video/SDL_blit_slow.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_bmp.lo: ../SDL2-2.28.5/src/video/SDL_bmp.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_clipboard.lo: ../SDL2-2.28.5/src/video/SDL_clipboard.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_egl.lo: ../SDL2-2.28.5/src/video/SDL_egl.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_fillrect.lo: ../SDL2-2.28.5/src/video/SDL_fillrect.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_pixels.lo: ../SDL2-2.28.5/src/video/SDL_pixels.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rect.lo: ../SDL2-2.28.5/src/video/SDL_rect.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shape.lo: ../SDL2-2.28.5/src/video/SDL_shape.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_stretch.lo: ../SDL2-2.28.5/src/video/SDL_stretch.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_surface.lo: ../SDL2-2.28.5/src/video/SDL_surface.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_video.lo: ../SDL2-2.28.5/src/video/SDL_video.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_vulkan_utils.lo: ../SDL2-2.28.5/src/video/SDL_vulkan_utils.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_yuv.lo: ../SDL2-2.28.5/src/video/SDL_yuv.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/yuv_rgb.lo: ../SDL2-2.28.5/src/video/yuv2rgb/yuv_rgb.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_locale.lo: ../SDL2-2.28.5/src/locale/SDL_locale.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_nullevents.lo: ../SDL2-2.28.5/src/video/dummy/SDL_nullevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_nullframebuffer.lo: ../SDL2-2.28.5/src/video/dummy/SDL_nullframebuffer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_nullvideo.lo: ../SDL2-2.28.5/src/video/dummy/SDL_nullvideo.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_offscreenevents.lo: ../SDL2-2.28.5/src/video/offscreen/SDL_offscreenevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_offscreenframebuffer.lo: ../SDL2-2.28.5/src/video/offscreen/SDL_offscreenframebuffer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_offscreenopengles.lo: ../SDL2-2.28.5/src/video/offscreen/SDL_offscreenopengles.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_offscreenvideo.lo: ../SDL2-2.28.5/src/video/offscreen/SDL_offscreenvideo.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_offscreenwindow.lo: ../SDL2-2.28.5/src/video/offscreen/SDL_offscreenwindow.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_combined.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_combined.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_gamecube.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_gamecube.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_luna.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_luna.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_ps3.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_ps3.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_ps4.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_ps4.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_ps5.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_ps5.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_rumble.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_rumble.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_shield.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_shield.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_stadia.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_stadia.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_steam.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_steam.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_switch.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_switch.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_wii.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_wii.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_xbox360.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_xbox360.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_xbox360w.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_xbox360w.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapi_xboxone.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapi_xboxone.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hidapijoystick.lo: ../SDL2-2.28.5/src/joystick/hidapi/SDL_hidapijoystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hid.lo: ../SDL2-2.28.5/src/core/windows/SDL_hid.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_immdevice.lo: ../SDL2-2.28.5/src/core/windows/SDL_immdevice.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windows.lo: ../SDL2-2.28.5/src/core/windows/SDL_windows.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_xinput.lo: ../SDL2-2.28.5/src/core/windows/SDL_xinput.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysurl.lo: ../SDL2-2.28.5/src/misc/windows/SDL_sysurl.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syslocale.lo: ../SDL2-2.28.5/src/locale/windows/SDL_syslocale.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsclipboard.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsclipboard.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsevents.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsevents.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsframebuffer.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsframebuffer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowskeyboard.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowskeyboard.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsmessagebox.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsmessagebox.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsmodes.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsmodes.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsmouse.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsmouse.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsopengl.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsopengl.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsopengles.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsopengles.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsshape.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsshape.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsvideo.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsvideo.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsvulkan.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowsvulkan.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowswindow.lo: ../SDL2-2.28.5/src/video/windows/SDL_windowswindow.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dinputjoystick.lo: ../SDL2-2.28.5/src/joystick/windows/SDL_dinputjoystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rawinputjoystick.lo: ../SDL2-2.28.5/src/joystick/windows/SDL_rawinputjoystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windows_gaming_input.lo: ../SDL2-2.28.5/src/joystick/windows/SDL_windows_gaming_input.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowsjoystick.lo: ../SDL2-2.28.5/src/joystick/windows/SDL_windowsjoystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_xinputjoystick.lo: ../SDL2-2.28.5/src/joystick/windows/SDL_xinputjoystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dinputhaptic.lo: ../SDL2-2.28.5/src/haptic/windows/SDL_dinputhaptic.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowshaptic.lo: ../SDL2-2.28.5/src/haptic/windows/SDL_windowshaptic.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_xinputhaptic.lo: ../SDL2-2.28.5/src/haptic/windows/SDL_xinputhaptic.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowssensor.lo: ../SDL2-2.28.5/src/sensor/windows/SDL_windowssensor.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syspower.lo: ../SDL2-2.28.5/src/power/windows/SDL_syspower.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysfilesystem.lo: ../SDL2-2.28.5/src/filesystem/windows/SDL_sysfilesystem.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syscond_cv.lo: ../SDL2-2.28.5/src/thread/windows/SDL_syscond_cv.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysmutex.lo: ../SDL2-2.28.5/src/thread/windows/SDL_sysmutex.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syssem.lo: ../SDL2-2.28.5/src/thread/windows/SDL_syssem.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_systhread.lo: ../SDL2-2.28.5/src/thread/windows/SDL_systhread.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_systls.lo: ../SDL2-2.28.5/src/thread/windows/SDL_systls.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syscond.lo: ../SDL2-2.28.5/src/thread/generic/SDL_syscond.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_systimer.lo: ../SDL2-2.28.5/src/timer/windows/SDL_systimer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysloadso.lo: ../SDL2-2.28.5/src/loadso/windows/SDL_sysloadso.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_virtualjoystick.lo: ../SDL2-2.28.5/src/joystick/virtual/SDL_virtualjoystick.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dummyaudio.lo: ../SDL2-2.28.5/src/audio/dummy/SDL_dummyaudio.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/version.lo: ../SDL2-2.28.5/src/main/windows/version.rc $(objects)/.created
	$(RUN_CMD_RC)$(LIBTOOL) --mode=compile --tag=RC $(RC) -i $< -o $@

$(objects)/SDL_windows_main.lo: ../SDL2-2.28.5/src/main/windows/SDL_windows_main.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_test_assert.lo: ../SDL2-2.28.5/src/test/SDL_test_assert.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_common.lo: ../SDL2-2.28.5/src/test/SDL_test_common.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_compare.lo: ../SDL2-2.28.5/src/test/SDL_test_compare.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_crc32.lo: ../SDL2-2.28.5/src/test/SDL_test_crc32.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_font.lo: ../SDL2-2.28.5/src/test/SDL_test_font.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_fuzzer.lo: ../SDL2-2.28.5/src/test/SDL_test_fuzzer.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_harness.lo: ../SDL2-2.28.5/src/test/SDL_test_harness.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imageBlit.lo: ../SDL2-2.28.5/src/test/SDL_test_imageBlit.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imageBlitBlend.lo: ../SDL2-2.28.5/src/test/SDL_test_imageBlitBlend.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imageFace.lo: ../SDL2-2.28.5/src/test/SDL_test_imageFace.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imagePrimitives.lo: ../SDL2-2.28.5/src/test/SDL_test_imagePrimitives.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imagePrimitivesBlend.lo: ../SDL2-2.28.5/src/test/SDL_test_imagePrimitivesBlend.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_log.lo: ../SDL2-2.28.5/src/test/SDL_test_log.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_md5.lo: ../SDL2-2.28.5/src/test/SDL_test_md5.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_memory.lo: ../SDL2-2.28.5/src/test/SDL_test_memory.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_random.lo: ../SDL2-2.28.5/src/test/SDL_test_random.c $(objects)/.created
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

