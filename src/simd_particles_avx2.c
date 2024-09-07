#include "include/simd_common.h"

#if defined(HAVE_IMMINTRIN_H) && !defined(SDL_DISABLE_IMMINTRIN_H)
#include <immintrin.h>
#endif /* defined(HAVE_IMMINTRIN_H) && !defined(SDL_DISABLE_IMMINTRIN_H) */

#define BAD_AVX2_FUNCTION_CALL                                               \
    printf(                                                                  \
        "Fatal Error: Attempted calling an AVX2 function when both compile " \
        "time and runtime support is missing. If you are seeing this "       \
        "message, you have stumbled across a bug, please report it "         \
        "to the devs!");                                                     \
    Py_Exit(1);

int
_Has_AVX2()
{
#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
    return SDL_HasAVX2();
#else
    return 0;
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          !defined(SDL_DISABLE_IMMINTRIN_H) */
}

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
_update_particles_avx2(ParticleGroup *group, float dt)
{
    const __m256 dt_v = _mm256_set1_ps(dt);

    float *g_pos = group->p_pos.data;
    float *g_vel = group->p_vel.data;
    float *g_acc = group->p_acc.data;
    float *g_time = group->p_time.data;
    float *g_u_fac = group->u_fac.data;

    Py_ssize_t i;
    const Py_ssize_t n_iters_8 = group->n_particles / 8;
    const Py_ssize_t n_excess = group->n_particles % 8;
    const __m256 grav_v = _mm256_set_ps(
        group->gravity.y, group->gravity.x, group->gravity.y, group->gravity.x,
        group->gravity.y, group->gravity.x, group->gravity.y, group->gravity.x);

    for (i = 0; i < n_iters_8; i++) {
        __m256 acc_v = _mm256_loadu_ps(g_acc);
        __m256 vel_v = _mm256_loadu_ps(g_vel);
        __m256 pos_v = _mm256_loadu_ps(g_pos);

        __m256 acc_v_2 = _mm256_loadu_ps(g_acc + 8);
        __m256 vel_v_2 = _mm256_loadu_ps(g_vel + 8);
        __m256 pos_v_2 = _mm256_loadu_ps(g_pos + 8);

        __m256 time_v = _mm256_loadu_ps(g_time);
        __m256 u_fac_v = _mm256_loadu_ps(g_u_fac);

        acc_v = _mm256_add_ps(acc_v, grav_v);
        acc_v_2 = _mm256_add_ps(acc_v_2, grav_v);
        vel_v = _mm256_add_ps(vel_v, _mm256_mul_ps(acc_v, dt_v));
        vel_v_2 = _mm256_add_ps(vel_v_2, _mm256_mul_ps(acc_v_2, dt_v));
        pos_v = _mm256_add_ps(pos_v, _mm256_mul_ps(vel_v, dt_v));
        pos_v_2 = _mm256_add_ps(pos_v_2, _mm256_mul_ps(vel_v_2, dt_v));

        time_v = _mm256_add_ps(time_v, _mm256_mul_ps(u_fac_v, dt_v));

        _mm256_storeu_ps(g_acc, acc_v);
        _mm256_storeu_ps(g_acc + 8, acc_v_2);

        _mm256_storeu_ps(g_vel, vel_v);
        _mm256_storeu_ps(g_vel + 8, vel_v_2);

        _mm256_storeu_ps(g_pos, pos_v);
        _mm256_storeu_ps(g_pos + 8, pos_v_2);

        _mm256_storeu_ps(g_time, time_v);

        g_pos += 16;
        g_vel += 16;
        g_acc += 16;

        g_time += 8;
        g_u_fac += 8;
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

    g_pos = group->p_pos.data;
    g_vel = group->p_vel.data;
    g_acc = group->p_acc.data;
    g_time = group->p_time.data;
    g_u_fac = group->u_fac.data;

    i = 0;
    while (i < group->n_particles) {
        if ((Py_ssize_t)g_time[i] > group->n_img_frames[0] - 1) {
            if (i != group->n_particles - 1) {
                g_pos[i * 2] = g_pos[(group->n_particles - 1) * 2];
                g_pos[i * 2 + 1] = g_pos[(group->n_particles - 1) * 2 + 1];

                g_vel[i * 2] = g_vel[(group->n_particles - 1) * 2];
                g_vel[i * 2 + 1] = g_vel[(group->n_particles - 1) * 2 + 1];

                g_acc[i * 2] = g_acc[(group->n_particles - 1) * 2];
                g_acc[i * 2 + 1] = g_acc[(group->n_particles - 1) * 2 + 1];

                g_time[i] = g_time[group->n_particles - 1];
                g_u_fac[i] = g_u_fac[group->n_particles - 1];
            }
            group->n_particles--;
        }
        else
            i++;
    }
}
#else
void
_update_particles_avx2(ParticleGroup *group, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */
