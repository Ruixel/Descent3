# 3DS platform library — replaces linux/ and ddio/ for the 3DS target.
# Start with a minimal stub that lets the project link; flesh out over time.

# ---------------------------------------------------------------------------
# Build PICA200 vertex shaders with picasso
# ---------------------------------------------------------------------------
find_program(PICASSO picasso PATHS ${DEVKITPRO}/tools/bin REQUIRED)

set(SHADER_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/vshader_2d.v.pica
)

set(SHADER_HEADERS)
foreach(shader ${SHADER_SOURCES})
  get_filename_component(shader_name ${shader} NAME_WE)
  set(out_h   ${CMAKE_CURRENT_BINARY_DIR}/${shader_name}_shbin.h)
  set(out_bin ${CMAKE_CURRENT_BINARY_DIR}/${shader_name}_shbin.bin)
  add_custom_command(
    OUTPUT  ${out_h} ${out_bin}
    COMMAND ${PICASSO} -o ${out_bin} ${shader}
    # Convert binary to a C header with xxd-style output
    COMMAND ${CMAKE_COMMAND}
      -DINPUT=${out_bin}
      -DOUTPUT=${out_h}
      -DVARNAME=${shader_name}_shbin
      -P ${CMAKE_CURRENT_SOURCE_DIR}/bin2h.cmake
    DEPENDS ${shader}
    COMMENT "Compiling PICA shader ${shader_name}"
  )
  list(APPEND SHADER_HEADERS ${out_h})
endforeach()

set(HEADERS
  ctr_platform.h
  ${SHADER_HEADERS}
)

set(CPPS
  ctr_platform.cpp   # Main entry point / applet init
  ctr_ddio.cpp       # ddio stubs (input, file I/O)
  ctr_renderer.cpp   # citro3d 2D renderer
  ctr_sound.cpp      # sound stubs
  ctr_stubs.cpp      # misc subsystem stubs (sndlib arrays, UI, Osiris, etc.)
  ${CMAKE_SOURCE_DIR}/ddio/chrono_timer.cpp  # portable timer via std::chrono
)

add_library(platform3ds STATIC ${HEADERS} ${CPPS})
target_include_directories(platform3ds PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(platform3ds PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  ${CTRULIB}/include
  ${CMAKE_SOURCE_DIR}/sndlib
  ${CMAKE_SOURCE_DIR}/Descent3
  ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/ui
  ${CMAKE_SOURCE_DIR}/grtext
  ${PROJECT_BINARY_DIR}/lib
)

target_link_libraries(platform3ds PUBLIC
  citro3d
  ctru
)

# Expose this library under the same alias names the rest of the project
# expects, so we don't have to patch every CMakeLists.txt that links these.
# Each name here is a module that is either stubbed inside platform3ds or
# not yet ported — add real subdirectories and remove the alias as each is done.
add_library(ddio         ALIAS platform3ds)
add_library(linux        ALIAS platform3ds)
add_library(grtext       ALIAS platform3ds)
add_library(manage       ALIAS platform3ds)
add_library(model        ALIAS platform3ds)
add_library(module       ALIAS platform3ds)
add_library(music        ALIAS platform3ds)
add_library(networking   ALIAS platform3ds)
add_library(physics      ALIAS platform3ds)
add_library(renderer     ALIAS platform3ds)
add_library(rtperformance ALIAS platform3ds)
add_library(sndlib       ALIAS platform3ds)
add_library(stream_audio ALIAS platform3ds)
add_library(ui           ALIAS platform3ds)
add_library(libmve       ALIAS platform3ds)
add_library(inetfile     ALIAS platform3ds)
