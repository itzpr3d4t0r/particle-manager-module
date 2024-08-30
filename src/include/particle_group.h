#pragma once
#include "particle.h"

typedef enum {
    GroupKind_POINT,
    GroupKind_RECT_AREA,
} GroupKind;

typedef struct {
    Particle *particles;
    Py_ssize_t n_size;
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
    PyMem_Free(g->particles);

    for (Py_ssize_t k = 0; k < g->n_images; k++)
        Py_DECREF(g->images[k]);

    PyMem_Free(g->images);
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

    PyObject *blit_list = PyList_New(g->n_size);
    if (!blit_list) {
        Py_DECREF(tup);
        return NULL;
    }

    for (Py_ssize_t i = 0; i < g->n_size; i++) {
        Particle *p = &g->particles[i];

        PyObject *blit_item = PyTuple_New(2);
        if (!blit_item) {
            Py_DECREF(blit_list);
            Py_DECREF(tup);
            return NULL;
        }

        PyObject *img = g->images[(int)p->img_ix];
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