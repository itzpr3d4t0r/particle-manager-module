#include "include/data_block.h"

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
    switch (emitter->spawn_shape) {
        case POINT:
            for (int i = 0; i < emitter->emission_number; i++) {
                x->data[i] = position.x;
                y->data[i] = position.y;
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
    int alloc_x = emitter->acceleration_x.in_use;
    int alloc_y = emitter->acceleration_y.in_use;

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
    int i;
    if (alloc_x && !alloc_y) {
        for (i = 0; i < emitter->emission_number; i++)
            x->data[i] = genrand_from(&emitter->acceleration_x);
    }
    else if (!alloc_x && alloc_y) {
        for (i = 0; i < emitter->emission_number; i++)
            y->data[i] = genrand_from(&emitter->acceleration_y);
    }
    else {
        for (i = 0; i < emitter->emission_number; i++) {
            x->data[i] = genrand_from(&emitter->acceleration_x);
            y->data[i] = genrand_from(&emitter->acceleration_y);
        }
    }

    return 1;
}

int
alloc_and_init_lifetimes(DataBlock *block, Emitter *emitter)
{
    /* Lifetimes must be allocated since no particle can last forever */
    float_array *lifetimes = &block->lifetimes;

    /* Allocate memory for the lifetimes array */
    if (!float_array_alloc(lifetimes, emitter->emission_number))
        return 0;

    /* Initialize the lifetimes array */
    for (int i = 0; i < emitter->emission_number; i++)
        lifetimes->data[i] = genrand_from(&emitter->lifetime);

    return 1;
}

void
UDB_no_acceleration(DataBlock *block, float dt)
{
    /* TODO: These need to dispatch the appropriate SIMD functions if possible */
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float const *restrict velocities_x = block->velocities_x.data;
    float const *restrict velocities_y = block->velocities_y.data;

    for (int i = 0; i < block->particles_count; i++) {
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        block->lifetimes.data[i] -= dt;
    }
}

void
UDB_all(DataBlock *block, float dt)
{
    /* TODO: These need to dispatch the appropriate SIMD functions if possible */
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float const *restrict accelerations_x = block->accelerations_x.data;
    float const *restrict accelerations_y = block->accelerations_y.data;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        block->lifetimes.data[i] -= dt;
    }
}

void
UDB_acceleration_x(DataBlock *block, float dt)
{
    /* TODO: These need to dispatch the appropriate SIMD functions if possible */
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float *restrict velocities_x = block->velocities_x.data;
    float const *restrict velocities_y = block->velocities_y.data;
    float const *restrict accelerations_x = block->accelerations_x.data;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_x[i] += accelerations_x[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        block->lifetimes.data[i] -= dt;
    }
}

void
UDB_acceleration_y(DataBlock *block, float dt)
{
    /* TODO: These need to dispatch the appropriate SIMD functions if possible */
    float *restrict positions_x = block->positions_x.data;
    float *restrict positions_y = block->positions_y.data;
    float const *restrict velocities_x = block->velocities_x.data;
    float *restrict velocities_y = block->velocities_y.data;
    float const *restrict accelerations_y = block->accelerations_y.data;

    for (int i = 0; i < block->particles_count; i++) {
        velocities_y[i] += accelerations_y[i] * dt;
        positions_x[i] += velocities_x[i] * dt;
        positions_y[i] += velocities_y[i] * dt;

        block->lifetimes.data[i] -= dt;
    }
}

int
init_data_block(DataBlock *block, Emitter *emitter, vec2 position)
{
    block->particles_count = emitter->emission_number;

    /* Conditionally allocate memory for the arrays based on emitter properties */
    if (!alloc_and_init_positions(block, emitter, position) ||
        !alloc_and_init_velocities(block, emitter) ||
        !alloc_and_init_accelerations(block, emitter) ||
        !alloc_and_init_lifetimes(block, emitter))
        return 0;

    /* Set the updater function */
    if (!emitter->acceleration_x.in_use && !emitter->acceleration_y.in_use)
        block->updater = UDB_no_acceleration;
    else if (emitter->acceleration_x.in_use && !emitter->acceleration_y.in_use)
        block->updater = UDB_acceleration_x;
    else if (!emitter->acceleration_x.in_use && emitter->acceleration_y.in_use)
        block->updater = UDB_acceleration_y;
    else
        block->updater = UDB_all;

    return 1;
}

void
update_data_block(DataBlock *block, float dt)
{
    block->updater(block, dt);
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
}