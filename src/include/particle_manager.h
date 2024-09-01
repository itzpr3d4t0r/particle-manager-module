#pragma once
#include "particle_group.h"
#include "MT19937.h"

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
        self->groups = (ParticleGroup *)PyMem_Calloc(10, sizeof(ParticleGroup));
        if (!self->groups) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }
        self->g_alloc = 10;
        self->g_used = 0;
    }

    return (PyObject *)self;
}

static int
pm_init(ParticleManager *self, PyObject *args, PyObject *kwds)
{
    Py_ssize_t alloc_request = 10;
    if (!PyArg_ParseTuple(args, "|i", &alloc_request))
        return -1;

    if (alloc_request < self->g_alloc) {
        PyErr_SetString(PyExc_ValueError, "Cannot allocate less than 10 groups");
        return -1;
    }

    if (alloc_request > self->g_alloc) {
        self->groups = PyMem_Resize(self->groups, ParticleGroup, alloc_request);
        if (!self->groups) {
            PyErr_NoMemory();
            return -1;
        }
        Py_ssize_t width = alloc_request - self->g_alloc;
        memset(self->groups + width, 0, width * sizeof(ParticleGroup));
        self->g_alloc = alloc_request;
    }

    return 0;
}

static void
pm_dealloc(ParticleManager *self)
{
    for (Py_ssize_t i = 0; i < self->g_used; i++)
        dealloc_group(&self->groups[i]);

    PyMem_Free(self->groups);

    Py_TYPE(self)->tp_free((PyObject *)self);
}
