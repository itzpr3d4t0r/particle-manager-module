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
UDB_all_avx2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict accelerations_x = block->accelerations_x.data;
    float *restrict accelerations_y = block->accelerations_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_8 = block->particles_count / 8;
    const int n_excess = block->particles_count % 8;
    const __m256 dt_v = _mm256_set1_ps(dt);
    const __m256i load_mask = _mm256_set_epi32(
        0, n_excess > 6 ? -1 : 0, n_excess > 5 ? -1 : 0, n_excess > 4 ? -1 : 0,
        n_excess > 3 ? -1 : 0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0,
        n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_8; i++) {
        __m256 ax = _mm256_loadu_ps(accelerations_x);
        __m256 ay = _mm256_loadu_ps(accelerations_y);
        __m256 vx = _mm256_loadu_ps(velocities_x);
        __m256 vy = _mm256_loadu_ps(velocities_y);
        __m256 px = _mm256_loadu_ps(positions_x);
        __m256 py = _mm256_loadu_ps(positions_y);
        __m256 t = _mm256_loadu_ps(lifetimes);

        vx = _mm256_add_ps(vx, _mm256_mul_ps(ax, dt_v));
        vy = _mm256_add_ps(vy, _mm256_mul_ps(ay, dt_v));

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_storeu_ps(velocities_x, vx);
        _mm256_storeu_ps(velocities_y, vy);
        _mm256_storeu_ps(positions_x, px);
        _mm256_storeu_ps(positions_y, py);
        _mm256_storeu_ps(lifetimes, t);

        accelerations_x += 8;
        accelerations_y += 8;
        velocities_x += 8;
        velocities_y += 8;
        positions_x += 8;
        positions_y += 8;
        lifetimes += 8;
    }

    if (n_excess) {
        __m256 ax = _mm256_maskload_ps(accelerations_x, load_mask);
        __m256 ay = _mm256_maskload_ps(accelerations_y, load_mask);
        __m256 vx = _mm256_maskload_ps(velocities_x, load_mask);
        __m256 vy = _mm256_maskload_ps(velocities_y, load_mask);
        __m256 px = _mm256_maskload_ps(positions_x, load_mask);
        __m256 py = _mm256_maskload_ps(positions_y, load_mask);
        __m256 t = _mm256_maskload_ps(lifetimes, load_mask);

        vx = _mm256_add_ps(vx, _mm256_mul_ps(ax, dt_v));
        vy = _mm256_add_ps(vy, _mm256_mul_ps(ay, dt_v));

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_maskstore_ps(velocities_x, load_mask, vx);
        _mm256_maskstore_ps(velocities_y, load_mask, vy);
        _mm256_maskstore_ps(positions_x, load_mask, px);
        _mm256_maskstore_ps(positions_y, load_mask, py);
        _mm256_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
}
#else
void
UDB_all_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
UDB_no_acceleration_avx2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    int i;
    const int n_iters_8 = block->particles_count / 8;
    const int n_excess = block->particles_count % 8;
    const __m256 dt_v = _mm256_set1_ps(dt);
    const __m256i load_mask = _mm256_set_epi32(
        0, n_excess > 6 ? -1 : 0, n_excess > 5 ? -1 : 0, n_excess > 4 ? -1 : 0,
        n_excess > 3 ? -1 : 0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0,
        n_excess > 0 ? -1 : 0);

    for (i = 0; i < n_iters_8; i++) {
        __m256 vx = _mm256_loadu_ps(velocities_x);
        __m256 vy = _mm256_loadu_ps(velocities_y);
        __m256 px = _mm256_loadu_ps(positions_x);
        __m256 py = _mm256_loadu_ps(positions_y);
        __m256 t = _mm256_loadu_ps(lifetimes);

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_storeu_ps(positions_x, px);
        _mm256_storeu_ps(positions_y, py);
        _mm256_storeu_ps(lifetimes, t);

        velocities_x += 8;
        velocities_y += 8;
        positions_x += 8;
        positions_y += 8;
        lifetimes += 8;
    }

    if (n_excess) {
        __m256 vx = _mm256_maskload_ps(velocities_x, load_mask);
        __m256 vy = _mm256_maskload_ps(velocities_y, load_mask);
        __m256 px = _mm256_maskload_ps(positions_x, load_mask);
        __m256 py = _mm256_maskload_ps(positions_y, load_mask);
        __m256 t = _mm256_maskload_ps(lifetimes, load_mask);

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_maskstore_ps(positions_x, load_mask, px);
        _mm256_maskstore_ps(positions_y, load_mask, py);
        _mm256_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
}
#else
void
UDB_no_acceleration_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
UDB_acceleration_x_avx2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict accelerations_x = block->accelerations_x.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_8 = block->particles_count / 8;
    const int n_excess = block->particles_count % 8;
    const __m256 dt_v = _mm256_set1_ps(dt);
    const __m256i load_mask = _mm256_set_epi32(
        0, n_excess > 6 ? -1 : 0, n_excess > 5 ? -1 : 0, n_excess > 4 ? -1 : 0,
        n_excess > 3 ? -1 : 0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0,
        n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_8; i++) {
        __m256 ax = _mm256_loadu_ps(accelerations_x);
        __m256 vx = _mm256_loadu_ps(velocities_x);
        __m256 vy = _mm256_loadu_ps(velocities_y);
        __m256 px = _mm256_loadu_ps(positions_x);
        __m256 py = _mm256_loadu_ps(positions_y);
        __m256 t = _mm256_loadu_ps(lifetimes);

        vx = _mm256_add_ps(vx, _mm256_mul_ps(ax, dt_v));

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_storeu_ps(velocities_x, vx);
        _mm256_storeu_ps(positions_x, px);
        _mm256_storeu_ps(positions_y, py);
        _mm256_storeu_ps(lifetimes, t);

        accelerations_x += 8;
        velocities_x += 8;
        velocities_y += 8;
        positions_x += 8;
        positions_y += 8;
        lifetimes += 8;
    }

    if (n_excess) {
        __m256 ax = _mm256_maskload_ps(accelerations_x, load_mask);
        __m256 vx = _mm256_maskload_ps(velocities_x, load_mask);
        __m256 vy = _mm256_maskload_ps(velocities_y, load_mask);
        __m256 px = _mm256_maskload_ps(positions_x, load_mask);
        __m256 py = _mm256_maskload_ps(positions_y, load_mask);
        __m256 t = _mm256_maskload_ps(lifetimes, load_mask);

        vx = _mm256_add_ps(vx, _mm256_mul_ps(ax, dt_v));

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_maskstore_ps(velocities_x, load_mask, vx);
        _mm256_maskstore_ps(positions_x, load_mask, px);
        _mm256_maskstore_ps(positions_y, load_mask, py);
        _mm256_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
}
#else
void
UDB_acceleration_x_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
UDB_acceleration_y_avx2(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict accelerations_y = block->accelerations_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    const int n_iters_8 = block->particles_count / 8;
    const int n_excess = block->particles_count % 8;
    const __m256 dt_v = _mm256_set1_ps(dt);
    const __m256i load_mask = _mm256_set_epi32(
        0, n_excess > 6 ? -1 : 0, n_excess > 5 ? -1 : 0, n_excess > 4 ? -1 : 0,
        n_excess > 3 ? -1 : 0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0,
        n_excess > 0 ? -1 : 0);

    int i;

    for (i = 0; i < n_iters_8; i++) {
        __m256 ay = _mm256_loadu_ps(accelerations_y);
        __m256 vx = _mm256_loadu_ps(velocities_x);
        __m256 vy = _mm256_loadu_ps(velocities_y);
        __m256 px = _mm256_loadu_ps(positions_x);
        __m256 py = _mm256_loadu_ps(positions_y);
        __m256 t = _mm256_loadu_ps(lifetimes);

        vy = _mm256_add_ps(vy, _mm256_mul_ps(ay, dt_v));

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_storeu_ps(velocities_y, vy);
        _mm256_storeu_ps(positions_x, px);
        _mm256_storeu_ps(positions_y, py);
        _mm256_storeu_ps(lifetimes, t);

        accelerations_y += 8;
        velocities_x += 8;
        velocities_y += 8;
        positions_x += 8;
        positions_y += 8;
        lifetimes += 8;
    }

    if (n_excess) {
        __m256 ay = _mm256_maskload_ps(accelerations_y, load_mask);
        __m256 vx = _mm256_maskload_ps(velocities_x, load_mask);
        __m256 vy = _mm256_maskload_ps(velocities_y, load_mask);
        __m256 px = _mm256_maskload_ps(positions_x, load_mask);
        __m256 py = _mm256_maskload_ps(positions_y, load_mask);
        __m256 t = _mm256_maskload_ps(lifetimes, load_mask);

        vy = _mm256_add_ps(vy, _mm256_mul_ps(ay, dt_v));

        px = _mm256_add_ps(px, _mm256_mul_ps(vx, dt_v));
        py = _mm256_add_ps(py, _mm256_mul_ps(vy, dt_v));

        t = _mm256_sub_ps(t, dt_v);

        _mm256_maskstore_ps(velocities_y, load_mask, vy);
        _mm256_maskstore_ps(positions_x, load_mask, px);
        _mm256_maskstore_ps(positions_y, load_mask, py);
        _mm256_maskstore_ps(lifetimes, load_mask, t);
    }

    recalculate_particle_count(block);
}
#else
void
UDB_acceleration_y_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */
