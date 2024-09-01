#pragma once
#include "particle.h"

typedef struct {
    Particle *particles;
    Py_ssize_t n_particles;
    PyObject **images;
    Py_ssize_t n_images;
    int blend_flag;
    float grav_x;
    float grav_y;
} ParticleGroup;

void
dealloc_group(ParticleGroup *g)
{
    /* free the particles array first */
    if (g->particles)
        PyMem_Free(g->particles);

    if (g->images) {
        for (Py_ssize_t k = 0; k < g->n_images; k++)
            Py_DECREF(g->images[k]);

        PyMem_Free(g->images);
    }
}

PyObject *
group_str(ParticleGroup *g)
{
    PyObject *gx, *gy;
    gx = PyFloat_FromDouble(g->grav_x);
    if (!gx)
        return NULL;
    gy = PyFloat_FromDouble(g->grav_y);
    if (!gy) {
        Py_DECREF(gx);
        return NULL;
    }

    PyObject *str = PyUnicode_FromFormat(
        "<particles: %zd | images: %zd | blend: %d | grav: (%R, %R)>",
        g->n_particles, g->n_images, g->blend_flag, gx, gy);

    Py_DECREF(gx);
    Py_DECREF(gy);

    return str;
}

PyObject *
pythonify_group(ParticleGroup *g)
{
    PyObject *tup = PyTuple_New(2);
    if (!tup)
        return NULL;

    PyObject *flag_obj = PyLong_FromLong(g->blend_flag);
    if (!flag_obj) {
        Py_DECREF(tup);
        return NULL;
    }
    PyTuple_SET_ITEM(tup, 1, flag_obj);

    PyObject *blit_list = PyList_New(g->n_particles);
    if (!blit_list) {
        Py_DECREF(tup);
        return NULL;
    }

    for (Py_ssize_t i = 0; i < g->n_particles; i++) {
        Particle *p = &g->particles[i];

        PyObject *blit_item = PyTuple_New(2);
        if (!blit_item) {
            Py_DECREF(blit_list);
            Py_DECREF(tup);
            return NULL;
        }

        PyObject *img = g->images[(int)p->energy];
        Py_INCREF(img);
        PyTuple_SET_ITEM(blit_item, 0, img);

        PyObject *pos = TupleFromDoublePair(p->x, p->y);
        if (!pos) {
            Py_DECREF(blit_list);
            Py_DECREF(tup);
            return NULL;
        }
        PyTuple_SET_ITEM(blit_item, 1, pos);

        PyList_SET_ITEM(blit_list, i, blit_item);
    }

    PyTuple_SET_ITEM(tup, 0, blit_list);

    return tup;
}

PyObject *
get_group_str_list(ParticleGroup *groups, Py_ssize_t n_groups)
{
    PyObject *list = PyList_New(n_groups);
    if (!list)
        return NULL;

    for (Py_ssize_t i = 0; i < n_groups; i++) {
        PyObject *g = group_str(&groups[i]);
        if (!g) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, g);
    }

    return list;
}