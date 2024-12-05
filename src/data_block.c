#include <search.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "include/data_block.h"
#include "include/simd_common.h"
#include "include/option.h"

/* ====================| Public facing DataBlock functions |==================== */
int
init_data_block(DataBlock *block, Emitter *emitter, vec2 position)
{
    block->particles_count = emitter->emission_number;
    Py_INCREF(emitter->animation);
    block->animation = emitter->animation;
    block->num_frames = emitter->num_frames;
    block->blend_mode = emitter->blend_mode;
    block->ended = false;

    /* Conditionally allocate memory for the arrays based on emitter properties */
    if (!alloc_and_init_positions(block, emitter, position) ||
        !alloc_and_init_accelerations(block, emitter) ||
        !alloc_and_init_velocities(block, emitter) ||
        !alloc_and_init_lifetimes(block, emitter) ||
        !alloc_and_init_animation_indices(block, emitter) ||
        !init_fragmentation_map(block))
        return 0;

    choose_and_set_update_function(block, emitter);

    return 1;
}

void
dealloc_data_block(DataBlock *block)
{
    Py_DECREF(block->animation);
    float_array_free(&block->positions_x);
    float_array_free(&block->positions_y);
    float_array_free(&block->velocities_x);
    float_array_free(&block->velocities_y);
    float_array_free(&block->accelerations_x);
    float_array_free(&block->accelerations_y);
    float_array_free(&block->lifetimes);
    float_array_free(&block->max_lifetimes);
    PyMem_Free(block->animation_indices);
    dealloc_fragmentation_map(&block->frag_map);
}

void
update_data_block(DataBlock *block, float dt)
{
    block->updater(block, dt);

    recalculate_particle_count(block);
}

int
draw_data_block(DataBlock *block, pgSurfaceObject *dest, const int blend_flag)
{
    update_indices(block);

    if (!calculate_fragmentation_map(dest, block))
        return 0;

    blit_fragments(dest, &block->frag_map, block, blend_flag);

    return 1;
}

/* ====================| Internal DataBlock functions |==================== */

void
choose_and_set_update_function(DataBlock *block, Emitter *emitter)
{
    const int use_x_acc = OPTION_IS_SET(emitter->acceleration_x);
    const int use_y_acc = OPTION_IS_SET(emitter->acceleration_y);

#if !defined(__EMSCRIPTEN__)
    if (_Has_AVX2()) {
        if (!use_x_acc && !use_y_acc)
            block->updater = update_with_no_acceleration_avx2;
        else if (use_x_acc && !use_y_acc)
            block->updater = update_with_acceleration_x_avx2;
        else if (!use_x_acc && use_y_acc)
            block->updater = update_with_acceleration_y_avx2;
        else
            block->updater = update_with_acceleration_avx2;

        return;
    }

#if ENABLE_SSE_NEON
    if (_HasSSE_NEON()) {
        if (!use_x_acc && !use_y_acc)
            block->updater = update_with_no_acceleration_sse2;
        else if (use_x_acc && !use_y_acc)
            block->updater = update_with_acceleration_x_sse2;
        else if (!use_x_acc && use_y_acc)
            block->updater = update_with_acceleration_y_sse2;
        else
            block->updater = update_with_acceleration_sse2;

        return;
    }
#endif /* ENABLE_SSE_NEON */
#endif /* __EMSCRIPTEN__ */

    if (!use_x_acc && !use_y_acc)
        block->updater = update_with_no_acceleration;
    else if (use_x_acc && !use_y_acc)
        block->updater = update_with_acceleration_x;
    else if (!use_x_acc && use_y_acc)
        block->updater = update_with_acceleration_y;
    else
        block->updater = update_with_acceleration;
}

void
calculate_surface_index_occurrences(DataBlock *block)
{
    block->frag_map.used_f = 0;

    int const *indices = block->animation_indices;
    int particle_count = block->particles_count;

    int current_value = indices[0];
    int current_count = 1;

    FragmentationMap *frag_map = &block->frag_map;
    Fragment *fragments = frag_map->fragments;

    for (int i = 1; i < particle_count; ++i) {
        if (indices[i] == current_value)
            current_count++;
        else {
            Fragment *fragment = &fragments[frag_map->used_f++];
            fragment->animation_index =
                clamp_int(current_value, 0, block->num_frames - 1);
            fragment->length = current_count;

            current_value = indices[i];
            current_count = 1;
        }
    }

    if (current_count > 0) {
        Fragment *fragment = &fragments[frag_map->used_f++];
        fragment->animation_index =
            clamp_int(current_value, 0, block->num_frames - 1);
        fragment->length = current_count;
    }
}

