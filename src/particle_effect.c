#include "include/particle_effect.h"

int
particle_effect_init(ParticleEffectObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"emitters", NULL};
    PyObject *emitters = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &emitters))
        return -1;

    if (!emitters || !PyTuple_Check(emitters)) {
        PyErr_SetString(PyExc_TypeError, "Invalid emitters, must be a tuple");
        return -1;
    }

    Py_INCREF(emitters);
    self->effect.emitters = emitters;
    Py_ssize_t size = PyTuple_GET_SIZE(emitters);
    if (size == 0) {
        PyErr_SetString(PyExc_ValueError, "emitters tuple is empty");
        return -1;
    }

    self->effect.emitters_count = size;

    PyObject **items = PySequence_Fast_ITEMS(emitters);

    for (Py_ssize_t i = 0; i < size; i++) {
        if (!Emitter_Check(items[i])) {
            PyErr_SetString(PyExc_TypeError,
                            "Invalid emitter, must be an Emitter instance");
            return -1;
        }
    }

    return 0;
}

void
particle_effect_dealloc(ParticleEffectObject *self)
{
    Py_XDECREF(self->effect.emitters);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *
particle_effect_str(ParticleEffectObject *self)
{
    return PyUnicode_FromFormat("ParticleEffect(emitters=%d)",
                                self->effect.emitters_count);
}
