#pragma once

#include "particle_group.h"
#include <SDL.h>

#if !defined(PG_ENABLE_ARM_NEON) && defined(__aarch64__)
#define PG_ENABLE_ARM_NEON 1
#endif

#if defined(__SSE2__)
#define PG_ENABLE_SSE_NEON 1
#elif PG_ENABLE_ARM_NEON
#define PG_ENABLE_SSE_NEON 1
#else
#define PG_ENABLE_SSE_NEON 0
#endif

int
_Has_AVX2();

int
_HasSSE_NEON();

void
_update_particles_avx2(ParticleGroup *group, float dt);

void
_update_particles_sse2(ParticleGroup *group, float dt);
