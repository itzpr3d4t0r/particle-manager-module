#pragma once

#include "float_array.h"
#include "MT19937.h"
#include "emitter.h"

#define UNROLL_2(x) \
    x;              \
    x;

#define UNROLL_3(x) \
    x;              \
    x;              \
    x;

#define UNROLL_4(x) \
    x;              \
    x;              \
    x;              \
    x;

typedef struct {
    int animation_index;
    int length;
} Fragment;

typedef struct {
    uint32_t *pixels;
    int width, rows, src_offset;
} BlitDestination;

typedef struct {
    Fragment *fragments;
    BlitDestination *destinations;
    int used_f;
    int alloc_f;
    int dest_count;
} FragmentationMap;

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

    int num_frames;
    PyObject *animation;
    FragmentationMap frag_map;

    int blend_mode;
    bool ended;

    int particles_count;
    void (*updater)(struct DataBlock *, float);
} DataBlock;

/* ====================| Public facing DataBlock functions |==================== */
int
init_data_block(DataBlock *block, Emitter *emitter, vec2 position);

void
dealloc_data_block(DataBlock *block);

void
choose_and_set_update_function(DataBlock *block, Emitter *emitter);

void
update_data_block(DataBlock *block, float dt);

int
draw_data_block(DataBlock *block, pgSurfaceObject *dest, const int blend_flag);

/* ====================| Internal DataBlock functions |==================== */

int
init_fragmentation_map(DataBlock *block);

void
dealloc_fragmentation_map(FragmentationMap *frag_map);

int
calculate_fragmentation_map(pgSurfaceObject *dest, DataBlock *block);

void
blit_fragments_blitcopy(FragmentationMap *frag_map, pgSurfaceObject *dest,
                        DataBlock *block);

void
blit_fragments_add(FragmentationMap *frag_map, pgSurfaceObject *dest,
                   DataBlock *block);

void
blit_fragments(pgSurfaceObject *dest, FragmentationMap *frag_map, DataBlock *block,
               int blend_flags);

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
update_with_acceleration(DataBlock *block, float dt);

void
update_with_no_acceleration(DataBlock *block, float dt);

void
update_with_acceleration_x(DataBlock *block, float dt);

void
update_with_acceleration_y(DataBlock *block, float dt);

void
update_indices_scalar(DataBlock *block);

void
update_indices(DataBlock *block);

void
recalculate_particle_count(DataBlock *block);

void
blit_fragments_add_scalar(FragmentationMap *frag_map, PyObject **animation,
                          int dst_skip);