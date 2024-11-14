#pragma once

#include "float_array.h"
#include "MT19937.h"
#include "emitter.h"

typedef struct DataBlock {
    float_array positions_x;
    float_array positions_y;
    float_array velocities_x;
    float_array velocities_y;
    float_array accelerations_x;
    float_array accelerations_y;
    float_array lifetimes;
    float_array max_lifetimes;

    int *animation_indices;
    int *num_frames;
    PyObject ***animations;

    int blend_mode;
    bool ended;

    int particles_count;
    void (*updater)(struct DataBlock *, float);
} DataBlock;

void
UDB_no_acceleration(DataBlock *block, float dt);

void
UDB_acceleration_x(DataBlock *block, float dt);

void
UDB_acceleration_y(DataBlock *block, float dt);

void
UDB_all(DataBlock *block, float dt);

int
init_data_block(DataBlock *block, Emitter *emitter, vec2 position);

int
alloc_and_init_positions(DataBlock *block, Emitter *emitter, vec2 position);

int
alloc_and_init_velocities(DataBlock *block, Emitter *emitter);

int
alloc_and_init_accelerations(DataBlock *block, Emitter *emitter);

int
alloc_and_init_lifetimes(DataBlock *block, Emitter *emitter);

int
alloc_and_init_animation_indices(DataBlock *block, Emitter *emitter);

void
update_data_block(DataBlock *block, float dt);

void
choose_and_set_update_function(DataBlock *block, Emitter *emitter);

int
draw_data_block(DataBlock *block, pgSurfaceObject *dest, const int blend_flag);

void
recalculate_particle_count(DataBlock *block);

void
dealloc_data_block(DataBlock *block);