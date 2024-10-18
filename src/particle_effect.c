#include "include/particle_effect.h"

int
particle_effect_init(ParticleEffectObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"emitters", NULL};
    PyObject *emitters = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &emitters))
        return -1;

    if (!PyList_Check(emitters)) {
        PyErr_SetString(PyExc_TypeError, "Invalid emitters, must be a list");
        return -1;
    }

    PyObject **items = PySequence_Fast_ITEMS(emitters);
    Py_ssize_t size = PySequence_Fast_GET_SIZE(emitters);

    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject *item = items[i];
        if (!Emitter_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "Invalid emitter, must be an Emitter instance");
            return -1;
        }
    }

    self->effect.e_alloc = size;
    self->effect.e_used = size;
    self->effect.done = false;

    return 0;
}

PyObject *
particle_effect_str(ParticleEffectObject *self)
{
    return PyUnicode_FromFormat("ParticleEffect(emitters=%d)", self->effect.e_used);
}