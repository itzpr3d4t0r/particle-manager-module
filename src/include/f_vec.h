#pragma once

#include "Python.h"

#define DEFINE_ARR(T, name)                                       \
    typedef struct {                                              \
        T *data;                                                  \
        Py_ssize_t capacity;                                      \
    } name##_arr;                                                 \
                                                                  \
    static int name##arr_init(name##_arr *v, Py_ssize_t capacity) \
    {                                                             \
        v->capacity = capacity;                                   \
        v->data = PyMem_New(T, capacity);                         \
        if (!v->data)                                             \
            return 0;                                             \
        return 1;                                                 \
    }                                                             \
                                                                  \
    static void name##arr_free(name##_arr *v)                     \
    {                                                             \
        PyMem_Free(v->data);                                      \
        v->capacity = 0;                                          \
    }

DEFINE_ARR(float, f)
