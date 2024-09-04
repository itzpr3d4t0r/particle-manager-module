#pragma once

#include "base.h"
#include "f_vec.h"
#include "MT19937.h"
#include <immintrin.h>

typedef struct {
    /* particles data */
    f_arr p_pos;    /* positions array */
    f_arr p_vel;    /* velocities array */
    f_arr p_acc;    /* accelerations array */
    f_arr p_time;   /* times array*/
    f_arr u_fac;    /* update speed factor array */
    char *p_img_ix; /* image index array */

    Py_ssize_t n_particles; /* number of particles */
    Py_ssize_t max_ix;      /* maximum index of an active particle */

    /* images data */
    PyObject ***images;       /* array of image sequences */
    Py_ssize_t *n_img_frames; /* array of number of frames in each sequence */
    Py_ssize_t n_img_sequences;

    /* additional settings */
    int blend_flag;
    vec2 gravity;
} ParticleGroup;

int
init_group(ParticleGroup *g, Py_ssize_t n_particles, PyObject **images,
           Py_ssize_t n_img_sequences)
{
    g->n_particles = n_particles;
    g->n_img_sequences = n_img_sequences;
    g->max_ix = -1;

    if (!farr_init(&g->p_pos, 2 * n_particles) ||
        !farr_init(&g->p_vel, 2 * n_particles) ||
        !farr_init(&g->p_acc, 2 * n_particles) ||
        !farr_init(&g->p_time, n_particles) || !farr_init(&g->u_fac, n_particles))
        return 0;

    INIT_MEMORY(g->images, PyObject **, n_img_sequences, 0, {})
    INIT_MEMORY(g->n_img_frames, Py_ssize_t, n_img_sequences, 0, {})
    INIT_MEMORY(g->p_img_ix, char, n_particles, 0, {})

    memset(g->images, 0, sizeof(PyObject **) * n_img_sequences);
    memset(g->n_img_frames, 0, sizeof(Py_ssize_t) * n_img_sequences);

    /* initialize the image sequences */
    Py_ssize_t i, j;
    for (i = 0; i < n_img_sequences; i++) {
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

    g->gravity = (vec2){0.0f, 0.0f};

    return 1;
}

void
setup_particles_general(ParticleGroup *g, const generator *pos_x_g,
                        const generator *pos_y_g, const generator *vel_x_g,
                        const generator *vel_y_g, const generator *acc_x_g,
                        const generator *acc_y_g, const generator *time_g,
                        const generator *update_speed_g)
{
    /* initialize the particles from RNG generators */
    for (Py_ssize_t i = 0; i < g->n_particles; i++) {
        g->p_pos.data[i * 2] = genrand_from(pos_x_g);
        g->p_pos.data[i * 2 + 1] = genrand_from(pos_y_g);

        g->p_vel.data[i * 2] = genrand_from(vel_x_g);
        g->p_vel.data[i * 2 + 1] = genrand_from(vel_y_g);

        g->p_acc.data[i * 2] = genrand_from(acc_x_g);
        g->p_acc.data[i * 2 + 1] = genrand_from(acc_y_g);

        g->p_time.data[i] = genrand_from(time_g);
        g->u_fac.data[i] = genrand_from(update_speed_g);
    }

    g->max_ix = g->n_particles - 1;
}

void
setup_particles_point(ParticleGroup *g, float x, float y, const generator *vel_x_g,
                      const generator *vel_y_g, const generator *acc_x_g,
                      const generator *acc_y_g, const generator *time_g,
                      const generator *update_speed_g)
{
    /* initialize the particles from RNG generators */
    for (Py_ssize_t i = 0; i < g->n_particles; i++) {
        g->p_pos.data[i * 2] = x;
        g->p_pos.data[i * 2 + 1] = y;

        g->p_vel.data[i * 2] = genrand_from(vel_x_g);
        g->p_vel.data[i * 2 + 1] = genrand_from(vel_y_g);

        g->p_acc.data[i * 2] = genrand_from(acc_x_g);
        g->p_acc.data[i * 2 + 1] = genrand_from(acc_y_g);

        g->p_time.data[i] = genrand_from(time_g);
        g->u_fac.data[i] = genrand_from(update_speed_g);
    }

    g->max_ix = g->n_particles - 1;
}

void
dealloc_group(ParticleGroup *g)
{
    farr_free(&g->p_pos);
    farr_free(&g->p_vel);
    farr_free(&g->p_acc);
    farr_free(&g->p_time);
    farr_free(&g->u_fac);
    PyMem_Free(g->p_img_ix);

    for (Py_ssize_t i = 0; i < g->n_img_sequences; i++) {
        for (Py_ssize_t j = 0; j < g->n_img_frames[i]; j++)
            Py_DECREF(g->images[i][j]);
        PyMem_Free(g->images[i]);
    }

    PyMem_Free(g->images);
    PyMem_Free(g->n_img_frames);
}
