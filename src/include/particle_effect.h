#pragma once

#include "emitter.h"

typedef struct {
    Emitter *emitter; /* emitter array */
    int e_alloc;      /* number of allocated emitters */
    int e_used;       /* number of currently used emitters */
    bool done;        /* indicates whether the effect is done */
} ParticleEffect;

typedef struct {
    PyObject_HEAD ParticleEffect effect;
} ParticleEffectObject;

extern PyTypeObject ParticleEffect_Type;

#define ParticleEffect_Check(o) (Py_TYPE(o) == &ParticleEffect_Type)

int
particle_effect_init(ParticleEffectObject *self, PyObject *args, PyObject *kwds);

PyObject *
particle_effect_str(ParticleEffectObject *self);