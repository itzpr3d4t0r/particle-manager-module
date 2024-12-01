#pragma once

#include "emitter.h"

typedef struct {
    PyObject *emitters; /* emitters tuple */
    int emitters_count; /* number of emitters */
} ParticleEffect;

typedef struct {
    PyObject_HEAD ParticleEffect effect;
} ParticleEffectObject;

extern PyTypeObject ParticleEffect_Type;

#define ParticleEffect_Check(o) (Py_TYPE(o) == &ParticleEffect_Type)

int
particle_effect_init(ParticleEffectObject *self, PyObject *args, PyObject *kwds);

void
particle_effect_dealloc(ParticleEffectObject *self);

PyObject *
particle_effect_str(ParticleEffectObject *self);