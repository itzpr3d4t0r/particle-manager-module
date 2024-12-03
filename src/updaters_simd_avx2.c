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
update_with_acceleration_avx2(DataBlock *block, float dt)
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
}
#else
void
update_with_acceleration_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
update_with_no_acceleration_avx2(DataBlock *block, float dt)
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
}
#else
void
update_with_no_acceleration_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
update_with_acceleration_x_avx2(DataBlock *block, float dt)
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
}
#else
void
update_with_acceleration_x_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
update_with_acceleration_y_avx2(DataBlock *block, float dt)
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
}
#else
void
update_with_acceleration_y_avx2(DataBlock *block, float dt)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void
update_indices_avx2(DataBlock *block)
{
    float *restrict lifetimes = block->lifetimes.data;
    float *restrict max_lifetimes = block->max_lifetimes.data;
    __m256i *indices = (__m256i *)block->animation_indices;

    const int n_iters_8 = block->particles_count / 8;
    const int n_excess = block->particles_count % 8;
    const __m256 one_v = _mm256_set1_ps(1.0f);
    const __m256 num_frames_v = _mm256_set1_ps((float)(block->num_frames));
    const __m256i load_mask = _mm256_set_epi32(
        0, n_excess > 6 ? -1 : 0, n_excess > 5 ? -1 : 0, n_excess > 4 ? -1 : 0,
        n_excess > 3 ? -1 : 0, n_excess > 2 ? -1 : 0, n_excess > 1 ? -1 : 0,
        n_excess > 0 ? -1 : 0);

    for (int i = 0; i < n_iters_8; i++) {
        __m256 t = _mm256_loadu_ps(lifetimes);
        __m256 max_t = _mm256_loadu_ps(max_lifetimes);

        __m256i idx = _mm256_cvttps_epi32(_mm256_mul_ps(
            _mm256_sub_ps(one_v, _mm256_div_ps(t, max_t)), num_frames_v));

        _mm256_storeu_si256(indices, idx);

        lifetimes += 8;
        max_lifetimes += 8;
        indices++;
    }

    if (n_excess) {
        __m256 t = _mm256_maskload_ps(lifetimes, load_mask);
        __m256 max_t = _mm256_maskload_ps(max_lifetimes, load_mask);

        __m256i idx = _mm256_cvttps_epi32(_mm256_mul_ps(
            _mm256_sub_ps(one_v, _mm256_div_ps(t, max_t)), num_frames_v));

        _mm256_maskstore_epi32((int *)indices, load_mask, idx);
    }
}
#else
void
update_indices_avx2(DataBlock *block)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */

#if defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
    !defined(SDL_DISABLE_IMMINTRIN_H)
