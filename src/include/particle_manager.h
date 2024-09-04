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
//
// static int
//_pm_g_add_point(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
//{
//    float x, y;
//    float vx_min = 0, vx_max = 0;
//    float vy_min = 0, vy_max = 0;
//    float gx = 0, gy = 0;
//    int rand_x = 0, rand_y = 0;
//    int n_particles;
//    Py_ssize_t imgs_list_size;
//
//    if (!IntFromObj(args[0], &n_particles) || n_particles <= 0)
//        return IRAISE(PyExc_TypeError, "Invalid number of particles");
//
//    if (!TwoFloatsFromObj(args[1], &x, &y))
//        return IRAISE(PyExc_TypeError, "Invalid type for paramenter: pos");
//
//    if (!PyList_Check(args[2]) || (imgs_list_size = PyList_GET_SIZE(args[2])) == 0)
//        return IRAISE(PyExc_TypeError, "Invalid images list");
//
//    switch (nargs) {
//        case 6:
//            if (!TwoFloatsFromObj(args[5], &gx, &gy))
//                return IRAISE(PyExc_TypeError,
//                              "Invalid gravity. Must be a tuple of 2 floats");
//        case 5:
//            if (!TwoFloatsAndBoolFromTuple(args[4], &vy_min, &vy_max, &rand_y))
//                return IRAISE(
//                    PyExc_TypeError,
//                    "Invalid vy settings, must be a tuple of 2 floats and a bool");
//        case 4:
//            if (!TwoFloatsAndBoolFromTuple(args[3], &vx_min, &vx_max, &rand_x))
//                return IRAISE(
//                    PyExc_TypeError,
//                    "Invalid vx settings, must be a tuple of 2 floats and a bool");
//            break;
//    }
//
//    group->gravity = (vec2){gx, gy};
//    group->n_images = imgs_list_size;
//    group->n_particles = n_particles;
//
//    if (!(group->particles = PyMem_New(Particle, n_particles)))
//        return IRAISE(PyExc_MemoryError, "Could not allocate memory for
//        particles");
//
//    Particle *const particles = group->particles;
//
//    for (Py_ssize_t k = 0; k < group->n_particles; k++) {
//        Particle *const p = &particles[k];
//        p->pos = (vec2){x, y};
//        p->vel = (vec2){rand_x ? rand_between(vx_min, vx_max) : vx_min,
//                        rand_y ? rand_between(vy_min, vy_max) : vy_min};
//        p->energy = (float)(group->n_images - 1);
//        p->acc = (vec2){0, 0};
//        p->update_speed = 1.0f;
//    }
//
//    if (!(group->images = PyMem_New(PyObject *, group->n_images)))
//        return IRAISE(PyExc_MemoryError, "Could not allocate memory for images");
//
//    PyObject **list_items = PySequence_Fast_ITEMS(args[2]);
//    for (Py_ssize_t k = 0; k < group->n_images; k++) {
//        Py_INCREF(list_items[k]);
//        group->images[k] = list_items[k];
//    }
//
//    return 1;
//}
//
// static int
//_pm_add_group(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
//{
//    int blend_flag = 0;
//    if (!IntFromObj(args[0], &blend_flag)) {
//        PyErr_SetString(PyExc_TypeError, "Invalid blend_flag type");
//        return 0;
//    }
//    group->blend_flag = blend_flag;
//
//    int kind;
//    if (!IntFromObj(args[1], &kind)) {
//        PyErr_SetString(PyExc_TypeError, "Invalid spawn_type type");
//        return 0;
//    }
//
//    nargs -= 2;
//
//    switch (kind) {
//        case SPAWN_POINT:
//            if (nargs < 3 || nargs > 6) {
//                PyErr_SetString(PyExc_TypeError,
//                                "SPAWN_POINT spawn_type requires between 3 "
//                                "and 6 arguments.");
//                return 0;
//            }
//            return _pm_g_add_point(group, args + 2, nargs);
//        default:
//            PyErr_SetString(PyExc_NotImplementedError,
//                            "The supplied spawn_type doesn't exist.");
//            return 0;
//    }
//
//    return 1;
//}

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

    //    if (!_pm_add_group(group, args, nargs))
    //        return NULL;

    self->g_used++;

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

/* ===================================================================== */

static PyMethodDef PM_methods[] = {
    //    {"update", (PyCFunction)pm_update, METH_O, NULL},
    {"add_group", (PyCFunction)pm_add_group, METH_FASTCALL, NULL},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef PM_attributes[] = {
    {"num_particles", (getter)pm_get_num_particles, NULL, NULL, NULL},
    {"num_groups", (getter)pm_get_num_groups, NULL, NULL, NULL},
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
