#pragma once

#include "base.h"
#include "f_vec.h"
#include "MT19937.h"

typedef struct {
    /* particles data */
    f_arr p_pos;            /* positions array */
    f_arr p_vel;            /* velocities array */
    f_arr p_acc;            /* accelerations array */
    f_arr p_time;           /* times array*/
    f_arr u_fac;            /* update speed factor array */
    Py_ssize_t n_particles; /* number of particles */
    Py_ssize_t max_ix;      /* maximum index of an active particle */

    /* images data */
    PyObject ***images;       /* array of image sequences */
    Py_ssize_t *n_img_frames; /* number of frames in each image sequence */
    Py_ssize_t n_img_sequences;

    /* additional settings */
    int blend_flag;
    vec2 gravity;
} ParticleGroup;

int
init_group(ParticleGroup *g, Py_ssize_t n_particles, Py_ssize_t n_img_sequences)
{
    g->n_particles = n_particles;
    g->n_img_sequences = n_img_sequences;
    g->max_ix = -1;

    if (!farr_init(&g->p_pos, n_particles) || !farr_init(&g->p_vel, n_particles) ||
        !farr_init(&g->p_acc, n_particles) || !farr_init(&g->p_time, n_particles) ||
        !farr_init(&g->u_fac, n_particles))
        return 0;

    INIT_MEMORY(g->images, PyObject **, n_img_sequences, 0, {})
    INIT_MEMORY(g->n_img_frames, Py_ssize_t, n_img_sequences, 0, {})

    memset(g->images, 0, sizeof(PyObject **) * n_img_sequences);
    memset(g->n_img_frames, 0, sizeof(Py_ssize_t) * n_img_sequences);

    g->blend_flag = 0;
    g->gravity = (vec2){0.0f, 0.0f};

    return 1;
}

int
setup_group(ParticleGroup *g, PyObject ***images, Py_ssize_t n_images,
            const generator *pos_g, const generator *vel_g, const generator *acc_g,
            const generator *time_g, const generator *update_speed_g)
{
    Py_ssize_t i, j;

    /* initialize the image sequences */
    for (i = 0; i < n_images; i++) {
        PyObject *seq = images[i];
        if (!PyList_Check(seq)) {
            PyErr_SetString(PyExc_TypeError, "images must be a list of sequences");
            return 0;
        }

        Py_ssize_t n_frames = PySequence_Fast_GET_SIZE(seq);
        g->images[i] = PyMem_New(PyObject *, n_frames);
        if (!g->images[i])
            return 0;

        PyObject **seq_items = PySequence_Fast_ITEMS(seq);
        for (j = 0; j < n_frames; j++) {
            PyObject *img = seq_items[j];
            Py_INCREF(img);
            g->images[i][j] = img;
        }

        g->n_img_frames[i] = n_frames;
    }

    /* initialize the particles from RNG generators */
    for (i = 0; i < g->n_particles; i++) {
        g->p_pos.data[i] = genrand_from(pos_g);
        g->p_vel.data[i] = genrand_from(vel_g);
        g->p_acc.data[i] = genrand_from(acc_g);
        g->p_time.data[i] = genrand_from(time_g);
        g->u_fac.data[i] = genrand_from(update_speed_g);
    }

    return 1;
}

void
dealloc_group(ParticleGroup *g)
{
    farr_free(&g->p_pos);
    farr_free(&g->p_vel);
    farr_free(&g->p_acc);
    farr_free(&g->p_time);
    farr_free(&g->u_fac);

    for (Py_ssize_t i = 0; i < g->n_img_sequences; i++) {
        for (Py_ssize_t j = 0; j < g->n_img_frames[i]; j++)
            Py_DECREF(g->images[i][j]);
        PyMem_Free(g->images[i]);
    }

    PyMem_Free(g->images);
    PyMem_Free(g->n_img_frames);
}
