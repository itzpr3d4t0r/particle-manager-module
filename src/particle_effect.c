#include "include/particle_effect.h"

int
particle_effect_init(ParticleEffectObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"emitters", NULL};
    PyObject *emitters = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &emitters))
        return -1;

    if (!SequenceFast_Check(emitters)) {
        PyErr_SetString(PyExc_TypeError,
                        "Invalid emitters, must be a list or tuple");
        return -1;
    }

    PyObject **items = PySequence_Fast_ITEMS(emitters);
    Py_ssize_t size = PySequence_Fast_GET_SIZE(emitters);

    self->effect.emitters = PyMem_New(Emitter, size);
    if (!self->effect.emitters) {
        PyErr_NoMemory();
        return -1;
    }

    Emitter *effect_emitters = self->effect.emitters;

    /* copy emitters over*/
    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject *item = items[i];
        if (!Emitter_Check(item)) {
            PyErr_SetString(PyExc_TypeError,
                            "Invalid emitter, must be an Emitter instance");
            return -1;
        }
        Emitter *emitter = &(((EmitterObject *)item)->emitter);

        effect_emitters[i] = *emitter;
        effect_emitters[i].animations = NULL;

        if (!emitter_allocate_and_copy_animations(emitter, &effect_emitters[i])) {
            return -1;
        }
    }

    self->effect.emitters_count = size;

    return 0;
}

void
particle_effect_dealloc(ParticleEffectObject *self)
{
    for (int i = 0; i < self->effect.emitters_count; i++)
        _emitter_free(&self->effect.emitters[i]);

    PyMem_Free(self->effect.emitters);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *
particle_effect_str(ParticleEffectObject *self)
{
    return PyUnicode_FromFormat("ParticleEffect(emitters=%d)",
                                self->effect.emitters_count);
}