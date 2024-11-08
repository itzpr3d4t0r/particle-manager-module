#include "include/float_array.h"

int
float_array_alloc(float_array *v, Py_ssize_t capacity)
{
    /* allocate the array */
    v->capacity = capacity;
    v->data = PyMem_New(float, capacity);
    if (!v->data)
        return 0;
    return 1;
}

int
float_array_calloc(float_array *v, Py_ssize_t capacity)
{
    /* allocate and zero-initialize the array */
    v->capacity = capacity;
    v->data = PyMem_Calloc(capacity, sizeof(float));
    if (!v->data)
        return 0;
    return 1;
}

void
float_array_free(float_array *v)
{
    PyMem_Free(v->data);
    v->capacity = 0;
}