int
populate_destinations_array(pgSurfaceObject *dest, DataBlock *block)
{
    FragmentationMap *frag_map = &block->frag_map;
    Fragment *fragments = frag_map->fragments;

    BlitDestination *destinations = frag_map->destinations;
    float *positions_x = block->positions_x.data;
    float *positions_y = block->positions_y.data;
    PyObject **animation = PySequence_Fast_ITEMS(block->animation);
    SDL_Surface *dest_surf = dest->surf;

    const int dest_skip = dest_surf->pitch / 4;
    uint32_t *dest_pixels = (uint32_t *)dest_surf->pixels;

    const SDL_Rect dest_clip = dest_surf->clip_rect;

    const int dst_clip_x = dest_clip.x;
    const int dst_clip_y = dest_clip.y;
    const int dst_clip_right = dest_clip.x + dest_clip.w;
    const int dst_clip_bottom = dest_clip.y + dest_clip.h;

    frag_map->dest_count = 0;

    for (int i = 0; i < frag_map->used_f; i++) {
        Fragment *frg = &fragments[i];
        int length = frg->length;
        const pgSurfaceObject *src_obj =
            (pgSurfaceObject *)animation[frg->animation_index];
        if (!src_obj) {
            PyErr_SetString(PyExc_RuntimeError, "Surface is not initialized");
            return 0;
        }

        SDL_Surface const *src_surf = src_obj->surf;
        const int src_pitch = src_surf->pitch / 4;
        const int width = src_surf->w;
        const int height = src_surf->h;

        for (int j = 0; j < length; j++) {
            const int A_x = (int)positions_x[j];
            const int A_y = (int)positions_y[j];
            const int A_x_right = A_x + width;
            const int A_y_bottom = A_y + height;

            SDL_Rect clipped;
            if (!IntersectRect(A_x, A_x_right, dst_clip_x, dst_clip_right, A_y,
                               A_y_bottom, dst_clip_y, dst_clip_bottom, &clipped)) {
                frg->length--;
                continue;
            }

            BlitDestination *destination = &destinations[frag_map->dest_count++];

            destination->pixels = dest_pixels + clipped.y * dest_skip + clipped.x;
            destination->width = clipped.w;
            destination->rows = clipped.h;
            destination->src_offset =
                (A_x < dst_clip_x ? dst_clip_x - A_x : 0) +
                (A_y < dst_clip_y ? dst_clip_y - A_y : 0) * src_pitch;
        }

        positions_x += length;
        positions_y += length;
    }

    return 1;
}

int
calculate_fragmentation_map(pgSurfaceObject *dest, DataBlock *block)
{
    calculate_surface_index_occurrences(block);

    if (!populate_destinations_array(dest, block))
        return 0;

    return 1;
}

void
blit_fragments(pgSurfaceObject *dest, FragmentationMap *frag_map, DataBlock *block,
               int blend_flags)
{
    switch (blend_flags) {
        case 0: /* blitcopy */
            blit_fragments_blitcopy(frag_map, dest, block);
            return;
        case 1: /* add */
            blit_fragments_add(frag_map, dest, block);
            return;
        default:
            return;
    }
}

