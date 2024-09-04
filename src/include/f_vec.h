#pragma once
#include "base.h"

#define DEFINE_ARR_STRUCT(T, name) \
    typedef struct {               \
        T *data;                   \
        Py_ssize_t capacity;       \
    } name##_arr;

#define DEFINE_ARR_FUNCTIONS(T, name)                      \
    int name##arr_init(name##_arr *v, Py_ssize_t capacity) \
    {                                                      \
        v->capacity = capacity;                            \
        v->data = PyMem_New(T, capacity);                  \
        if (!v->data)                                      \
            return 0;                                      \
        return 1;                                          \
    }                                                      \
                                                           \
    void name##arr_free(name##_arr *v)                     \
    {                                                      \
        PyMem_Free(v->data);                               \
        v->capacity = 0;                                   \
    }

DEFINE_ARR_STRUCT(float, f)
DEFINE_ARR_FUNCTIONS(float, f)