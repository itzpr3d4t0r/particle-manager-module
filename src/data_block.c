#include "include/data_block.h"
#include <search.h>
#include "include/simd_common.h"

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
        case POINT:
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
    /* Check if the emitter has no speed */
    if (emitter->speed_x.min == 0.0f && emitter->speed_x.max == 0.0f &&
        emitter->speed_y.min == 0.0f && emitter->speed_y.max == 0.0f)
        return 1;

    float_array *x = &block->velocities_x;
    float_array *y = &block->velocities_y;

    /* Allocate memory for the velocities array */
    if (!float_array_alloc(x, emitter->emission_number) ||
        !float_array_alloc(y, emitter->emission_number))
        return 0;

    /* Initialize the velocities array */
    for (int i = 0; i < emitter->emission_number; i++) {
        x->data[i] = genrand_from(&emitter->speed_x);
        y->data[i] = genrand_from(&emitter->speed_y);
    }

    return 1;
}

int
alloc_and_init_accelerations(DataBlock *block, Emitter *emitter)
{
    /* Check if the emitter has no acceleration */
    const int alloc_x = emitter->acceleration_x.in_use;
    const int alloc_y = emitter->acceleration_y.in_use;

    if (!alloc_x && !alloc_y)
        return 1;

    float_array *x = &block->accelerations_x;
    float_array *y = &block->accelerations_y;

    /* Allocate memory for the accelerations arrays */
    if (alloc_x && !float_array_alloc(x, emitter->emission_number))
        return 0;

    if (alloc_y && !float_array_alloc(y, emitter->emission_number))
        return 0;

    /* Initialize the accelerations arrays */
    for (int i = 0; i < emitter->emission_number; i++) {
        if (alloc_x)
            x->data[i] = genrand_from(&emitter->acceleration_x);
        if (alloc_y)
            y->data[i] = genrand_from(&emitter->acceleration_y);
    }

    return 1;
}

int
_compare_desc(const void *a, const void *b)
{
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa < fb) - (fa > fb);
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
        lifetimes->data[i] = genrand_from(&emitter->lifetime);

    /* Sort lifetimes in descending order */
    qsort(lifetimes->data, num_particles, sizeof(float), _compare_desc);

    /* Copy the sorted lifetimes to the max_lifetimes array */
    memcpy(max_lifetimes->data, lifetimes->data, sizeof(float) * num_particles);

    return 1;
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

int
alloc_and_init_animation_indices(DataBlock *block, Emitter *emitter)
{
    block->animation_indices = PyMem_New(int, emitter->emission_number);
    if (!block->animation_indices)
        return 0;

    int *restrict animation_indices = block->animation_indices;

    for (int i = 0; i < emitter->emission_number; i++)
        animation_indices[i] = rand_int_between(0, emitter->animations_count - 1);

    return 1;
}

void
recalculate_particle_count(DataBlock *block)
{
    /* Find the first particle with a lifetime less than or equal to zero */
    int index = find_first_leq_zero(block->lifetimes.data, block->particles_count);

    /* If all particles are alive the index will be -1, so just return */
    if (index == -1 && block->particles_count == block->lifetimes.capacity)
        return;

    if (index == 0) {
        block->particles_count = 0;
        block->ended = true;
        return;
    }

    block->particles_count = index + 1;
}

void
UDB_no_acceleration(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float const *restrict velocities_x = block->velocities_x.data;
    float const *restrict velocities_y = block->velocities_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    for (int i = 0; i < block->particles_count; i++) {
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
    }

    recalculate_particle_count(block);
}

void
UDB_all(DataBlock *block, float dt)
{
    float *positions_x = block->positions_x.data;
    float *positions_y = block->positions_y.data;
    float *velocities_x = block->velocities_x.data;
    float *velocities_y = block->velocities_y.data;
    float const *accelerations_x = block->accelerations_x.data;
    float const *accelerations_y = block->accelerations_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
    }

    recalculate_particle_count(block);
}

void
UDB_acceleration_x(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float const *restrict velocities_y = block->velocities_y.data;
    float const *restrict accelerations_x = block->accelerations_x.data;
    float *restrict lifetimes = block->lifetimes.data;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
    }

    recalculate_particle_count(block);
}

void
UDB_acceleration_y(DataBlock *block, float dt)
{
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float const *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float const *restrict accelerations_y = block->accelerations_y.data;
    float *restrict lifetimes = block->lifetimes.data;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        lifetimes[i] -= dt;
    }

    recalculate_particle_count(block);
}