void
blit_fragments_blitcopy(FragmentationMap *frag_map, pgSurfaceObject *dest,
                        DataBlock *block)
{
    PyObject **animation = PySequence_Fast_ITEMS(block->animation);
    Fragment *fragments = frag_map->fragments;
    BlitDestination *destinations = frag_map->destinations;
    const int dst_skip = dest->surf->pitch / 4;

    for (int i = 0; i < frag_map->used_f; i++) {
        Fragment *fragment = &fragments[i];
        SDL_Surface *src_surf =
            ((pgSurfaceObject *)animation[fragment->animation_index])->surf;
        const int src_skip = src_surf->pitch / 4;
        uint32_t *const src_start = (uint32_t *)src_surf->pixels;

        for (int j = 0; j < fragment->length; j++) {
            BlitDestination *item = &destinations[j];

            uint32_t *srcp32 = src_start + item->src_offset;
            uint32_t *dstp32 = item->pixels;

            if (item->width == 1 && item->rows == 1) {
                *dstp32 = *srcp32;
                continue;
            }
            else if (item->width == 2 && item->rows == 2) {
                dstp32[0] = srcp32[0];
                dstp32[1] = srcp32[1];

                srcp32 += src_skip;
                dstp32 += dst_skip;

                dstp32[0] = srcp32[0];
                dstp32[1] = srcp32[1];

                continue;
            }
            else if (item->width == 3 && item->rows == 3) {
                UNROLL_2({
                    dstp32[0] = srcp32[0];
                    dstp32[1] = srcp32[1];
                    dstp32[2] = srcp32[2];

                    srcp32 += src_skip;
                    dstp32 += dst_skip;
                })

                dstp32[0] = srcp32[0];
                dstp32[1] = srcp32[1];
                dstp32[2] = srcp32[2];

                continue;
            }
            else if (item->width == 4 && item->rows == 4) {
                UNROLL_3({
                    dstp32[0] = srcp32[0];
                    dstp32[1] = srcp32[1];
                    dstp32[2] = srcp32[2];
                    dstp32[3] = srcp32[3];

                    srcp32 += src_skip;
                    dstp32 += dst_skip;
                })

                dstp32[0] = srcp32[0];
                dstp32[1] = srcp32[1];
                dstp32[2] = srcp32[2];
                dstp32[3] = srcp32[3];

                continue;
            }
            else if (item->width == 5 && item->rows == 5) {
                UNROLL_4({
                    dstp32[0] = srcp32[0];
                    dstp32[1] = srcp32[1];
                    dstp32[2] = srcp32[2];
                    dstp32[3] = srcp32[3];
                    dstp32[4] = srcp32[4];

                    srcp32 += src_skip;
                    dstp32 += dst_skip;
                })

                dstp32[0] = srcp32[0];
                dstp32[1] = srcp32[1];
                dstp32[2] = srcp32[2];
                dstp32[3] = srcp32[3];
                dstp32[4] = srcp32[4];

                continue;
            }

            int h = item->rows;
            const int copy_w = item->width * 4;

            while (h--) {
                memcpy(dstp32, srcp32, copy_w);
                srcp32 += src_skip;
                dstp32 += dst_skip;
            }
        }

        destinations += fragment->length;
    }
}

void
blit_fragments_add(FragmentationMap *frag_map, pgSurfaceObject *dest,
                   DataBlock *block)
{
    PyObject **animation = PySequence_Fast_ITEMS(block->animation);
    const int dst_skip = dest->surf->pitch / 4;

#if !defined(__EMSCRIPTEN__)
    if (_Has_AVX2()) {
        blit_fragments_add_avx2(frag_map, animation, dst_skip);
        return;
    }
#if ENABLE_SSE_NEON
    if (_HasSSE_NEON()) {
        blit_fragments_add_sse2(frag_map, animation, dst_skip);
        return;
    }
#endif /* ENABLE_SSE_NEON */
#endif /* __EMSCRIPTEN__ */

    blit_fragments_add_scalar(frag_map, animation, dst_skip);
}

void
blit_fragments_add_scalar(FragmentationMap *frag_map, PyObject **animation,
                          int dst_skip)
{
    Fragment *fragments = frag_map->fragments;
    BlitDestination *destinations = frag_map->destinations;
    SDL_PixelFormat *fmt = ((pgSurfaceObject *)animation[0])->surf->format;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    const int Ridx = fmt->Rshift >> 3;
    const int Gidx = fmt->Gshift >> 3;
    const int Bidx = fmt->Bshift >> 3;
#else
    const int Ridx = 3 - (fmt->Rshift >> 3);
    const int Gidx = 3 - (fmt->Gshift >> 3);
    const int Bidx = 3 - (fmt->Bshift >> 3);
#endif

    for (int i = 0; i < frag_map->used_f; i++) {
        Fragment *fragment = &fragments[i];
        SDL_Surface *src_surf =
            ((pgSurfaceObject *)animation[fragment->animation_index])->surf;
        uint8_t *const src_start = (uint8_t *)src_surf->pixels;

        for (int j = 0; j < fragment->length; j++) {
            BlitDestination *item = &destinations[j];

            uint8_t *srcp8 = src_start + item->src_offset * 4;
            uint8_t *dstp8 = (uint8_t *)item->pixels;
            const int actual_dst_skip = 4 * (dst_skip - item->width);
            const int src_skip = src_surf->pitch - item->width * 4;

            int h = item->rows;

            while (h--) {
                for (int k = 0; k < item->width; k++) {
                    uint8_t sr = srcp8[Ridx];
                    uint8_t sg = srcp8[Gidx];
                    uint8_t sb = srcp8[Bidx];

                    uint8_t dr = dstp8[Ridx];
                    uint8_t dg = dstp8[Gidx];
                    uint8_t db = dstp8[Bidx];

                    dstp8[Ridx] = sr + dr > 255 ? 255 : sr + dr;
                    dstp8[Gidx] = sg + dg > 255 ? 255 : sg + dg;
                    dstp8[Bidx] = sb + db > 255 ? 255 : sb + db;

                    srcp8 += 4;
                    dstp8 += 4;
                }

                srcp8 += src_skip;
                dstp8 += actual_dst_skip;
            }
        }

        destinations += fragment->length;
    }
}

