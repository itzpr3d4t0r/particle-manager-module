#include "include/simd_common.h"

#if PG_ENABLE_ARM_NEON
// sse2neon.h is from here: https://github.com/DLTcollab/sse2neon
#include "include/sse2neon.h"
#endif /* PG_ENABLE_ARM_NEON */

#define BAD_SSE2_FUNCTION_CALL                                               \
    printf(                                                                  \
        "Fatal Error: Attempted calling an SSE2 function when both compile " \
        "time and runtime support is missing. If you are seeing this "       \
        "message, you have stumbled across a bug, please report it "         \
        "to the devs!");                                                     \
    Py_Exit(1);

int
_HasSSE_NEON()
{
#if defined(__SSE2__)
    return SDL_HasSSE2();
#elif PG_ENABLE_ARM_NEON
    return SDL_HasNEON();
#else
    return 0;
#endif
}

#if defined(__SSE2__) || defined(PG_ENABLE_ARM_NEON)
void
_update_particles_sse2(ParticleGroup *group, float dt)
{
    float *g_pos = group->p_pos.data;
    float *g_vel = group->p_vel.data;
    float *g_acc = group->p_acc.data;

    Py_ssize_t i;
    const Py_ssize_t n_iters_4 = group->n_particles / 4;
    const Py_ssize_t n_excess = group->n_particles % 4;
    const __m128 dt_v = _mm_set1_ps(dt);
    const __m128 grav_v = _mm_set_ps(group->gravity.y, group->gravity.x,
                                     group->gravity.y, group->gravity.x);

    for (i = 0; i < n_iters_4; i++) {
        __m128 acc_v = _mm_loadu_ps(g_acc);
        __m128 vel_v = _mm_loadu_ps(g_vel);
        __m128 pos_v = _mm_loadu_ps(g_pos);

        acc_v = _mm_add_ps(acc_v, grav_v);
        vel_v = _mm_add_ps(vel_v, _mm_mul_ps(acc_v, dt_v));
        pos_v = _mm_add_ps(pos_v, _mm_mul_ps(vel_v, dt_v));

        _mm_storeu_ps(g_acc, acc_v);
        _mm_storeu_ps(g_vel, vel_v);
        _mm_storeu_ps(g_pos, pos_v);

        g_pos += 4;
        g_vel += 4;
        g_acc += 4;
    }

    for (i = 0; i < n_excess; i++) {
        g_acc[i] += group->gravity.y;
        g_vel[i] += g_acc[i] * dt;
        g_pos[i] += g_vel[i] * dt;
    }
}
#else
void
_update_particles_sse2(ParticleGroup *group, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || PG_ENABLE_ARM_NEON */