void inline blit_add_avx2_1x1(uint32_t *srcp32, uint32_t *dstp32)
{
    __m128i src128 = _mm_cvtsi32_si128(*srcp32);
    __m128i dst128 = _mm_cvtsi32_si128(*dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    *dstp32 = _mm_cvtsi128_si32(dst128);
}

void inline blit_add_avx2_2x2(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
                              int dst_skip)
{
    __m128i src128 = _mm_loadl_epi64((__m128i *)srcp32);
    __m128i dst128 = _mm_loadl_epi64((__m128i *)dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    _mm_storel_epi64((__m128i *)dstp32, dst128);

    srcp32 += 2 + src_skip;
    dstp32 += 2 + dst_skip;

    src128 = _mm_loadl_epi64((__m128i *)srcp32);
    dst128 = _mm_loadl_epi64((__m128i *)dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    _mm_storel_epi64((__m128i *)dstp32, dst128);
}

void inline blit_add_avx2_3x3(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
                              int dst_skip)
{
    __m128i src128 = _mm_loadl_epi64((__m128i *)srcp32);
    __m128i dst128 = _mm_loadl_epi64((__m128i *)dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    _mm_storel_epi64((__m128i *)dstp32, dst128);

    srcp32 += 2;
    dstp32 += 2;

    src128 = _mm_cvtsi32_si128(*srcp32);
    dst128 = _mm_cvtsi32_si128(*dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    *dstp32 = _mm_cvtsi128_si32(dst128);

    srcp32 += 1 + src_skip;
    dstp32 += 1 + dst_skip;

    src128 = _mm_loadl_epi64((__m128i *)srcp32);
    dst128 = _mm_loadl_epi64((__m128i *)dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    _mm_storel_epi64((__m128i *)dstp32, dst128);

    srcp32 += 2;
    dstp32 += 2;

    src128 = _mm_cvtsi32_si128(*srcp32);
    dst128 = _mm_cvtsi32_si128(*dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    *dstp32 = _mm_cvtsi128_si32(dst128);
}

void inline blit_add_avx2_4x4(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
                              int dst_skip)
{
    __m128i src128;
    __m128i dst128;
    UNROLL_3({
        src128 = _mm_loadu_si128((__m128i *)srcp32);
        dst128 = _mm_loadu_si128((__m128i *)dstp32);

        dst128 = _mm_adds_epu8(src128, dst128);

        _mm_storeu_si128((__m128i *)dstp32, dst128);

        srcp32 += 4 + src_skip;
        dstp32 += 4 + dst_skip;
    })

    src128 = _mm_loadu_si128((__m128i *)srcp32);
    dst128 = _mm_loadu_si128((__m128i *)dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    _mm_storeu_si128((__m128i *)dstp32, dst128);
}

void inline blit_add_avx2_5x5(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
                              int dst_skip)
{
    __m128i src128;
    __m128i dst128;
    UNROLL_4({
        src128 = _mm_loadu_si128((__m128i *)srcp32);
        dst128 = _mm_loadu_si128((__m128i *)dstp32);

        dst128 = _mm_adds_epu8(src128, dst128);

        _mm_storeu_si128((__m128i *)dstp32, dst128);

        srcp32 += 4;
        dstp32 += 4;

        src128 = _mm_cvtsi32_si128(*srcp32);
        dst128 = _mm_cvtsi32_si128(*dstp32);

        dst128 = _mm_adds_epu8(src128, dst128);

        *dstp32 = _mm_cvtsi128_si32(dst128);

        srcp32 += 1 + src_skip;
        dstp32 += 1 + dst_skip;
    })

    src128 = _mm_loadu_si128((__m128i *)srcp32);
    dst128 = _mm_loadu_si128((__m128i *)dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    _mm_storeu_si128((__m128i *)dstp32, dst128);

    srcp32 += 4;
    dstp32 += 4;

    src128 = _mm_cvtsi32_si128(*srcp32);
    dst128 = _mm_cvtsi32_si128(*dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    *dstp32 = _mm_cvtsi128_si32(dst128);
}

void
blit_fragments_add_avx2(FragmentationMap *frag_map, PyObject **animation,
                        int dst_skip)
{
    Fragment *fragments = frag_map->fragments;
    BlitDestination *destinations = frag_map->destinations;

    for (int i = 0; i < frag_map->used_f; i++) {
        Fragment *fragment = &fragments[i];
        SDL_Surface *src_surf =
            ((pgSurfaceObject *)animation[fragment->animation_index])->surf;
        const int src_skip = src_surf->pitch / 4 - src_surf->w;
        const int actual_dst_skip = dst_skip - src_surf->w;
        uint32_t *const src_start = (uint32_t *)src_surf->pixels;

        for (int j = 0; j < fragment->length; j++) {
            BlitDestination *item = &destinations[j];
            uint32_t *srcp32 = src_start;
            uint32_t *dstp32 = item->pixels;

            if (item->width == 1 && item->rows == 1) {
                blit_add_avx2_1x1(srcp32, dstp32);
                continue;
            }
            else if (item->width == 2 && item->rows == 2) {
                blit_add_avx2_2x2(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }
            else if (item->width == 3 && item->rows == 3) {
                blit_add_avx2_3x3(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }
            else if (item->width == 4 && item->rows == 4) {
                blit_add_avx2_4x4(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }
            else if (item->width == 5 && item->rows == 5) {
                blit_add_avx2_5x5(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }

            int h = item->rows;

            const int n_iters_8 = item->width / 8;
            const int pxl_excess = item->width % 8;

            while (h--) {
                for (int k = 0; k < n_iters_8; k++) {
                    __m256i src256 = _mm256_loadu_si256((__m256i *)srcp32);
                    __m256i dst256 = _mm256_loadu_si256((__m256i *)dstp32);

                    dst256 = _mm256_adds_epu8(src256, dst256);

                    _mm256_storeu_si256((__m256i *)dstp32, dst256);

                    srcp32 += 8;
                    dstp32 += 8;
                }

                for (int k = 0; k < pxl_excess; k++) {
                    __m128i src128 = _mm_cvtsi32_si128(*srcp32);
                    __m128i dst128 = _mm_cvtsi32_si128(*dstp32);

                    dst128 = _mm_adds_epu8(src128, dst128);

                    *dstp32 = _mm_cvtsi128_si32(dst128);

                    srcp32++;
                    dstp32++;
                }

                srcp32 += src_skip;
                dstp32 += actual_dst_skip;
            }
        }

        destinations += fragment->length;
    }
}
#else
void
blit_fragments_add_avx2(FragmentationMap *frag_map, PyObject **animation,
                        int dst_skip)
{
    BAD_AVX2_FUNCTION_CALL
}
#endif /* defined(__AVX2__) && defined(HAVE_IMMINTRIN_H) && \
          \ !defined(SDL_DISABLE_IMMINTRIN_H) */