#pragma once

#include "particle_group.h"
#include "MT19937.h"
#include <math.h>

#define PM_BASE_GROUP_SIZE 10

typedef struct {
    PyObject_HEAD ParticleGroup *groups;
    Py_ssize_t g_alloc; /* number of allocated groups */
    Py_ssize_t g_used;  /* number of currently used groups */
} ParticleManager;

enum {
    SPAWN_POINT,  /* indicates spawning at a single point */
    SPAWN_RADIUS, /* indicates spawning within or on the border of a circle */
    SPAWN_RECT,   /* indicates spawning within or on the border of a rectangle */
    SPAWN_LINE,   /* indicates spawning along a line */
};

PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

void
pm_dealloc(ParticleManager *self);

/* =======================| INTERNAL FUNCTIONALITY |======================= */

int
_pm_g_add_point(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs);

int
_pm_add_group(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs);

/* ======================================================================== */

PyObject *
pm_add_group(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs);

PyObject *
pm_update(ParticleManager *self, PyObject *arg);

PyObject *
pm_draw(ParticleManager *self, PyObject *arg);

PyObject *
pm_str(ParticleManager *self);

PyObject *
pm_get_num_particles(ParticleManager *self, void *closure);

PyObject *
pm_get_num_groups(ParticleManager *self, void *closure);

PyObject *
pm_get_groups(ParticleManager *self, void *closure);

/* ===================================================================== */