void
choose_and_set_update_function(DataBlock *block, Emitter *emitter)
{
    const bool use_x_acc = emitter->acceleration_x.in_use;
    const bool use_y_acc = emitter->acceleration_y.in_use;

#if !defined(__EMSCRIPTEN__)
    if (_Has_AVX2()) {
        if (!use_x_acc && !use_y_acc)
            block->updater = UDB_no_acceleration_avx2;
        else if (use_x_acc && !use_y_acc)
            block->updater = UDB_acceleration_x_avx2;
        else if (!use_x_acc && use_y_acc)
            block->updater = UDB_acceleration_y_avx2;
        else
            block->updater = UDB_all_avx2;

        return;
    }

#if ENABLE_SSE_NEON
    if (_HasSSE_NEON()) {
        if (!use_x_acc && !use_y_acc)
            block->updater = UDB_no_acceleration_sse2;
        else if (use_x_acc && !use_y_acc)
            block->updater = UDB_acceleration_x_sse2;
        else if (!use_x_acc && use_y_acc)
            block->updater = UDB_acceleration_y_sse2;
        else
            block->updater = UDB_all_sse2;

        return;
    }
#endif /* ENABLE_SSE_NEON */
#endif /* __EMSCRIPTEN__ */

    if (!use_x_acc && !use_y_acc)
        block->updater = UDB_no_acceleration;
    else if (use_x_acc && !use_y_acc)
        block->updater = UDB_acceleration_x;
    else if (!use_x_acc && use_y_acc)
        block->updater = UDB_acceleration_y;
    else
        block->updater = UDB_all;
}

int
init_data_block(DataBlock *block, Emitter *emitter, vec2 position)
{
    block->particles_count = emitter->emission_number;
    block->animations = emitter->animations;
    block->num_frames = emitter->num_frames;
    block->blend_mode = emitter->blend_mode;
    block->ended = false;

    /* Conditionally allocate memory for the arrays based on emitter properties */
    if (!alloc_and_init_positions(block, emitter, position) ||
        !alloc_and_init_velocities(block, emitter) ||
        !alloc_and_init_accelerations(block, emitter) ||
        !alloc_and_init_lifetimes(block, emitter) ||
        !alloc_and_init_animation_indices(block, emitter))
        return 0;

    choose_and_set_update_function(block, emitter);

    return 1;
}

void
update_data_block(DataBlock *block, float dt)
{
    block->updater(block, dt);
}

int
draw_data_block(DataBlock *block, pgSurfaceObject *dest, const int blend_flag)
{
    pgSurfaceObject *src_obj;
    float const *restrict positions_x = block->positions_x.data;
    float const *restrict positions_y = block->positions_y.data;
    float const *restrict lifetimes = block->lifetimes.data;
    float const *restrict max_lifetimes = block->max_lifetimes.data;
    int const *restrict animation_indices = block->animation_indices;
    int const *restrict num_frames = block->num_frames;
    PyObject const ***animations = block->animations;

    for (int i = 0; i < block->particles_count; i++) {
        const int animation_index = animation_indices[i];
        const int max_ix = num_frames[animation_index];
        int img_ix = (int)((1 - lifetimes[i] / max_lifetimes[i]) * max_ix);
        img_ix = MAX(0, MIN(img_ix, max_ix - 1));

        src_obj = (pgSurfaceObject *)(animations[animation_index][img_ix]);
        if (!src_obj) {
            PyErr_SetString(PyExc_RuntimeError, "Surface is not initialized");
            return 0;
        }

        const SDL_Surface *surf = src_obj->surf;

        SDL_Rect dst_rect = {(int)positions_x[i] - surf->w / 2,
                             (int)positions_y[i] - surf->h / 2, surf->w, surf->h};

        if (pgSurface_Blit(dest, src_obj, &dst_rect, NULL, blend_flag))
            return 0;
    }

    return 1;
}

void
dealloc_data_block(DataBlock *block)
{
    float_array_free(&block->positions_x);
    float_array_free(&block->positions_y);
    float_array_free(&block->velocities_x);
    float_array_free(&block->velocities_y);
    float_array_free(&block->accelerations_x);
    float_array_free(&block->accelerations_y);
    float_array_free(&block->lifetimes);
    float_array_free(&block->max_lifetimes);
    PyMem_Free(block->animation_indices);
}