int
_compare_desc(const void *a, const void *b)
{
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa < fb) - (fa > fb);
}

int
find_first_leq_zero(const float *restrict arr, int size)
{
    int low = 0;
    int high = size - 1;
    int result = -1;

    while (low <= high) {
        int mid = (low + high) / 2;
        if (arr[mid] > 0) {
            low = mid + 1;
        }
        else {
            result = mid;
            high = mid - 1;
        }
    }

    return result;
}

void
recalculate_particle_count(DataBlock *block)
{
    /* Find the first particle with a lifetime less than or equal to zero */
    int index = find_first_leq_zero(block->lifetimes.data, block->particles_count);

    /* If all particles are alive the index will be -1, so just return */
    if (index == -1)
        return;

    block->particles_count = index;

    if (!index)
        block->ended = true;
}

int
init_fragmentation_map(DataBlock *block)
{
    FragmentationMap *frag_map = &block->frag_map;

    frag_map->fragments = PyMem_New(Fragment, block->num_frames);
    if (!frag_map->fragments)
        return 0;

    frag_map->destinations = PyMem_New(BlitDestination, block->particles_count);
    if (!frag_map->destinations)
        return 0;

    frag_map->used_f = 0;
    frag_map->alloc_f = block->num_frames;
    frag_map->dest_count = 0;

    return 1;
}

void
dealloc_fragmentation_map(FragmentationMap *frag_map)
{
    PyMem_Free(frag_map->fragments);
    PyMem_Free(frag_map->destinations);
}

int
alloc_and_init_positions(DataBlock *block, Emitter *emitter, vec2 position)
{
    float_array *x = &block->positions_x;
    float_array *y = &block->positions_y;

    /* Allocate memory for the positions array */
    if (!float_array_alloc(x, emitter->emission_number) ||
        !float_array_alloc(y, emitter->emission_number))
        return 0;

    /* Initialize the positions array */
    float *restrict positions_x = x->data;
    float *restrict positions_y = y->data;

    switch (emitter->spawn_shape) {
        case _POINT:
            for (int i = 0; i < emitter->emission_number; i++) {
                positions_x[i] = position.x;
                positions_y[i] = position.y;
            }
            break;
        default:
            return 0;
    }

    return 1;
}

