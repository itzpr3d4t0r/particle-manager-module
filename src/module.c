#include "include/particle_manager.h"
#include "include/pygame.h"
#include "include/emitter.h"
#include "include/particle_effect.h"

void **_PGSLOTS_surface;
/* internal data for the random number generator */
uint32_t mt[N];
int mti = N + 1;

/* ===================================================================== */
PyTypeObject Emitter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "Emitter",
    .tp_doc = "Particle Emitter Object",
    .tp_basicsize = sizeof(EmitterObject),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)emitter_new,
    .tp_init = (initproc)emitter_init,
    .tp_dealloc = (destructor)emitter_dealloc,
    .tp_str = (reprfunc)emitter_str,
    .tp_repr = (reprfunc)emitter_str,
};

PyTypeObject ParticleEffect_Type = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "ParticleEffect",
    .tp_doc = "ParticleEffect Object",
    .tp_basicsize = sizeof(ParticleEffectObject),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)particle_effect_init,
    .tp_dealloc = (destructor)particle_effect_dealloc,
    .tp_str = (reprfunc)particle_effect_str,
};

static PyMethodDef ParticleManagerMethods[] = {
    {"spawn_effect", (PyCFunction)pm_spawn_effect, METH_FASTCALL, NULL},
    {"update", (PyCFunction)pm_update, METH_O, NULL},
    {"draw", (PyCFunction)pm_draw, METH_O, NULL},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef ParticleManagerAttributes[] = {
    {"num_particles", (getter)pm_get_num_particles, NULL, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};

static PyTypeObject ParticleManagerType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "particle_manager.ParticleManager",
    .tp_doc = "Particle Manager",
    .tp_basicsize = sizeof(ParticleManager),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)pm_new,
    .tp_str = (reprfunc)pm_str,
    .tp_repr = (reprfunc)pm_str,
    .tp_dealloc = (destructor)pm_dealloc,
    .tp_methods = ParticleManagerMethods,
    .tp_getset = ParticleManagerAttributes,
};

static struct PyModuleDef itz_particle_manager_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "itz_particle_manager",
    .m_doc = "ItzPr4d4t0r's Particle Manager module",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_itz_particle_manager(void)
{
    import_pygame_surface();

    PyObject *module = PyModule_Create(&itz_particle_manager_module);
    if (!module)
        return NULL;

    if (PyType_Ready(&ParticleManagerType) < 0)
        return NULL;

    Py_INCREF(&ParticleManagerType);
    PyModule_AddObject(module, "ParticleManager", (PyObject *)&ParticleManagerType);

    if (PyType_Ready(&Emitter_Type) < 0)
        return NULL;

    Py_INCREF(&Emitter_Type);
    PyModule_AddObject(module, "Emitter", (PyObject *)&Emitter_Type);

    if (PyType_Ready(&ParticleEffect_Type) < 0)
        return NULL;

    Py_INCREF(&ParticleEffect_Type);
    PyModule_AddObject(module, "ParticleEffect", (PyObject *)&ParticleEffect_Type);

    if (PyModule_AddIntConstant(module, "EMIT_POINT", POINT) == -1)
        return NULL;

    init_genrand((uint32_t)time(NULL));

    return module;
}
