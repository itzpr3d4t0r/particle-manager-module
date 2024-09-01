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
    SPAWN_POINT,
    SPAWN_RADIUS,
    SPAWN_RECT,
    SPAWN_LINE,
};

static PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ParticleManager *self = (ParticleManager *)type->tp_alloc(type, 0);
    if (self) {
        self->groups =
            (ParticleGroup *)PyMem_Calloc(PM_BASE_GROUP_SIZE, sizeof(ParticleGroup));
        if (!self->groups) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }
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