#pragma once

#include "float_array.h"
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

typedef enum {
    // Velocity flags
    VELOCITY_X_SINGLE = 1 << 0,  // Single s_velocity_x
    VELOCITY_Y_SINGLE = 1 << 1,  // Single s_velocity_y
    VELOCITY_X_ARRAY = 1 << 2,   // Velocity x array
    VELOCITY_Y_ARRAY = 1 << 3,   // Velocity y array

    // Acceleration flags
    ACCEL_X_SINGLE = 1 << 4,  // Single s_acceleration_x
    ACCEL_Y_SINGLE = 1 << 5,  // Single s_acceleration_y
    ACCEL_X_ARRAY = 1 << 6,   // Acceleration x array
    ACCEL_Y_ARRAY = 1 << 7,   // Acceleration y array

    NO_ACCELERATION = 1 << 8,     // No acceleration
    NO_ACCELERATION_X = 1 << 9,   // No acceleration x
    NO_ACCELERATION_Y = 1 << 10,  // No acceleration y
    NO_VELOCITY = 1 << 11,        // No velocity
} UpdateFunctionFlags;

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

    float s_velocity_x;
    float s_velocity_y;
    float s_acceleration_x;
    float s_acceleration_y;
    float s_lifetime;

    int num_frames;
    PyObject *animation;
    FragmentationMap frag_map;

    int blend_mode;
    bool ended;

    int particles_count;
    int update_flags;
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

void
calculate_surface_index_occurrences(DataBlock *block);

int
populate_destinations_array(pgSurfaceObject *dest, DataBlock *block);

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

int FORCEINLINE
RectEmpty(const SDL_Rect *r);

int FORCEINLINE
IntersectRect(int Amin_x, int Amax_x, const int Bmin_x, const int Bmax_x, int Amin_y,
              int Amax_y, const int Bmin_y, const int Bmax_y, SDL_Rect *result);
