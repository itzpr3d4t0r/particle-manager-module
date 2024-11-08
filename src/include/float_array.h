#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
    float *data;
    Py_ssize_t capacity;
} float_array;

int
float_array_alloc(float_array *v, Py_ssize_t capacity);

int
float_array_calloc(float_array *v, Py_ssize_t capacity);

void
float_array_free(float_array *v);
