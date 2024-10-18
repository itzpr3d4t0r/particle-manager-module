#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
    float *data;
    Py_ssize_t capacity;
} float_array;

int
farr_init(float_array *v, Py_ssize_t capacity);

void
farr_free(float_array *v);
