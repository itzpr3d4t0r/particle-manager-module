#include "include/simd_common.h"

#if ENABLE_ARM_NEON
// sse2neon.h is from here: https://github.com/DLTcollab/sse2neon
#include "include/sse2neon.h"
#endif /* ENABLE_ARM_NEON */

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
#elif ENABLE_ARM_NEON
    return SDL_HasNEON();
#else
    return 0;
#endif
}

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
UDB_all_sse2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict accelerations_x = block->accelerations_x.data;
    float *restrict accelerations_y = block->accelerations_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_4 = block->particles_count / 4;
    const int n_excess = block->particles_count % 4;
    const __m128 dt_v = _mm_set1_ps(dt);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 ax = _mm_loadu_ps(accelerations_x);
        __m128 ay = _mm_loadu_ps(accelerations_y);
        __m128 vx = _mm_loadu_ps(velocities_x);
        __m128 vy = _mm_loadu_ps(velocities_y);
        __m128 px = _mm_loadu_ps(positions_x);
        __m128 py = _mm_loadu_ps(positions_y);
        __m128 t = _mm_loadu_ps(lifetimes);

        vx = _mm_add_ps(vx, _mm_mul_ps(ax, dt_v));
        vy = _mm_add_ps(vy, _mm_mul_ps(ay, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_storeu_ps(velocities_x, vx);
        _mm_storeu_ps(velocities_y, vy);
        _mm_storeu_ps(positions_x, px);
        _mm_storeu_ps(positions_y, py);
        _mm_storeu_ps(lifetimes, t);

        accelerations_x += 4;
        accelerations_y += 4;
        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    for (i = 0; i < n_excess; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;
        lifetimes[i] -= dt;
    }
}
#else
void
UDB_all_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
UDB_no_acceleration_sse2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_4 = block->particles_count / 4;
    const int n_excess = block->particles_count % 4;
    const __m128 dt_v = _mm_set1_ps(dt);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 vx = _mm_loadu_ps(velocities_x);
        __m128 vy = _mm_loadu_ps(velocities_y);
        __m128 px = _mm_loadu_ps(positions_x);
        __m128 py = _mm_loadu_ps(positions_y);
        __m128 t = _mm_loadu_ps(lifetimes);

        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_storeu_ps(positions_x, px);
        _mm_storeu_ps(positions_y, py);
        _mm_storeu_ps(lifetimes, t);

        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    for (i = 0; i < n_excess; i++) {
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;
        lifetimes[i] -= dt;
    }
}
#else
void
UDB_no_acceleration_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
UDB_acceleration_x_sse2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict accelerations_x = block->accelerations_x.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_4 = block->particles_count / 4;
    const int n_excess = block->particles_count % 4;
    const __m128 dt_v = _mm_set1_ps(dt);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 ax = _mm_loadu_ps(accelerations_x);
        __m128 vx = _mm_loadu_ps(velocities_x);
        __m128 vy = _mm_loadu_ps(velocities_y);
        __m128 px = _mm_loadu_ps(positions_x);
        __m128 py = _mm_loadu_ps(positions_y);
        __m128 t = _mm_loadu_ps(lifetimes);

        vx = _mm_add_ps(vx, _mm_mul_ps(ax, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_storeu_ps(velocities_x, vx);
        _mm_storeu_ps(positions_x, px);
        _mm_storeu_ps(positions_y, py);
        _mm_storeu_ps(lifetimes, t);

        accelerations_x += 4;
        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    for (i = 0; i < n_excess; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;
        lifetimes[i] -= dt;
    }
}
#else
void
UDB_acceleration_x_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
UDB_acceleration_y_sse2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict accelerations_y = block->accelerations_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_4 = block->particles_count / 4;
    const int n_excess = block->particles_count % 4;
    const __m128 dt_v = _mm_set1_ps(dt);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 ay = _mm_loadu_ps(accelerations_y);
        __m128 vx = _mm_loadu_ps(velocities_x);
        __m128 vy = _mm_loadu_ps(velocities_y);
        __m128 px = _mm_loadu_ps(positions_x);
        __m128 py = _mm_loadu_ps(positions_y);
        __m128 t = _mm_loadu_ps(lifetimes);

        vy = _mm_add_ps(vy, _mm_mul_ps(ay, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_storeu_ps(velocities_y, vy);
        _mm_storeu_ps(positions_x, px);
        _mm_storeu_ps(positions_y, py);
        _mm_storeu_ps(lifetimes, t);

        accelerations_y += 4;
        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    for (i = 0; i < n_excess; i++) {
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;
        lifetimes[i] -= dt;
    }
}
#else
void
UDB_acceleration_y_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
update_indices_sse2(DataBlock *block)
{
    float *restrict lifetimes = block->lifetimes.data;
    float *restrict max_lifetimes = block->max_lifetimes.data;
    int *restrict indices = block->animation_indices;

    const int n_iters_4 = block->particles_count / 4;
    const int n_excess = block->particles_count % 4;
    const __m128 one_v = _mm_set1_ps(1.0f);
    const __m128 num_frames_v = _mm_set1_ps((float)(block->num_frames));

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 t = _mm_loadu_ps(lifetimes);
        __m128 max_t = _mm_loadu_ps(max_lifetimes);

        __m128i idx = _mm_cvttps_epi32(
            _mm_mul_ps(_mm_sub_ps(one_v, _mm_div_ps(t, max_t)), num_frames_v));

        _mm_storeu_si128((__m128i *)indices, idx);

        lifetimes += 4;
        max_lifetimes += 4;
        indices += 4;
    }

    for (i = 0; i < n_excess; i++) {
        indices[i] =
            (int)((1.0f - lifetimes[i] / max_lifetimes[i]) * block->num_frames);
    }
}
#else
void
update_indices_sse2(DataBlock *block)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */
