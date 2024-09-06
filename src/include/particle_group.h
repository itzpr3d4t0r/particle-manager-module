#pragma once

#include "base.h"
#include "f_vec.h"
#include "MT19937.h"
#include "pygame.h"

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
           Py_ssize_t n_img_sequences);

void
setup_particles_general(ParticleGroup *g, const generator *pos_x_g,
                        const generator *pos_y_g, const generator *vel_x_g,
                        const generator *vel_y_g, const generator *acc_x_g,
                        const generator *acc_y_g, const generator *time_g,
                        const generator *update_speed_g);
void
setup_particles_point(ParticleGroup *g, float x, float y, const generator *vel_x_g,
                      const generator *vel_y_g, const generator *acc_x_g,
                      const generator *acc_y_g, const generator *time_g,
                      const generator *update_speed_g);
void
_update_particles_scalar(ParticleGroup *g, float dt);

void
update_group(ParticleGroup *g, float dt);

PyObject *
pythonify_group(ParticleGroup *g);

void
dealloc_group(ParticleGroup *g);