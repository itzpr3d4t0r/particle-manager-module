#pragma once

#include <stdbool.h>
#include "MT19937.h"
#include "base.h"

typedef enum {
    POINT,
    CIRCLE,
    RECTANGLE,
} EmitterSpawnShape;

typedef struct {
    /* Emitter type data */
    EmitterSpawnShape spawn_shape;

    /* Core emitter settings */
    bool looping;            /* if the emitter should loop */
    float emission_interval; /* time between emissions */
    float emission_time;     /* time the emitter is active */
    float emission_counter;  /* counter used to determine when to emit */
    int emission_number;     /* number of particles to emit */

    /* Core particle settings */
    PyObject *animation;   /* particle animation tuple */
    int num_frames;        /* animation frames number */

    generator lifetime;
    generator speed_x;
    generator speed_y;
    generator acceleration_x;
    generator acceleration_y;
    generator angle;

    /* Additional particle settings */
    int blend_mode;
    bool angled;
    bool align_speed_to_angle;
    bool align_acceleration_to_angle;
} Emitter;

typedef struct {
    PyObject_HEAD Emitter emitter;
} EmitterObject;

extern PyTypeObject Emitter_Type;

#define Emitter_Check(o) (Py_TYPE(o) == &Emitter_Type)

PyObject *
emitter_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

int
emitter_init(EmitterObject *self, PyObject *args, PyObject *kwds);

PyObject *
emitter_str(EmitterObject *self);

void
emitter_dealloc(EmitterObject *self);
