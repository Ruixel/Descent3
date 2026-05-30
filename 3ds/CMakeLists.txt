# 3DS platform library — replaces linux/ and ddio/ for the 3DS target.
# Start with a minimal stub that lets the project link; flesh out over time.

set(HEADERS
  ctr_platform.h
)

set(CPPS
  ctr_platform.cpp   # Main entry point / applet init
  ctr_ddio.cpp       # ddio stubs (input, file I/O)
  ctr_renderer.cpp   # renderer stubs (replaces OpenGL renderer)
  ctr_sound.cpp      # sound stubs
)

add_library(platform3ds STATIC ${HEADERS} ${CPPS})

target_include_directories(platform3ds PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  ${CTRULIB}/include
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
