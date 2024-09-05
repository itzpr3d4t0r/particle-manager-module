#pragma once

#include "Python.h"

typedef struct {
    float *data;
    Py_ssize_t capacity;
} f_arr;

int
farr_init(f_arr *v, Py_ssize_t capacity);

void
farr_free(f_arr *v);