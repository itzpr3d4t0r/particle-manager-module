#pragma once

#include <stdbool.h>
#include "MT19937.h"
#include "base.h"

typedef enum {
    _POINT,
} EmitterSpawnShape;

typedef struct {
    /* Emitter type data */
    EmitterSpawnShape spawn_shape;

    /* Core emitter settings */
    int emission_number; /* number of particles to emit */

    /* Core particle settings */
    PyObject *animation; /* python tuple containing animation frames */
    int num_frames;      /* animation frames number */

    generator lifetime;
    generator speed_x;
    generator speed_y;
    generator acceleration_x;
    generator acceleration_y;

    /* Additional particle settings */
    int blend_mode;
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
