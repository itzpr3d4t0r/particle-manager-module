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
update_with_acceleration_sse2(DataBlock *block, float dt)
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
update_with_acceleration_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
update_with_no_acceleration_sse2(DataBlock *block, float dt)
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
update_with_no_acceleration_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
update_with_acceleration_x_sse2(DataBlock *block, float dt)
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
update_with_acceleration_x_sse2(DataBlock *block, float dt)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void
update_with_acceleration_y_sse2(DataBlock *block, float dt)
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
update_with_acceleration_y_sse2(DataBlock *block, float dt)
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
    __m128i *indices128p = (__m128i *)block->animation_indices;

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

        _mm_storeu_si128(indices128p, idx);

        lifetimes += 4;
        max_lifetimes += 4;
        indices128p++;
    }

    int *indices = (int *)indices128p;

    for (i = 0; i < n_excess; i++) {
        __m128 t = _mm_load_ss(lifetimes);
        __m128 max_t = _mm_load_ss(max_lifetimes);

        __m128i idx = _mm_cvttps_epi32(
            _mm_mul_ss(_mm_sub_ss(one_v, _mm_div_ss(t, max_t)), num_frames_v));

        indices[i] = _mm_cvtsi128_si32(idx);
    }
}
#else
void
update_indices_sse2(DataBlock *block)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */

#if defined(__SSE2__) || defined(ENABLE_ARM_NEON)
void inline blit_add_sse2_1x1(uint32_t *srcp32, uint32_t *dstp32)
{
    __m128i src128 = _mm_cvtsi32_si128(*srcp32);
    __m128i dst128 = _mm_cvtsi32_si128(*dstp32);

    dst128 = _mm_adds_epu8(src128, dst128);

    *dstp32 = _mm_cvtsi128_si32(dst128);
}

void inline blit_add_sse2_2x2(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
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

void inline blit_add_sse2_3x3(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
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

void inline blit_add_sse2_4x4(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
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

void inline blit_add_sse2_5x5(uint32_t *srcp32, uint32_t *dstp32, int src_skip,
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
blit_fragments_add_sse2(FragmentationMap *frag_map, PyObject **animation,
                        int dst_skip)
{
    Fragment *fragments = frag_map->fragments;
    BlitDestination *destinations = frag_map->destinations;

    for (int i = 0; i < frag_map->used_f; i++) {
        Fragment *fragment = &fragments[i];
        SDL_Surface *src_surf =
            ((pgSurfaceObject *)animation[fragment->animation_index])->surf;
        uint32_t *const src_start = (uint32_t *)src_surf->pixels;

        for (int j = 0; j < fragment->length; j++) {
            BlitDestination *item = &destinations[j];

            uint32_t *srcp32 = src_start + item->src_offset;
            uint32_t *dstp32 = item->pixels;
            const int src_skip = src_surf->pitch / 4 - item->width;
            const int actual_dst_skip = dst_skip - item->width;

            if (item->width == 1 && item->rows == 1) {
                blit_add_sse2_1x1(srcp32, dstp32);
                continue;
            }
            else if (item->width == 2 && item->rows == 2) {
                blit_add_sse2_2x2(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }
            else if (item->width == 3 && item->rows == 3) {
                blit_add_sse2_3x3(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }
            else if (item->width == 4 && item->rows == 4) {
                blit_add_sse2_4x4(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }
            else if (item->width == 5 && item->rows == 5) {
                blit_add_sse2_5x5(srcp32, dstp32, src_skip, actual_dst_skip);
                continue;
            }

            int h = item->rows;
            const int n_iters_4 = item->width / 4;
            const int pxl_excess = item->width % 4;
            int k;

            while (h--) {
                for (k = 0; k < n_iters_4; k++) {
                    __m128i src128 = _mm_loadu_si128((__m128i *)srcp32);
                    __m128i dst128 = _mm_loadu_si128((__m128i *)dstp32);

                    dst128 = _mm_adds_epu8(src128, dst128);

                    _mm_storeu_si128((__m128i *)dstp32, dst128);

                    srcp32 += 4;
                    dstp32 += 4;
                }

                for (k = 0; k < pxl_excess; k++) {
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
blit_fragments_add_sse2(FragmentationMap *frag_map, PyObject **animation,
                        int dst_skip)
{
    BAD_SSE2_FUNCTION_CALL
}
#endif /* __SSE2__ || ENABLE_ARM_NEON */
