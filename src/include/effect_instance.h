#pragma once

#include "base.h"
#include "float_array.h"
#include "MT19937.h"
#include "data_block.h"
#include "particle_effect.h"

typedef struct {
    ParticleEffect *effect; /* particle effect */
    DataBlock *p_data;      /* particles data blocks */
    int blocks_count;       /* number of data blocks */
    vec2 position;          /* position of the effect */
    bool ended;             /* if the effect has ended */
} EffectInstance;

int
init_effect_instance(EffectInstance *instance, ParticleEffect *effect,
                     vec2 position);

void
update_effect_instance(EffectInstance *instance, float dt);

int
draw_effect_instance(EffectInstance *instance, pgSurfaceObject *dest);

void
dealloc_effect_instance(EffectInstance *g);
