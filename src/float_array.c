#include "include/float_array.h"

int
farr_init(float_array *v, Py_ssize_t capacity)
{
    v->capacity = capacity;
    v->data = PyMem_New(float, capacity);
    if (!v->data)
        return 0;
    return 1;
}

void
farr_free(float_array *v)
{
    PyMem_Free(v->data);
    v->capacity = 0;
}
