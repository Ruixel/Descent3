// 3DS shim for PreInitD3Systems() from Descent3/init.cpp
// Pulls in only the parts that compile cleanly on 3DS.
// Expand this as more of the game is ported.

#include <stdio.h>
#include <cstdlib>

#include "args.h"
#include "init.h"
#include "mem.h"
#include "pserror.h"
#include "log.h"

// Globals referenced by PreInitD3Systems that normally live in other .cpp files
// Declare them here until those files are added to the build.
bool Use_file_xfer = true;
float Min_allowed_frametime = 0.0f;
float Mouselook_sensitivity = 1.0f;
float Mouse_sensitivity = 1.0f;

// grtext_Init stub — real version needs the renderer
void grtext_Init() {
  printf("[3DS] grtext_Init stub\n");
}


void PreInitD3Systems() {
  printf("[3DS] PreInitD3Systems() start\n");

  bool debugging = false;
#ifndef RELEASE
  debugging = (FindArg("-debug") != 0);
#endif

  printf("[3DS] before error_Init...\n");
  error_Init(debugging, "Descent3");
  printf("[3DS] after error_Init...\n");

  if (FindArg("-lowmem"))
    Mem_low_memory_mode = true;

  printf("[3DS] before mem_Init...\n");
  mem_Init();
  printf("[3DS] after mem_Init...\n");

  // Default framecap of 30 on 3DS (conservative)
  Min_allowed_frametime = (1.0f / 30.0f) * 1000.0f;
  printf("[3DS] framecap set to 30fps\n");

  printf("[3DS] grtext_Init...\n");
  grtext_Init();

  printf("[3DS] PreInitD3Systems() done!\n");
}
