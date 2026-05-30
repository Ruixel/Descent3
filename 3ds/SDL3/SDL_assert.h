// 3DS stub for SDL3/SDL_assert.h
// SDL_assert maps to a simple abort() on assertion failure.
#pragma once
#include <assert.h>
#define SDL_assert(cond)       assert(cond)
#define SDL_TriggerBreakpoint() __builtin_trap()
