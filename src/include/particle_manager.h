#pragma once

#include "particle_effect.h"
#include "effect_data_block.h"
#include "MT19937.h"
#include <math.h>

#define PM_BASE_BLOCK_SIZE 10

typedef struct {
    PyObject_HEAD EffectDataBlock *blocks;
    Py_ssize_t allocated_blocks;
    Py_ssize_t used_blocks;
} ParticleManager;

PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

void
pm_dealloc(ParticleManager *self);

/* =======================| INTERNAL FUNCTIONALITY |======================= */

int
_pm_spawn_effect_helper(EffectDataBlock *block, PyObject *const *args,
                        Py_ssize_t nargs);

/* ======================================================================== */

PyObject *
pm_spawn_effect(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs);

PyObject *
pm_update(ParticleManager *self, PyObject *arg);

PyObject *
pm_str(ParticleManager *self);

/* ===================================================================== */
