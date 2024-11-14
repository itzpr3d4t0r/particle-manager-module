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
    const __m128i load_mask = _mm_set_epi32(
        0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0, n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 ax = _mm_maskload_ps(accelerations_x, load_mask);
        __m128 ay = _mm_maskload_ps(accelerations_y, load_mask);
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        vx = _mm_add_ps(vx, _mm_mul_ps(ax, dt_v));
        vy = _mm_add_ps(vy, _mm_mul_ps(ay, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(velocities_x, load_mask, vx);
        _mm_maskstore_ps(velocities_y, load_mask, vy);
        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);

        accelerations_x += 4;
        accelerations_y += 4;
        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    if (n_excess) {
        __m128 ax = _mm_maskload_ps(accelerations_x, load_mask);
        __m128 ay = _mm_maskload_ps(accelerations_y, load_mask);
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        vx = _mm_add_ps(vx, _mm_mul_ps(ax, dt_v));
        vy = _mm_add_ps(vy, _mm_mul_ps(ay, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(velocities_x, load_mask, vx);
        _mm_maskstore_ps(velocities_y, load_mask, vy);
        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
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
    const __m128i load_mask = _mm_set_epi32(
        0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0, n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);

        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    if (n_excess) {
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));

        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);

        _mm_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
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
    const __m128i load_mask = _mm_set_epi32(
        0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0, n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 ax = _mm_maskload_ps(accelerations_x, load_mask);
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        vx = _mm_add_ps(vx, _mm_mul_ps(ax, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(velocities_x, load_mask, vx);
        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);

        accelerations_x += 4;
        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    if (n_excess) {
        __m128 ax = _mm_maskload_ps(accelerations_x, load_mask);
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        vx = _mm_add_ps(vx, _mm_mul_ps(ax, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(velocities_x, load_mask, vx);
        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
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
    const __m128i load_mask = _mm_set_epi32(
        0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0, n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_4; i++) {
        __m128 ay = _mm_maskload_ps(accelerations_y, load_mask);
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        vy = _mm_add_ps(vy, _mm_mul_ps(ay, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(velocities_y, load_mask, vy);
        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);

        accelerations_y += 4;
        velocities_x += 4;
        velocities_y += 4;
        positions_x += 4;
        positions_y += 4;
        lifetimes += 4;
    }

    if (n_excess) {
        __m128 ay = _mm_maskload_ps(accelerations_y, load_mask);
        __m128 vx = _mm_maskload_ps(velocities_x, load_mask);
        __m128 vy = _mm_maskload_ps(velocities_y, load_mask);
        __m128 px = _mm_maskload_ps(positions_x, load_mask);
        __m128 py = _mm_maskload_ps(positions_y, load_mask);
        __m128 t = _mm_maskload_ps(lifetimes, load_mask);

        vy = _mm_add_ps(vy, _mm_mul_ps(ay, dt_v));
        px = _mm_add_ps(px, _mm_mul_ps(vx, dt_v));
        py = _mm_add_ps(py, _mm_mul_ps(vy, dt_v));
        t = _mm_sub_ps(t, dt_v);

        _mm_maskstore_ps(velocities_y, load_mask, vy);
        _mm_maskstore_ps(positions_x, load_mask, px);
        _mm_maskstore_ps(positions_y, load_mask, py);
        _mm_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
}
#else
void
UDB_acceleration_y_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */
