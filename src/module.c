#include "include/particle_manager.h"

static PyObject *
pm_rand_point_in_circle(PyObject *_null, PyObject *const *args, Py_ssize_t nargs)
{
    float x, y, r;
    float r_x, r_y;

    if (nargs != 3)
        return RAISE(PyExc_TypeError, "Invalid number of arguments");

    if (!FloatFromObj(args[0], &x) || !FloatFromObj(args[1], &y) ||
        !FloatFromObj(args[2], &r))
        return RAISE(PyExc_TypeError, "Invalid arguments, expected 3 floats");

    const float r_sqr = r * r;

    do {
        r_x = r * (random() * 2 - 1);
        r_y = r * (random() * 2 - 1);
    } while (r_x * r_x + r_y * r_y > r_sqr);

    return TupleFromDoublePair(x + r_x, y + r_y);
}

/* ===================================================================== */

static PyMethodDef _module_methods[] = {
    {"rand_point_in_circle", (PyCFunction)pm_rand_point_in_circle, METH_FASTCALL,
     NULL},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef PM_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "itz_particle_manager",
    .m_doc = "ItzPr4d4t0r's Particle Manager module",
    .m_size = -1,
    .m_methods = _module_methods,
};

PyMODINIT_FUNC
PyInit_itz_particle_manager(void)
{
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
