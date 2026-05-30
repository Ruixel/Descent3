# bin2h.cmake — converts a binary file to a C header.
# Called by CMake with:
#   cmake -DINPUT=foo.bin -DOUTPUT=foo_shbin.h -DVARNAME=foo_shbin -P bin2h.cmake

file(READ "${INPUT}" data HEX)
string(LENGTH "${data}" hex_len)
math(EXPR byte_count "${hex_len} / 2")

# Split hex string into comma-separated 0xNN bytes
set(bytes "")
set(i 0)
while(i LESS ${hex_len})
  string(SUBSTRING "${data}" ${i} 2 byte)
  if(bytes STREQUAL "")
    set(bytes "0x${byte}")
  else()
    set(bytes "${bytes}, 0x${byte}")
  endif()
  math(EXPR i "${i} + 2")
endwhile()

file(WRITE "${OUTPUT}"
"#pragma once
#include <stdint.h>
// Auto-generated from ${INPUT} — do not edit.
static const uint8_t ${VARNAME}[] = { ${bytes} };
static const uint32_t ${VARNAME}_size = ${byte_count};
")
