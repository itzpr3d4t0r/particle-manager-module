#pragma once
#include "base.h"

typedef struct {
    vec2 pos;
    vec2 vel;
    vec2 acc;
    float update_speed;
    float energy;
} Particle;

static void FORCEINLINE
particle_update(Particle *p, float dt, vec2 gravity)
{
    p->pos.x += (p->vel.x += gravity.x + p->acc.x) * dt;
    p->pos.y += (p->vel.y += gravity.y + p->acc.y) * dt;
    p->energy = MAX(0.0f, p->energy - p->update_speed * dt);
}