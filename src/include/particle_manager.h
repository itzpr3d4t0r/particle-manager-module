#pragma once
#include "particle_group.h"
#include "MT19937.h"

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

static PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ParticleManager *self = (ParticleManager *)type->tp_alloc(type, 0);
    if (self) {
        self->groups = PyMem_New(ParticleGroup, PM_BASE_GROUP_SIZE);
        if (!self->groups) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }

        memset(self->groups, 0, sizeof(ParticleGroup) * PM_BASE_GROUP_SIZE);
        self->g_alloc = PM_BASE_GROUP_SIZE;
        self->g_used = 0;
    }

    return (PyObject *)self;
}

static void
pm_dealloc(ParticleManager *self)
{
    for (Py_ssize_t i = 0; i < self->g_used; i++)
        dealloc_group(&self->groups[i]);

    PyMem_Free(self->groups);

    Py_TYPE(self)->tp_free((PyObject *)self);
}