int
alloc_and_init_velocities(DataBlock *block, Emitter *emitter)
{
    const int x_set = OPTION_IS_SET(emitter->speed_x);
    const int y_set = OPTION_IS_SET(emitter->speed_y);

    /* Acceleration flags are already set when this is called */
    int flags = block->update_flags;

    if (!x_set && !y_set) {
        /* In this case, based on the acceleration flags, we either allocate an array
         * of zeroes or set a single value to zero */
        if (flags & NO_ACCELERATION) {
            block->update_flags |= NO_VELOCITY;
        }
        else if (flags & ACCEL_X_SINGLE && flags & ACCEL_Y_SINGLE) {
            block->update_flags |= VELOCITY_X_SINGLE | VELOCITY_Y_SINGLE;
            block->s_velocity_x = 0.0f;
            block->s_velocity_y = 0.0f;
        }
        else if (flags & ACCEL_X_ARRAY && flags & ACCEL_Y_ARRAY) {
            block->update_flags |= VELOCITY_X_ARRAY | VELOCITY_Y_ARRAY;

            float_array *x = &block->velocities_x;
            float_array *y = &block->velocities_y;

            /* Allocate memory for the velocities array ( 0 initialized ) */
            if (!float_array_calloc(x, emitter->emission_number) ||
                !float_array_calloc(y, emitter->emission_number))
                return 0;
        }
        else if (flags & ACCEL_X_SINGLE && flags & ACCEL_Y_ARRAY) {
            block->update_flags |= VELOCITY_X_SINGLE | VELOCITY_Y_ARRAY;
            block->s_velocity_x = 0.0f;

            float_array *y = &block->velocities_y;

            /* Allocate memory for the velocities array ( 0 initialized ) */
            if (!float_array_calloc(y, emitter->emission_number))
                return 0;
        }
        else if (flags & ACCEL_X_ARRAY && flags & ACCEL_Y_SINGLE) {
            block->update_flags |= VELOCITY_X_ARRAY | VELOCITY_Y_SINGLE;
            block->s_velocity_y = 0.0f;

            float_array *x = &block->velocities_x;

            /* Allocate memory for the velocities array ( 0 initialized ) */
            if (!float_array_calloc(x, emitter->emission_number))
                return 0;
        }

        return 1;
    }
    else if (x_set && y_set) {
        const int x_is_value = OPTION_IS_VALUE(emitter->speed_x);
        const int y_is_value = OPTION_IS_VALUE(emitter->speed_y);

        if (x_is_value && y_is_value) {
            /* Both velocities are set to a single constant value, the update
             * function will just store a single value each */
            if (flags & NO_ACCELERATION ||
                (flags & ACCEL_X_SINGLE && flags & ACCEL_Y_SINGLE)) {
                block->update_flags |= VELOCITY_X_SINGLE | VELOCITY_Y_SINGLE;

                block->s_velocity_x = option_get_value(&emitter->speed_x);
                block->s_velocity_y = option_get_value(&emitter->speed_y);
            }
            else if (flags & ACCEL_X_ARRAY && flags & ACCEL_Y_ARRAY) {
                block->update_flags |= VELOCITY_X_ARRAY | VELOCITY_Y_ARRAY;

                float_array *x = &block->velocities_x;
                float_array *y = &block->velocities_y;

                /* Allocate memory for the velocities array */
                if (!float_array_alloc(x, emitter->emission_number) ||
                    !float_array_alloc(y, emitter->emission_number))
                    return 0;

                /* Initialize the velocities array */
                for (int i = 0; i < emitter->emission_number; i++) {
                    x->data[i] = option_get_value(&emitter->speed_x);
                    y->data[i] = option_get_value(&emitter->speed_y);
                }
            }
            else if (flags & ACCEL_X_SINGLE && flags & ACCEL_Y_ARRAY) {
                block->update_flags |= VELOCITY_X_SINGLE | VELOCITY_Y_ARRAY;

                block->s_velocity_x = option_get_value(&emitter->speed_x);

                float_array *y = &block->velocities_y;
                if (!float_array_alloc(y, emitter->emission_number))
                    return 0;

                for (int i = 0; i < emitter->emission_number; i++)
                    y->data[i] = option_get_value(&emitter->speed_y);
            }
            else if (flags & ACCEL_X_ARRAY && flags & ACCEL_Y_SINGLE) {
                block->update_flags |= VELOCITY_X_ARRAY | VELOCITY_Y_SINGLE;

                block->s_velocity_y = option_get_value(&emitter->speed_y);

                float_array *x = &block->velocities_x;
                if (!float_array_alloc(x, emitter->emission_number))
                    return 0;

                for (int i = 0; i < emitter->emission_number; i++)
                    x->data[i] = option_get_value(&emitter->speed_x);
            }

            return 1;
        }
        else if (!x_is_value && !y_is_value) {
            /* Both velocities are randomized, the update function will store
             * a random value for each particle */
            block->update_flags |= VELOCITY_X_ARRAY | VELOCITY_Y_ARRAY;

            float_array *x = &block->velocities_x;
            float_array *y = &block->velocities_y;

            /* Allocate memory for the velocities array */
            if (!float_array_alloc(x, emitter->emission_number) ||
                !float_array_alloc(y, emitter->emission_number))
                return 0;

            /* Initialize the velocities array */
            for (int i = 0; i < emitter->emission_number; i++) {
                x->data[i] = option_get_value(&emitter->speed_x);
                y->data[i] = option_get_value(&emitter->speed_y);
            }
        }
        else if (x_is_value && !y_is_value) {
            /* Velocity x is set to a single constant value, velocity y
             * is randomized */
            if (flags & ACCEL_X_SINGLE || flags & NO_ACCELERATION_X) {
                block->update_flags |= VELOCITY_X_SINGLE;
                block->s_velocity_x = option_get_value(&emitter->speed_x);
            }
            else if (flags & ACCEL_X_ARRAY) {
                block->update_flags |= VELOCITY_X_ARRAY;

                float_array *x = &block->velocities_x;
                if (!float_array_alloc(x, emitter->emission_number))
                    return 0;

                for (int i = 0; i < emitter->emission_number; i++)
                    x->data[i] = option_get_value(&emitter->speed_x);
            }

            block->update_flags |= VELOCITY_Y_ARRAY;

            float_array *y = &block->velocities_y;
            if (!float_array_alloc(y, emitter->emission_number))
                return 0;

            for (int i = 0; i < emitter->emission_number; i++)
                y->data[i] = option_get_value(&emitter->speed_y);
        }
        else if (!x_is_value && y_is_value) {
            /* Velocity y is set to a single constant value, velocity x
             * is randomized */
            if (flags & ACCEL_Y_SINGLE || flags & NO_ACCELERATION_Y) {
                block->update_flags |= VELOCITY_Y_SINGLE;
                block->s_velocity_y = option_get_value(&emitter->speed_y);
            }
            else if (flags & ACCEL_Y_ARRAY) {
                block->update_flags |= VELOCITY_Y_ARRAY;

                float_array *y = &block->velocities_y;
                if (!float_array_alloc(y, emitter->emission_number))
                    return 0;

                for (int i = 0; i < emitter->emission_number; i++)
                    y->data[i] = option_get_value(&emitter->speed_y);
            }

            block->update_flags |= VELOCITY_X_ARRAY;

            float_array *x = &block->velocities_x;
            if (!float_array_alloc(x, emitter->emission_number))
                return 0;

            for (int i = 0; i < emitter->emission_number; i++)
                x->data[i] = option_get_value(&emitter->speed_x);
        }
    }
    else if (!x_set && y_set) {
        if (flags & ACCEL_X_SINGLE) {
            block->update_flags |= VELOCITY_X_SINGLE;
            block->s_velocity_x = 0.0f;
        }
        else if (flags & ACCEL_X_ARRAY) {
            block->update_flags |= VELOCITY_X_ARRAY;

            float_array *x = &block->velocities_x;
            if (!float_array_calloc(x, emitter->emission_number))
                return 0;
        }

        if (OPTION_IS_VALUE(emitter->speed_y)) {
            if (flags & ACCEL_Y_SINGLE) {
                block->update_flags |= VELOCITY_Y_SINGLE;
                block->s_velocity_y = option_get_value(&emitter->speed_y);
            }
            else if (flags & ACCEL_Y_ARRAY) {
                block->update_flags |= VELOCITY_Y_ARRAY;

                float_array *y = &block->velocities_y;
                if (!float_array_alloc(y, emitter->emission_number))
                    return 0;

                for (int i = 0; i < emitter->emission_number; i++)
                    y->data[i] = option_get_value(&emitter->speed_y);
            }
        }
        else {
            block->update_flags |= VELOCITY_Y_ARRAY;

            float_array *y = &block->velocities_y;
            if (!float_array_alloc(y, emitter->emission_number))
                return 0;

            for (int i = 0; i < emitter->emission_number; i++)
                y->data[i] = option_get_value(&emitter->speed_y);
        }
    }
    else if (x_set && !y_set) {
        if (flags & ACCEL_Y_SINGLE) {
            block->update_flags |= VELOCITY_Y_SINGLE;
            block->s_velocity_y = 0.0f;
        }
        else if (flags & ACCEL_Y_ARRAY) {
            block->update_flags |= VELOCITY_Y_ARRAY;

            float_array *y = &block->velocities_y;
            if (!float_array_calloc(y, emitter->emission_number))
                return 0;
        }

        if (OPTION_IS_VALUE(emitter->speed_x)) {
            if (flags & ACCEL_X_SINGLE) {
                block->update_flags |= VELOCITY_X_SINGLE;
                block->s_velocity_x = option_get_value(&emitter->speed_x);
            }
            else if (flags & ACCEL_X_ARRAY) {
                block->update_flags |= VELOCITY_X_ARRAY;

                float_array *x = &block->velocities_x;
                if (!float_array_alloc(x, emitter->emission_number))
                    return 0;

                for (int i = 0; i < emitter->emission_number; i++)
                    x->data[i] = option_get_value(&emitter->speed_x);
            }
        }
        else {
            block->update_flags |= VELOCITY_X_ARRAY;

            float_array *x = &block->velocities_x;
            if (!float_array_alloc(x, emitter->emission_number))
                return 0;

            for (int i = 0; i < emitter->emission_number; i++)
                x->data[i] = option_get_value(&emitter->speed_x);
        }
    }

    return 1;
}

