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
} ParticleGroup;

void
dealloc_group(ParticleGroup *g)
{
    /* free the particles array first */
    PyMem_Free(g->particles);

    /* decref each image composing the animation and free the array */
    for (Py_ssize_t i = 0; i < g->n_images; i++)
        Py_DECREF(g->images[i]);
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

int
initialize_group_from_PointMode(ParticleGroup *g, int number, double x,
                                double y, double radius, double vx_min,
                                double vx_max, double vy_min, double vy_max,
                                double gx, double gy, int rand_x, int rand_y,
                                PyObject **images, Py_ssize_t images_len)
{
    if (!g->particles) {
        g->particles = PyMem_New(Particle, number);
        if (!g->particles)
            return 0;
    }

    g->n_size = number;

    g->images = PyMem_New(PyObject *, images_len);
    if (!g->images)
        return 0;

    Py_ssize_t i;
    for (i = 0; i < images_len; i++) {
        Py_INCREF(images[i]);
        g->images[i] = images[i];
    }

    g->n_images = images_len;

    for (i = 0; i < g->n_size; i++) {
        Particle *p = &g->particles[i];
        p->x = (float)x;
        p->y = (float)y;
        p->vx = (float)vx_min;
        p->vy = (float)vy_min;
        p->img_ix = 0;
    }

    g->blend_flag = 0;

    return 1;
}