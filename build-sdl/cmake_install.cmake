# Install script for directory: /workspace/SDL2-2.28.5

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  set(CMAKE_OBJDUMP "/usr/bin/x86_64-w64-mingw32-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/workspace/build-sdl/libSDL2main.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/workspace/build-sdl/libSDL2.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2mainTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2mainTargets.cmake"
         "/workspace/build-sdl/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2mainTargets.cmake")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/build-sdl/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2mainTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/build-sdl/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2mainTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2staticTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2/SDL2staticTargets.cmake"
         "/workspace/build-sdl/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2staticTargets.cmake")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/build-sdl/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2staticTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES "/workspace/build-sdl/CMakeFiles/Export/f084604df1a27ef5b4fef7c7544737d1/SDL2staticTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2" TYPE FILE FILES
    "/workspace/build-sdl/SDL2Config.cmake"
    "/workspace/build-sdl/SDL2ConfigVersion.cmake"
    "/workspace/SDL2-2.28.5/cmake/sdlfind.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL2" TYPE FILE FILES
    "/workspace/SDL2-2.28.5/include/SDL.h"
    "/workspace/SDL2-2.28.5/include/SDL_assert.h"
    "/workspace/SDL2-2.28.5/include/SDL_atomic.h"
    "/workspace/SDL2-2.28.5/include/SDL_audio.h"
    "/workspace/SDL2-2.28.5/include/SDL_bits.h"
    "/workspace/SDL2-2.28.5/include/SDL_blendmode.h"
    "/workspace/SDL2-2.28.5/include/SDL_clipboard.h"
    "/workspace/SDL2-2.28.5/include/SDL_copying.h"
    "/workspace/SDL2-2.28.5/include/SDL_cpuinfo.h"
    "/workspace/SDL2-2.28.5/include/SDL_egl.h"
    "/workspace/SDL2-2.28.5/include/SDL_endian.h"
    "/workspace/SDL2-2.28.5/include/SDL_error.h"
    "/workspace/SDL2-2.28.5/include/SDL_events.h"
    "/workspace/SDL2-2.28.5/include/SDL_filesystem.h"
    "/workspace/SDL2-2.28.5/include/SDL_gamecontroller.h"
    "/workspace/SDL2-2.28.5/include/SDL_gesture.h"
    "/workspace/SDL2-2.28.5/include/SDL_guid.h"
    "/workspace/SDL2-2.28.5/include/SDL_haptic.h"
    "/workspace/SDL2-2.28.5/include/SDL_hidapi.h"
    "/workspace/SDL2-2.28.5/include/SDL_hints.h"
    "/workspace/SDL2-2.28.5/include/SDL_joystick.h"
    "/workspace/SDL2-2.28.5/include/SDL_keyboard.h"
    "/workspace/SDL2-2.28.5/include/SDL_keycode.h"
    "/workspace/SDL2-2.28.5/include/SDL_loadso.h"
    "/workspace/SDL2-2.28.5/include/SDL_locale.h"
    "/workspace/SDL2-2.28.5/include/SDL_log.h"
    "/workspace/SDL2-2.28.5/include/SDL_main.h"
    "/workspace/SDL2-2.28.5/include/SDL_messagebox.h"
    "/workspace/SDL2-2.28.5/include/SDL_metal.h"
    "/workspace/SDL2-2.28.5/include/SDL_misc.h"
    "/workspace/SDL2-2.28.5/include/SDL_mouse.h"
    "/workspace/SDL2-2.28.5/include/SDL_mutex.h"
    "/workspace/SDL2-2.28.5/include/SDL_name.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengl.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengl_glext.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengles.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengles2.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengles2_gl2.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengles2_gl2ext.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengles2_gl2platform.h"
    "/workspace/SDL2-2.28.5/include/SDL_opengles2_khrplatform.h"
    "/workspace/SDL2-2.28.5/include/SDL_pixels.h"
    "/workspace/SDL2-2.28.5/include/SDL_platform.h"
    "/workspace/SDL2-2.28.5/include/SDL_power.h"
    "/workspace/SDL2-2.28.5/include/SDL_quit.h"
    "/workspace/SDL2-2.28.5/include/SDL_rect.h"
    "/workspace/SDL2-2.28.5/include/SDL_render.h"
    "/workspace/SDL2-2.28.5/include/SDL_rwops.h"
    "/workspace/SDL2-2.28.5/include/SDL_scancode.h"
    "/workspace/SDL2-2.28.5/include/SDL_sensor.h"
    "/workspace/SDL2-2.28.5/include/SDL_shape.h"
    "/workspace/SDL2-2.28.5/include/SDL_stdinc.h"
    "/workspace/SDL2-2.28.5/include/SDL_surface.h"
    "/workspace/SDL2-2.28.5/include/SDL_system.h"
    "/workspace/SDL2-2.28.5/include/SDL_syswm.h"
    "/workspace/SDL2-2.28.5/include/SDL_test.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_assert.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_common.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_compare.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_crc32.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_font.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_fuzzer.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_harness.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_images.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_log.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_md5.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_memory.h"
    "/workspace/SDL2-2.28.5/include/SDL_test_random.h"
    "/workspace/SDL2-2.28.5/include/SDL_thread.h"
    "/workspace/SDL2-2.28.5/include/SDL_timer.h"
    "/workspace/SDL2-2.28.5/include/SDL_touch.h"
    "/workspace/SDL2-2.28.5/include/SDL_types.h"
    "/workspace/SDL2-2.28.5/include/SDL_version.h"
    "/workspace/SDL2-2.28.5/include/SDL_video.h"
    "/workspace/SDL2-2.28.5/include/SDL_vulkan.h"
    "/workspace/SDL2-2.28.5/include/begin_code.h"
    "/workspace/SDL2-2.28.5/include/close_code.h"
    "/workspace/build-sdl/include/SDL2/SDL_revision.h"
    "/workspace/build-sdl/include-config-release/SDL2/SDL_config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/licenses/SDL2" TYPE FILE FILES "/workspace/SDL2-2.28.5/LICENSE.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/workspace/build-sdl/sdl2.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/workspace/build-sdl/sdl2-config")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/aclocal" TYPE FILE FILES "/workspace/SDL2-2.28.5/sdl2.m4")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/workspace/build-sdl/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
