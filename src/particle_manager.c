#include "include/particle_manager.h"
#include "include/pygame.h"

PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    ParticleManager *self = (ParticleManager *) type->tp_alloc(type, 0);
    if (self) {
        self->instances = PyMem_Calloc(PM_BASE_BLOCK_SIZE, sizeof(EffectInstance));
        if (!self->instances) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }

        self->allocated_instances = PM_BASE_BLOCK_SIZE;
        self->used_instances = 0;
    }

    return (PyObject *) self;
}

void
pm_dealloc(ParticleManager *self) {
    for (Py_ssize_t i = 0; i < self->used_instances; i++)
        dealloc_effect_instance(&self->instances[i]);

    PyMem_Free(self->instances);

    Py_TYPE(self)->tp_free((PyObject *) self);
}

/* =======================| INTERNAL FUNCTIONALITY |======================= */

int
_pm_spawn_effect_helper(EffectInstance *instance, PyObject *const *args,
                        Py_ssize_t nargs) {
    if (!ParticleEffect_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError, "Invalid ParticleEffect object");
        return 0;
    }
    ParticleEffectObject *effect = (ParticleEffectObject *) args[0];

    vec2 pos;
    if (!TwoFloatsFromObj(args[1], &pos.x, &pos.y)) {
        PyErr_SetString(PyExc_TypeError, "Invalid position argument");
        return 0;
    }

    return init_effect_instance(instance, &effect->effect, pos);
}

Py_ssize_t
_pm_get_num_particles(ParticleManager *self) {
    Py_ssize_t num_particles = 0;
    for (Py_ssize_t i = 0; i < self->used_instances; i++)
        for (Py_ssize_t j = 0; j < self->instances[i].blocks_count; j++)
            num_particles += self->instances[i].p_data[j].particles_count;

    return num_particles;
}

/* ======================================================================== */

PyObject *
pm_spawn_effect(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 2) {
        PyErr_Format(PyExc_TypeError,
                     "pm_spawn_effect() requires 2 arguments, %zd given", nargs);
        return NULL;
    }

    if (self->used_instances + 1 > self->allocated_instances) {
        self->allocated_instances *= 2;
        self->instances = PyMem_Resize(self->instances, EffectInstance, self->allocated_instances);
        if (!self->instances)
            return PyErr_NoMemory();
    }

    EffectInstance *e_block = &self->instances[self->used_instances];

    if (!_pm_spawn_effect_helper(e_block, args, nargs))
        return NULL;

    self->used_instances++;

    Py_RETURN_NONE;
}

PyObject *
pm_update(ParticleManager *self, PyObject *arg) {
    float dt;
    if (!FloatFromObj(arg, &dt))
        return RAISE(PyExc_TypeError, "Invalid dt parameter, must be numeric");

    for (Py_ssize_t i = 0; i < self->used_instances; i++) {
        EffectInstance *effect = &self->instances[i];
        update_effect_instance(effect, dt);
        if (effect->ended) {
            dealloc_effect_instance(effect);
            memmove(effect, effect + 1,
                    sizeof(EffectInstance) * (self->used_instances - i - 1));
            self->used_instances--;
            i--;
        }
    }

    Py_RETURN_NONE;
}

PyObject *
pm_draw(ParticleManager *self, PyObject *arg) {
    if (!pgSurface_Check(arg))
        return RAISE(PyExc_TypeError, "Invalid surface object");

    pgSurfaceObject *dest = (pgSurfaceObject *) arg;
    SURF_INIT_CHECK((&dest->surf));

    for (Py_ssize_t i = 0; i < self->used_instances; i++)
        if (!draw_effect_instance(&self->instances[i], dest))
            return NULL;

    Py_RETURN_NONE;
}

PyObject *
pm_str(ParticleManager *self) {
    return PyUnicode_FromFormat(
            "ParticleManager(effects_playing: %lld, total particles: %lld)",
            self->used_instances, _pm_get_num_particles(self));
}

PyObject *
pm_get_num_particles(ParticleManager *self, void *closure) {
    return PyLong_FromSsize_t(_pm_get_num_particles(self));
}

/* ===================================================================== */
