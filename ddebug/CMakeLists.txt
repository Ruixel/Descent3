set(HEADERS
  debug.h
  mono.h
  pserror.h
)
set(CPPS
  debug.cpp
  error.cpp
  $<$<PLATFORM_ID:Darwin,Linux,OpenBSD>:
    lnxdebug.cpp
    lnxmono.cpp
  >
  $<$<PLATFORM_ID:Windows>:
    windebug.cpp
    winmono.cpp
  >
  $<$<BOOL:${NINTENDO_3DS}>:
    ${CMAKE_SOURCE_DIR}/3ds/ctr_debug.cpp
  >
)

add_library(ddebug STATIC ${HEADERS} ${CPPS})
add_dependencies(ddebug get_git_hash)
target_include_directories(ddebug PUBLIC
  $<BUILD_INTERFACE:
  ${PROJECT_SOURCE_DIR}/ddebug
  >
  PRIVATE ${PROJECT_BINARY_DIR}/lib   # For d3_version.h
  $<$<NOT:$<BOOL:${NINTENDO_3DS}>>:${SDL3_INCLUDE_DIRS}>
)
target_link_libraries(ddebug PRIVATE
  $<$<PLATFORM_ID:Windows>:
    mem
    misc
  >
  PUBLIC
  $<$<NOT:$<BOOL:${NINTENDO_3DS}>>:SDL3::SDL3>
  logger
)
