#pragma once
#include "base.h"

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float energy;
} Particle;

static void FORCEINLINE
particle_move(Particle *p, float dt)
{
    p->x += p->vx * dt;
    p->y += p->vy * dt;
}