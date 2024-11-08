#include "include/particle_manager.h"
#include "include/pygame.h"

PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ParticleManager *self = (ParticleManager *)type->tp_alloc(type, 0);
    if (self) {
        self->blocks = PyMem_Calloc(PM_BASE_BLOCK_SIZE, sizeof(EffectDataBlock));
        if (!self->blocks) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }

        self->allocated_blocks = PM_BASE_BLOCK_SIZE;
        self->used_blocks = 0;
    }

    return (PyObject *)self;
}

void
pm_dealloc(ParticleManager *self)
{
    for (Py_ssize_t i = 0; i < self->used_blocks; i++)
        dealloc_effect_data_block(&self->blocks[i]);

    PyMem_Free(self->blocks);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* =======================| INTERNAL FUNCTIONALITY |======================= */

int
_pm_spawn_effect_helper(EffectDataBlock *block, PyObject *const *args,
                        Py_ssize_t nargs)
{
    if (!ParticleEffect_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError, "Invalid ParticleEffect object");
        return 0;
    }
    ParticleEffectObject *effect = (ParticleEffectObject *)args[0];

    vec2 pos;
    if (!TwoFloatsFromObj(args[1], &pos.x, &pos.y)) {
        PyErr_SetString(PyExc_TypeError, "Invalid position argument");
        return 0;
    }

    return init_effect_data_block(block, &effect->effect, pos);
}

/* ======================================================================== */

PyObject *
pm_spawn_effect(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs)
{
    if (nargs != 2) {
        PyErr_Format(PyExc_TypeError,
                     "pm_spawn_effect() requires 2 arguments, %zd given", nargs);
        return NULL;
    }

    if (self->used_blocks + 1 > self->allocated_blocks) {
        self->allocated_blocks *= 2;
        self->blocks =
            PyMem_Resize(self->blocks, EffectDataBlock, self->allocated_blocks);
        if (!self->blocks)
            return PyErr_NoMemory();
    }

    EffectDataBlock *e_block = &self->blocks[self->used_blocks];

    if (!_pm_spawn_effect_helper(e_block, args, nargs))
        return NULL;

    self->used_blocks++;

    Py_RETURN_NONE;
}

PyObject *
pm_update(ParticleManager *self, PyObject *arg)
{
    float dt;
    if (!FloatFromObj(arg, &dt))
        return RAISE(PyExc_TypeError, "Invalid dt parameter, must be numeric");

    for (Py_ssize_t i = 0; i < self->used_blocks; i++) {
        EffectDataBlock *effect = &self->blocks[i];
        update_effect_data_block(effect, dt);
        if (effect->ended) {
            dealloc_effect_data_block(effect);
            memmove(effect, effect + 1,
                    sizeof(EffectDataBlock) * (self->used_blocks - i - 1));
            self->used_blocks--;
            i--;
        }
    }

    Py_RETURN_NONE;
}

PyObject *
pm_str(ParticleManager *self)
{
    return PyUnicode_FromFormat("ParticleManager(blocks: -, tot_particles: -)");
}
/* ===================================================================== */
