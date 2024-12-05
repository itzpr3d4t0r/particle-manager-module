#pragma once

#include <stdbool.h>
#include "base.h"
#include "option.h"

typedef enum {
    _POINT,
} EmitterSpawnShape;

typedef struct {
    /* Emitter type data */
    EmitterSpawnShape spawn_shape;

    /* Core emitter settings */
    int emission_number;

    /* Core particle settings */
    PyObject *animation; /* python tuple containing animation frames */
    int num_frames;

    Option lifetime;
    Option speed_x;
    Option speed_y;
    Option acceleration_x;
    Option acceleration_y;

    /* Additional particle settings */
    int blend_mode;
} Emitter;

typedef struct {
    PyObject_HEAD Emitter emitter;
} EmitterObject;

extern PyTypeObject Emitter_Type;

#define Emitter_Check(o) (Py_TYPE(o) == &Emitter_Type)

/* ==================| Public facing EmitterObject functions |================== */

PyObject *
emitter_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

int
emitter_init(EmitterObject *self, PyObject *args, PyObject *kwds);

PyObject *
emitter_str(EmitterObject *self);

void
emitter_dealloc(EmitterObject *self);

/* ====================| Internal EmitterObject functions |==================== */

int
validate_emitter_shape(int shape);

int
validate_blend_mode(int mode);

int
validate_animation(PyObject *py_animation, Emitter *emitter);

int
init_emitter_options(PyObject *py_objs[], Option *options[], int count);