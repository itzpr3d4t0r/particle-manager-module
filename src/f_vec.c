#include "include/f_vec.h"

int
farr_init(f_arr *v, Py_ssize_t capacity)
{
    v->capacity = capacity;
    v->data = PyMem_New(float, capacity);
    if (!v->data)
        return 0;
    return 1;
}

void
farr_free(f_arr *v)
{
    PyMem_Free(v->data);
    v->capacity = 0;
}
