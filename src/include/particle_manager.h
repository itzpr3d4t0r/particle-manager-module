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

/* =======================| INTERNAL FUNCTIONALITY |======================= */

static int
_pm_g_add_point(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    int n_particles;
    float x, y;
    Py_ssize_t imgs_list_size;

    generator vel_x_g = {0};
    generator vel_y_g = {0};
    generator acc_x_g = {0};
    generator acc_y_g = {0};
    generator uspeed_g = {1.0, 0.0, 0};
    generator time_g = {0};

    vec2 gravity = {0};

    if (!IntFromObj(args[0], &n_particles) || n_particles <= 0)
        return IRAISE(PyExc_TypeError, "Invalid number of particles");

    if (!TwoFloatsFromObj(args[1], &x, &y))
        return IRAISE(PyExc_TypeError, "Invalid type for paramenter: pos");

    if (!PyList_Check(args[2]) || (imgs_list_size = PyList_GET_SIZE(args[2])) == 0)
        return IRAISE(PyExc_TypeError, "Invalid images list");

    switch (nargs) {
        case 10:
            if (!TwoFloatsFromObj(args[9], &gravity.x, &gravity.y))
                return IRAISE(PyExc_TypeError,
                              "Invalid gravity. Must be a tuple of 2 floats");
        case 9:
            if (!TwoFloatsAndBoolFromTuple(args[8], &time_g.min, &time_g.max,
                                           &time_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid time settings, must be a tuple of 2 floats "
                              "and a bool");
        case 8:
            if (!TwoFloatsAndBoolFromTuple(args[7], &uspeed_g.min, &uspeed_g.max,
                                           &uspeed_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid update_speed settings, must be a tuple of 2 "
                              "floats and a bool");
        case 7:
            if (!TwoFloatsAndBoolFromTuple(args[6], &acc_y_g.min, &acc_y_g.max,
                                           &acc_y_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid acc_y settings, must be a tuple of 2 floats "
                              "and a bool");
        case 6:
            if (!TwoFloatsAndBoolFromTuple(args[5], &acc_x_g.min, &acc_x_g.max,
                                           &acc_x_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid acc_x settings, must be a tuple of 2 floats "
                              "and a bool");
        case 5:
            if (!TwoFloatsAndBoolFromTuple(args[4], &vel_y_g.min, &vel_y_g.max,
                                           &vel_y_g.randomize))
                return IRAISE(
                    PyExc_TypeError,
                    "Invalid vy settings, must be a tuple of 2 floats and a bool");
        case 4:
            if (!TwoFloatsAndBoolFromTuple(args[3], &vel_x_g.min, &vel_x_g.max,
                                           &vel_x_g.randomize))
                return IRAISE(
                    PyExc_TypeError,
                    "Invalid vx settings, must be a tuple of 2 floats and a bool");
    }

    PyObject **list_items = PySequence_Fast_ITEMS(args[2]);
    if (!init_group(group, n_particles, list_items, imgs_list_size))
        return IRAISE(PyExc_MemoryError,
                      "Failed to allocate enough memory for group");

    setup_particles_point(group, x, y, &vel_x_g, &vel_y_g, &acc_x_g, &acc_y_g,
                          &time_g, &uspeed_g);

    group->gravity = gravity;

    return 1;
}

static int
_pm_add_group(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    if (!IntFromObj(args[0], &group->blend_flag)) {
        PyErr_SetString(PyExc_TypeError, "Invalid blend_flag type");
        return 0;
    }

    int kind;
    if (!IntFromObj(args[1], &kind)) {
        PyErr_SetString(PyExc_TypeError, "Invalid spawn_type type");
        return 0;
    }

    nargs -= 2;

    switch (kind) {
        case SPAWN_POINT:
            if (nargs < 3 || nargs > 10) {
                PyErr_SetString(PyExc_TypeError,
                                "SPAWN_POINT spawn_type requires between 3 "
                                "and 10 arguments");
                return 0;
            }
            return _pm_g_add_point(group, args + 2, nargs);
        default:
            PyErr_SetString(PyExc_NotImplementedError,
                            "The supplied spawn_type doesn't exist.");
            return 0;
    }

    return 1;
}

/* ======================================================================== */

static PyObject *
pm_add_group(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs)
{
    if (nargs <= 2) {
        PyErr_SetString(PyExc_TypeError, "Invalid number of arguments");
        return NULL;
    }

    if (self->g_used + 1 > self->g_alloc) {
        self->g_alloc *= 2;
        self->groups = PyMem_Resize(self->groups, ParticleGroup, self->g_alloc);
        if (!self->groups)
            return PyErr_NoMemory();
    }

    ParticleGroup *group = &self->groups[self->g_used];

    if (!_pm_add_group(group, args, nargs))
        return NULL;

    self->g_used++;

    Py_RETURN_NONE;
}

static PyObject *
pm_update(ParticleManager *self, PyObject *arg)
{
    float dt;
    if (!FloatFromObj(arg, &dt))
        return RAISE(PyExc_TypeError, "Invalid dt parameter, must be nmumeric");

    Py_ssize_t i, j;
    for (j = 0; j < self->g_used; j++) {
        ParticleGroup *g = &self->groups[j];
        float *g_pos = g->p_pos.data;
        float *g_vel = g->p_vel.data;
        float *g_acc = g->p_acc.data;

        for (i = 0; i < g->n_particles; i++) {
            g_acc[i * 2] += g->gravity.x;
            g_acc[i * 2 + 1] += g->gravity.y;
            g_vel[i * 2] += g_acc[i * 2] * dt;
            g_vel[i * 2 + 1] += g_acc[i * 2 + 1] * dt;
            g_pos[i * 2] += g_vel[i * 2] * dt;
            g_pos[i * 2 + 1] += g_vel[i * 2 + 1] * dt;
            //            g->p_time.data[i] += dt * g->u_fac.data[i];
        }
    }

    Py_RETURN_NONE;
}

static PyObject *
pm_str(ParticleManager *self)
{
    Py_ssize_t n = 0, i;
    for (i = 0; i < self->g_used; i++)
        n += self->groups[i].n_particles;

    return PyUnicode_FromFormat("ParticleManager(groups: %d, tot_particles: %d)",
                                self->g_used, n);
}

static PyObject *
pm_get_num_particles(ParticleManager *self, void *closure)
{
    Py_ssize_t n = 0, i;
    for (i = 0; i < self->g_used; i++)
        n += self->groups[i].n_particles;

    return PyLong_FromSsize_t(n);
}

static PyObject *
pm_get_num_groups(ParticleManager *self, void *closure)
{
    return PyLong_FromSsize_t(self->g_used);
}

static PyObject *
pm_get_groups(ParticleManager *self, void *closure)
{
    PyObject *list = PyList_New(self->g_used);
    if (!list)
        return PyErr_NoMemory();

    for (Py_ssize_t i = 0; i < self->g_used; i++) {
        PyObject *group = pythonify_group(&self->groups[i]);
        if (!group) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, group);
    }

    return list;
}

/* ===================================================================== */

static PyMethodDef PM_methods[] = {
    {"update", (PyCFunction)pm_update, METH_O, NULL},
    {"add_group", (PyCFunction)pm_add_group, METH_FASTCALL, NULL},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef PM_attributes[] = {
    {"num_particles", (getter)pm_get_num_particles, NULL, NULL, NULL},
    {"num_groups", (getter)pm_get_num_groups, NULL, NULL, NULL},
    {"groups", (getter)pm_get_groups, NULL, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}};

static PyTypeObject ParticleManagerType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "particle_manager.ParticleManager",
    .tp_doc = "Particle Manager",
    .tp_basicsize = sizeof(ParticleManager),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)pm_new,
    .tp_str = (reprfunc)pm_str,
    .tp_repr = (reprfunc)pm_str,
    .tp_dealloc = (destructor)pm_dealloc,
    .tp_methods = PM_methods,
    .tp_getset = PM_attributes,
};
