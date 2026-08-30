# Install script for directory: /workspace/sdl32-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/i686-w64-mingw32-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/workspace/sdl32-build/libSDL2main.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/workspace/sdl32-build/libSDL2.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2mainTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2mainTargets.cmake"
         "/workspace/sdl32-build/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2mainTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2mainTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2mainTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/sdl32-build/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2mainTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/sdl32-build/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2mainTargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2staticTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2staticTargets.cmake"
         "/workspace/sdl32-build/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2staticTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2staticTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2staticTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/sdl32-build/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2staticTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/sdl32-build/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2staticTargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES
    "/workspace/sdl32-build/SDL2Config.cmake"
    "/workspace/sdl32-build/SDL2ConfigVersion.cmake"
    "/workspace/sdl32-src/cmake/sdlfind.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL2" TYPE FILE FILES
    "/workspace/sdl32-src/include/SDL.h"
    "/workspace/sdl32-src/include/SDL_assert.h"
    "/workspace/sdl32-src/include/SDL_atomic.h"
    "/workspace/sdl32-src/include/SDL_audio.h"
    "/workspace/sdl32-src/include/SDL_bits.h"
    "/workspace/sdl32-src/include/SDL_blendmode.h"
    "/workspace/sdl32-src/include/SDL_clipboard.h"
    "/workspace/sdl32-src/include/SDL_copying.h"
    "/workspace/sdl32-src/include/SDL_cpuinfo.h"
    "/workspace/sdl32-src/include/SDL_egl.h"
    "/workspace/sdl32-src/include/SDL_endian.h"
    "/workspace/sdl32-src/include/SDL_error.h"
    "/workspace/sdl32-src/include/SDL_events.h"
    "/workspace/sdl32-src/include/SDL_filesystem.h"
    "/workspace/sdl32-src/include/SDL_gamecontroller.h"
    "/workspace/sdl32-src/include/SDL_gesture.h"
    "/workspace/sdl32-src/include/SDL_guid.h"
    "/workspace/sdl32-src/include/SDL_haptic.h"
    "/workspace/sdl32-src/include/SDL_hidapi.h"
    "/workspace/sdl32-src/include/SDL_hints.h"
    "/workspace/sdl32-src/include/SDL_joystick.h"
    "/workspace/sdl32-src/include/SDL_keyboard.h"
    "/workspace/sdl32-src/include/SDL_keycode.h"
    "/workspace/sdl32-src/include/SDL_loadso.h"
    "/workspace/sdl32-src/include/SDL_locale.h"
    "/workspace/sdl32-src/include/SDL_log.h"
    "/workspace/sdl32-src/include/SDL_main.h"
    "/workspace/sdl32-src/include/SDL_messagebox.h"
    "/workspace/sdl32-src/include/SDL_metal.h"
    "/workspace/sdl32-src/include/SDL_misc.h"
    "/workspace/sdl32-src/include/SDL_mouse.h"
    "/workspace/sdl32-src/include/SDL_mutex.h"
    "/workspace/sdl32-src/include/SDL_name.h"
    "/workspace/sdl32-src/include/SDL_opengl.h"
    "/workspace/sdl32-src/include/SDL_opengl_glext.h"
    "/workspace/sdl32-src/include/SDL_opengles.h"
    "/workspace/sdl32-src/include/SDL_opengles2.h"
    "/workspace/sdl32-src/include/SDL_opengles2_gl2.h"
    "/workspace/sdl32-src/include/SDL_opengles2_gl2ext.h"
    "/workspace/sdl32-src/include/SDL_opengles2_gl2platform.h"
    "/workspace/sdl32-src/include/SDL_opengles2_khrplatform.h"
    "/workspace/sdl32-src/include/SDL_pixels.h"
    "/workspace/sdl32-src/include/SDL_platform.h"
    "/workspace/sdl32-src/include/SDL_power.h"
    "/workspace/sdl32-src/include/SDL_quit.h"
    "/workspace/sdl32-src/include/SDL_rect.h"
    "/workspace/sdl32-src/include/SDL_render.h"
    "/workspace/sdl32-src/include/SDL_rwops.h"
    "/workspace/sdl32-src/include/SDL_scancode.h"
    "/workspace/sdl32-src/include/SDL_sensor.h"
    "/workspace/sdl32-src/include/SDL_shape.h"
    "/workspace/sdl32-src/include/SDL_stdinc.h"
    "/workspace/sdl32-src/include/SDL_surface.h"
    "/workspace/sdl32-src/include/SDL_system.h"
    "/workspace/sdl32-src/include/SDL_syswm.h"
    "/workspace/sdl32-src/include/SDL_test.h"
    "/workspace/sdl32-src/include/SDL_test_assert.h"
    "/workspace/sdl32-src/include/SDL_test_common.h"
    "/workspace/sdl32-src/include/SDL_test_compare.h"
    "/workspace/sdl32-src/include/SDL_test_crc32.h"
    "/workspace/sdl32-src/include/SDL_test_font.h"
    "/workspace/sdl32-src/include/SDL_test_fuzzer.h"
    "/workspace/sdl32-src/include/SDL_test_harness.h"
    "/workspace/sdl32-src/include/SDL_test_images.h"
    "/workspace/sdl32-src/include/SDL_test_log.h"
    "/workspace/sdl32-src/include/SDL_test_md5.h"
    "/workspace/sdl32-src/include/SDL_test_memory.h"
    "/workspace/sdl32-src/include/SDL_test_random.h"
    "/workspace/sdl32-src/include/SDL_thread.h"
    "/workspace/sdl32-src/include/SDL_timer.h"
    "/workspace/sdl32-src/include/SDL_touch.h"
    "/workspace/sdl32-src/include/SDL_types.h"
    "/workspace/sdl32-src/include/SDL_version.h"
    "/workspace/sdl32-src/include/SDL_video.h"
    "/workspace/sdl32-src/include/SDL_vulkan.h"
    "/workspace/sdl32-src/include/begin_code.h"
    "/workspace/sdl32-src/include/close_code.h"
    "/workspace/sdl32-build/include/SDL2/SDL_revision.h"
    "/workspace/sdl32-build/include-config-/SDL2/SDL_config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/licenses/SDL2" TYPE FILE FILES "/workspace/sdl32-src/LICENSE.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/workspace/sdl32-build/sdl2.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/workspace/sdl32-build/sdl2-config")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/aclocal" TYPE FILE FILES "/workspace/sdl32-src/sdl2.m4")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/workspace/sdl32-build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
