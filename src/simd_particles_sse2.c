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
    float *g_time = group->p_time.data;
    float *g_u_fac = group->u_fac.data;

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

        __m128 acc_v_2 = _mm_loadu_ps(g_acc + 4);
        __m128 vel_v_2 = _mm_loadu_ps(g_vel + 4);
        __m128 pos_v_2 = _mm_loadu_ps(g_pos + 4);

        __m128 time_v = _mm_loadu_ps(g_time);
        __m128 u_fac_v = _mm_loadu_ps(g_u_fac);

        acc_v = _mm_add_ps(acc_v, grav_v);
        acc_v_2 = _mm_add_ps(acc_v_2, grav_v);
        vel_v = _mm_add_ps(vel_v, _mm_mul_ps(acc_v, dt_v));
        vel_v_2 = _mm_add_ps(vel_v_2, _mm_mul_ps(acc_v_2, dt_v));
        pos_v = _mm_add_ps(pos_v, _mm_mul_ps(vel_v, dt_v));
        pos_v_2 = _mm_add_ps(pos_v_2, _mm_mul_ps(vel_v_2, dt_v));

        time_v = _mm_add_ps(time_v, _mm_mul_ps(u_fac_v, dt_v));

        _mm_storeu_ps(g_acc, acc_v);
        _mm_storeu_ps(g_vel, vel_v);
        _mm_storeu_ps(g_pos, pos_v);

        _mm_storeu_ps(g_acc + 4, acc_v_2);
        _mm_storeu_ps(g_vel + 4, vel_v_2);
        _mm_storeu_ps(g_pos + 4, pos_v_2);

        _mm_storeu_ps(g_time, time_v);

        g_pos += 8;
        g_vel += 8;
        g_acc += 8;
        g_time += 4;
        g_u_fac += 4;
    }

    for (i = 0; i < n_excess; i++) {
        *g_acc++ += group->gravity.x;
        *g_vel++ += *g_acc * dt;
        *g_pos++ += *g_vel * dt;
        *g_acc++ += group->gravity.y;
        *g_vel++ += *g_acc * dt;
        *g_pos++ += *g_vel * dt;

        g_time[i] += g_u_fac[i] * dt;
    }
}
#else
void
_update_particles_sse2(ParticleGroup *group, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || PG_ENABLE_ARM_NEON */