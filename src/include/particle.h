#pragma once
#include "base.h"

typedef struct {
    vec2 pos;
    vec2 vel;
    float energy;
} Particle;

static void FORCEINLINE
particle_move(Particle *p, float dt, vec2 gravity)
{
    p->pos.x += (p->vel.x += gravity.x) * dt;
    p->pos.y += (p->vel.y += gravity.y) * dt;
}