int
alloc_and_init_accelerations(DataBlock *block, Emitter *emitter)
{
    const int x_set = OPTION_IS_SET(emitter->acceleration_x);
    const int y_set = OPTION_IS_SET(emitter->acceleration_y);

    if (!x_set && !y_set) {
        /* If both accelerations are not set, hust return.
         * Both s_acceleration_x and s_acceleration_y are already 0 */
        block->update_flags |=
            NO_ACCELERATION | NO_ACCELERATION_X | NO_ACCELERATION_Y;
        return 1;
    }
    else if (x_set && y_set) {
        const int x_is_value = OPTION_IS_VALUE(emitter->acceleration_x);
        const int y_is_value = OPTION_IS_VALUE(emitter->acceleration_y);

        if (x_is_value && y_is_value) {
            /* Both accelerations are set to a single constant value, the update
             * function will just store a single value each */
            block->update_flags |= ACCEL_X_SINGLE | ACCEL_Y_SINGLE;

            block->s_acceleration_x = option_get_value(&emitter->acceleration_x);
            block->s_acceleration_y = option_get_value(&emitter->acceleration_y);

            return 1;
        }
        else if (!x_is_value && !y_is_value) {
            /* Both accelerations are randomized, the update function will store
             * a random value for each particle */
            block->update_flags |= ACCEL_X_ARRAY | ACCEL_Y_ARRAY;

            float_array *x = &block->accelerations_x;
            float_array *y = &block->accelerations_y;

            /* Allocate memory for the accelerations arrays */
            if (!float_array_alloc(x, emitter->emission_number))
                return 0;

            if (!float_array_alloc(y, emitter->emission_number))
                return 0;

            /* Initialize the accelerations arrays */
            for (int i = 0; i < emitter->emission_number; i++) {
                x->data[i] = option_get_value(&emitter->acceleration_x);
                y->data[i] = option_get_value(&emitter->acceleration_y);
            }
        }
        else if (x_is_value && !y_is_value) {
            /* Acceleration x is set to a single constant value, acceleration y
             * is randomized */
            block->update_flags |= ACCEL_X_SINGLE | ACCEL_Y_ARRAY;

            block->s_acceleration_x = option_get_value(&emitter->acceleration_x);

            float_array *y = &block->accelerations_y;
            if (!float_array_alloc(y, emitter->emission_number))
                return 0;

            for (int i = 0; i < emitter->emission_number; i++)
                y->data[i] = option_get_value(&emitter->acceleration_y);
        }
        else if (!x_is_value && y_is_value) {
            /* Acceleration y is set to a single constant value, acceleration x
             * is randomized */
            block->update_flags |= ACCEL_X_ARRAY | ACCEL_Y_SINGLE;

            block->s_acceleration_y = option_get_value(&emitter->acceleration_y);

            float_array *x = &block->accelerations_x;
            if (!float_array_alloc(x, emitter->emission_number))
                return 0;

            for (int i = 0; i < emitter->emission_number; i++)
                x->data[i] = option_get_value(&emitter->acceleration_x);
        }
    }
    else if (!x_set && y_set) {
        block->update_flags |= NO_ACCELERATION_X;
        if (OPTION_IS_VALUE(emitter->acceleration_y)) {
            block->update_flags |= ACCEL_Y_SINGLE;
            block->s_acceleration_y = option_get_value(&emitter->acceleration_y);
            return 1;
        }
        block->update_flags |= ACCEL_Y_ARRAY;

        float_array *y = &block->accelerations_y;
        if (!float_array_alloc(y, emitter->emission_number))
            return 0;

        for (int i = 0; i < emitter->emission_number; i++)
            y->data[i] = option_get_value(&emitter->acceleration_y);
    }
    else if (x_set && !y_set) {
        block->update_flags |= NO_ACCELERATION_Y;
        if (OPTION_IS_VALUE(emitter->acceleration_x)) {
            block->update_flags |= ACCEL_X_SINGLE;
            block->s_acceleration_x = option_get_value(&emitter->acceleration_x);
            return 1;
        }
        block->update_flags |= ACCEL_X_ARRAY;

        float_array *x = &block->accelerations_x;
        if (!float_array_alloc(x, emitter->emission_number))
            return 0;

        for (int i = 0; i < emitter->emission_number; i++)
            x->data[i] = option_get_value(&emitter->acceleration_x);
    }

    return 1;
}

