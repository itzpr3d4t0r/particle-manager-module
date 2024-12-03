#pragma once

#include <SDL.h>
#include "data_block.h"

#if !defined(ENABLE_ARM_NEON) && defined(__aarch64__)
#define ENABLE_ARM_NEON 1
#endif

#if defined(__SSE2__)
#define ENABLE_SSE_NEON 1
#elif ENABLE_ARM_NEON
#define ENABLE_SSE_NEON 1
#else
#define ENABLE_SSE_NEON 0
#endif

int
_Has_AVX2();

int
_HasSSE_NEON();

/* =============| AVX2 |============= */

void
update_with_acceleration_avx2(DataBlock *block, float dt);

void
update_with_no_acceleration_avx2(DataBlock *block, float dt);

void
update_with_acceleration_x_avx2(DataBlock *block, float dt);

void
update_with_acceleration_y_avx2(DataBlock *block, float dt);

void
update_indices_avx2(DataBlock *block);

void
blit_fragments_add_avx2(FragmentationMap *frag_map, PyObject **animation,
                        int dst_skip);

/* =============| SSE2 |============= */

void
update_with_acceleration_sse2(DataBlock *block, float dt);

void
update_with_no_acceleration_sse2(DataBlock *block, float dt);

void
update_with_acceleration_x_sse2(DataBlock *block, float dt);

void
update_with_acceleration_y_sse2(DataBlock *block, float dt);

void
update_indices_sse2(DataBlock *block);

void
blit_fragments_add_sse2(FragmentationMap *frag_map, PyObject **animation,
                        int dst_skip);
