# 3DS toolchain for devkitARM
# Requires DEVKITPRO and DEVKITARM environment variables to be set.
# export DEVKITPRO=/opt/devkitpro
# export DEVKITARM=/opt/devkitpro/devkitARM

if(NOT DEFINED ENV{DEVKITPRO})
  message(FATAL_ERROR "DEVKITPRO environment variable not set. Please set it to your devkitPro installation path.")
endif()
if(NOT DEFINED ENV{DEVKITARM})
  message(FATAL_ERROR "DEVKITARM environment variable not set. Please set it to your devkitARM installation path.")
endif()

set(DEVKITPRO $ENV{DEVKITPRO})
set(DEVKITARM $ENV{DEVKITARM})

# "Generic" is CMake's built-in bare-metal/embedded system name.
# We set NINTENDO_3DS so our CMakeLists.txt files can branch on it.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR armv6k)
set(NINTENDO_3DS TRUE CACHE BOOL "Building for Nintendo 3DS" FORCE)

set(CMAKE_C_COMPILER   ${DEVKITARM}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${DEVKITARM}/bin/arm-none-eabi-g++)
set(CMAKE_AR           ${DEVKITARM}/bin/arm-none-eabi-ar CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       ${DEVKITARM}/bin/arm-none-eabi-ranlib CACHE FILEPATH "Ranlib")

set(CMAKE_CROSSCOMPILING ON)

# Don't try to link a test executable during compiler detection — the CRT
# startup (3dsx_crt0.o) references symbols from libctru that aren't available
# at CMake's internal test-compile stage. Compile-only check is sufficient.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Only search in devkitPro paths for libs/headers, never on the host
set(CMAKE_FIND_ROOT_PATH
  ${DEVKITARM}
  ${DEVKITPRO}/portlibs/3ds
  ${DEVKITPRO}/libctru
)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ARMv6k flags matching devkitARM 3DS rules
set(ARCH_FLAGS "-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mword-relocations")
set(COMMON_FLAGS "${ARCH_FLAGS} -fomit-frame-pointer -ffunction-sections -DARM11 -D__3DS__")

set(CMAKE_C_FLAGS_INIT   "${COMMON_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -fno-rtti -fno-exceptions" CACHE STRING "")
set(CMAKE_ASM_FLAGS_INIT "${ARCH_FLAGS}" CACHE STRING "")

# 3DSX executable specs
set(CMAKE_EXE_LINKER_FLAGS_INIT "-specs=${DEVKITARM}/arm-none-eabi/lib/3dsx.specs ${ARCH_FLAGS} -Wl,--gc-sections" CACHE STRING "")

# Convenience variables for CMakeLists.txt files
set(CTRULIB   ${DEVKITPRO}/libctru)
set(PORTLIBS  ${DEVKITPRO}/portlibs/3ds)

include_directories(
  ${CTRULIB}/include
  ${PORTLIBS}/include
)
link_directories(
  ${CTRULIB}/lib
  ${PORTLIBS}/lib
)