int
alloc_and_init_lifetimes(DataBlock *block, Emitter *emitter)
{
    /* Lifetimes must be allocated since no particle can last forever */
    float_array *lifetimes = &block->lifetimes;
    float_array *max_lifetimes = &block->max_lifetimes;
    const int num_particles = emitter->emission_number;

    if (!float_array_alloc(lifetimes, num_particles) ||
        !float_array_alloc(max_lifetimes, num_particles))
        return 0;

    for (int i = 0; i < num_particles; i++)
        lifetimes->data[i] = option_get_value(&emitter->lifetime);

    /* Sort lifetimes in descending order */
    qsort(lifetimes->data, num_particles, sizeof(float), _compare_desc);

    /* Copy the sorted lifetimes to the max_lifetimes array */
    memcpy(max_lifetimes->data, lifetimes->data, sizeof(float) * num_particles);

    return 1;
}

int
alloc_and_init_animation_indices(DataBlock *block, Emitter *emitter)
{
    block->animation_indices = PyMem_New(int, emitter->emission_number);

    return block->animation_indices != NULL;
}

void
update_with_acceleration(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict lifetimes = block->lifetimes.data;
    float const *restrict accelerations_x = block->accelerations_x.data;
    float const *restrict accelerations_y = block->accelerations_y.data;
    float const *restrict max_lifetimes = block->max_lifetimes.data;
    int *restrict indices = block->animation_indices;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
        indices[i] =
            (int)((1.0f - lifetimes[i] / max_lifetimes[i]) * block->num_frames);
    }
}

