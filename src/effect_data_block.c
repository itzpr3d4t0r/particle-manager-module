#include "include/effect_data_block.h"

int
init_effect_data_block(EffectDataBlock *block, ParticleEffect *effect, vec2 position)
{
    block->position = position;
    block->blocks_count = effect->emitters_count;
    block->ended = false;

    /* Allocate memory for the data blocks */
    block->p_data = PyMem_Calloc(effect->emitters_count, sizeof(DataBlock));
    if (!block->p_data) {
        PyErr_NoMemory();
        return 0;
    }

    /* Initialize each data block, one per emitter */
    for (Py_ssize_t i = 0; i < effect->emitters_count; i++) {
        Emitter *emitter = &effect->emitters[i];
        DataBlock *db = &block->p_data[i];
        if (!init_data_block(db, emitter, position))
            return 0;
    }

    return 1;
}

void
update_effect_data_block(EffectDataBlock *block, float dt)
{
    int active_blocks = 0;

    for (Py_ssize_t i = 0; i < block->blocks_count; i++) {
        if (block->p_data[i].ended)
            continue;

        update_data_block(&block->p_data[i], dt);

        if (!block->p_data[i].ended)
            active_blocks++;
    }

    if (active_blocks == 0)
        block->ended = true;
}

int
draw_effect_data_block(EffectDataBlock *block, pgSurfaceObject *dest)
{
    for (Py_ssize_t i = 0; i < block->blocks_count; i++)
        if (!draw_data_block(&block->p_data[i], dest, block->p_data[i].blend_mode))
            return 0;

    return 1;
}

void
dealloc_effect_data_block(EffectDataBlock *block)
{
    for (Py_ssize_t i = 0; i < block->blocks_count; i++)
        dealloc_data_block(&block->p_data[i]);

    PyMem_Free(block->p_data);
}
