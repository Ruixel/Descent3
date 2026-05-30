#pragma once

// 3DS platform initialisation / teardown.
// Call ctr_platform_init() at the top of main(), ctr_platform_fini() at exit.

#ifdef __cplusplus
extern "C" {
#endif

void ctr_platform_init(void);
void ctr_platform_fini(void);

#ifdef __cplusplus
}
#endif