void
update_with_no_acceleration(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict lifetimes = block->lifetimes.data;
    float const *restrict velocities_x = block->velocities_x.data;
    float const *restrict velocities_y = block->velocities_y.data;
    float const *restrict max_lifetimes = block->max_lifetimes.data;
    int *restrict indices = block->animation_indices;

    for (int i = 0; i < block->particles_count; i++) {
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
        indices[i] =
            (int)((1.0f - lifetimes[i] / max_lifetimes[i]) * block->num_frames);
    }
}

void
update_with_acceleration_x(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float const *restrict velocities_y = block->velocities_y.data;
    float const *restrict accelerations_x = block->accelerations_x.data;
    float const *restrict max_lifetimes = block->max_lifetimes.data;
    float *restrict lifetimes = block->lifetimes.data;
    int *restrict indices = block->animation_indices;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
        indices[i] =
            (int)((1.0f - lifetimes[i] / max_lifetimes[i]) * block->num_frames);
    }
}

void
update_with_acceleration_y(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_y = block->velocities_y.data;
    float *restrict lifetimes = block->lifetimes.data;
    float const *restrict max_lifetimes = block->max_lifetimes.data;
    float const *restrict velocities_x = block->velocities_x.data;
    float const *restrict accelerations_y = block->accelerations_y.data;
    int *restrict indices = block->animation_indices;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
        indices[i] =
            (int)((1.0f - lifetimes[i] / max_lifetimes[i]) * block->num_frames);
    }
}

void
update_indices_scalar(DataBlock *block)
{
    float const *restrict lifetimes = block->lifetimes.data;
    float const *restrict max_lifetimes = block->max_lifetimes.data;
    int *restrict indices = block->animation_indices;

    for (int i = 0; i < block->particles_count; i++)
        indices[i] =
            (int)((1.0f - lifetimes[i] / max_lifetimes[i]) * block->num_frames);
}

void
update_indices(DataBlock *block)
{
#if !defined(__EMSCRIPTEN__)
    if (_Has_AVX2()) {
        update_indices_avx2(block);
        return;
    }

#if ENABLE_SSE_NEON
    if (_HasSSE_NEON()) {
        update_indices_sse2(block);
        return;
    }
#endif /* ENABLE_SSE_NEON */
#endif /* __EMSCRIPTEN__ */

    update_indices_scalar(block);
}

int FORCEINLINE
RectEmpty(const SDL_Rect *r)
{
    return (r->w <= 0) || (r->h <= 0);
}

int FORCEINLINE
IntersectRect(int Amin_x, int Amax_x, const int Bmin_x, const int Bmax_x, int Amin_y,
              int Amax_y, const int Bmin_y, const int Bmax_y, SDL_Rect *result)
{
    /* Horizontal intersection */
    if (Bmin_x > Amin_x)
        Amin_x = Bmin_x;
    result->x = Amin_x;
    if (Bmax_x < Amax_x)
        Amax_x = Bmax_x;
    result->w = Amax_x - Amin_x;

    /* Vertical intersection */
    if (Bmin_y > Amin_y)
        Amin_y = Bmin_y;
    result->y = Amin_y;
    if (Bmax_y < Amax_y)
        Amax_y = Bmax_y;
    result->h = Amax_y - Amin_y;

    return !RectEmpty(result);
}