#include "include/effect_instance.h"

int
init_effect_instance(EffectInstance *instance, ParticleEffect *effect, vec2 position) {
    instance->position = position;
    instance->blocks_count = effect->emitters_count;
    instance->ended = false;

    /* Allocate memory for the data blocks */
    instance->p_data = PyMem_Calloc(effect->emitters_count, sizeof(DataBlock));
    if (!instance->p_data) {
        PyErr_NoMemory();
        return 0;
    }

    PyObject **emitter_objs = PySequence_Fast_ITEMS(effect->emitters);

    /* Initialize each data block, one per emitter */
    for (Py_ssize_t i = 0; i < effect->emitters_count; i++) {
        Emitter *emitter = &((EmitterObject *) emitter_objs[i])->emitter;
        DataBlock *db = &instance->p_data[i];
        if (!init_data_block(db, emitter, position))
            return 0;
    }

    return 1;
}

void
update_effect_instance(EffectInstance *instance, float dt) {
    int active_blocks = 0;

    for (Py_ssize_t i = 0; i < instance->blocks_count; i++) {
        if (instance->p_data[i].ended)
            continue;

        update_data_block(&instance->p_data[i], dt);

        if (!instance->p_data[i].ended)
            active_blocks++;
    }

    if (active_blocks == 0)
        instance->ended = true;
}

int
draw_effect_instance(EffectInstance *instance, pgSurfaceObject *dest) {
    for (Py_ssize_t i = 0; i < instance->blocks_count; i++)
        if (!draw_data_block(&instance->p_data[i], dest, instance->p_data[i].blend_mode))
            return 0;

    return 1;
}

void
dealloc_effect_instance(EffectInstance *instance) {
    for (Py_ssize_t i = 0; i < instance->blocks_count; i++)
        dealloc_data_block(&instance->p_data[i]);

    PyMem_Free(instance->p_data);
}
