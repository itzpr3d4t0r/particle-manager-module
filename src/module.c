#include "include/particle_manager.h"
#include "include/pygame.h"

void **_PGSLOTS_surface;

/* ===================================================================== */

static PyMethodDef PM_methods[] = {
    {"update", (PyCFunction)pm_update, METH_O, NULL},
    {"draw", (PyCFunction)pm_draw, METH_O, NULL},
    {"add_group", (PyCFunction)pm_add_group, METH_FASTCALL, NULL},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef PM_attributes[] = {
    {"num_particles", (getter)pm_get_num_particles, NULL, NULL, NULL},
    {"num_groups", (getter)pm_get_num_groups, NULL, NULL, NULL},
    {"groups", (getter)pm_get_groups, NULL, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}};

static PyTypeObject ParticleManagerType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "particle_manager.ParticleManager",
    .tp_doc = "Particle Manager",
    .tp_basicsize = sizeof(ParticleManager),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)pm_new,
    .tp_str = (reprfunc)pm_str,
    .tp_repr = (reprfunc)pm_str,
    .tp_dealloc = (destructor)pm_dealloc,
    .tp_methods = PM_methods,
    .tp_getset = PM_attributes,
};

static struct PyModuleDef PM_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "itz_particle_manager",
    .m_doc = "ItzPr4d4t0r's Particle Manager module",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_itz_particle_manager(void)
{
    import_pygame_surface();

    if (PyType_Ready(&ParticleManagerType) < 0)
        return NULL;

    PyObject *module = PyModule_Create(&PM_module);
    if (!module)
        return NULL;

    Py_INCREF(&ParticleManagerType);
    PyModule_AddObject(module, "ParticleManager", (PyObject *)&ParticleManagerType);

    if (PyModule_AddIntConstant(module, "SPAWN_POINT", SPAWN_POINT) == -1)
        return NULL;

    init_genrand((uint32_t)time(NULL));

    return module;
}
