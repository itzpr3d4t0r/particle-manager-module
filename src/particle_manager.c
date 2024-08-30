#include "include/particle_manager.h"

/* =======================| INTERNAL FUNCTIONALITY |======================= */

static int
_pm_g_add_point(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    int n_particles;
    double x, y;
    double radius = 0;
    double vx_min = 0, vx_max = 0, vy_min = 0, vy_max = 0;
    int rand_x = 0, rand_y = 0;

    double gx = 0, gy = 0;

    if (!IntFromObj(args[0], &n_particles) || n_particles <= 0)
        return IRAISE(PyExc_TypeError, "Invalid number of particles");
    group->n_size = n_particles;

    if (!TwoDoublesFromObj(args[1], &x, &y))
        return IRAISE(PyExc_TypeError, "Invalid type for paramenter: pos");

    if (!PyList_Check(args[2]))
        return IRAISE(PyExc_TypeError, "images must be a list");

    switch (nargs - 3) {
        case 5:
            if (!DoubleFromObj(args[7], &gy))
                return IRAISE(PyExc_TypeError, "Invalid gravity_y");
        case 4:
            if (!DoubleFromObj(args[6], &gx))
                return IRAISE(PyExc_TypeError, "Invalid gravity_x");
        case 3:
            if (!TwoDoublesAndBoolFromTuple(args[5], &vy_min, &vy_max,
                                            &rand_y))
                return IRAISE(PyExc_TypeError, "Invalid vy");
        case 2:
            if (!TwoDoublesAndBoolFromTuple(args[4], &vx_min, &vx_max,
                                            &rand_x))
                return IRAISE(PyExc_TypeError, "Invalid vx");
        case 1:
            if (!DoubleFromObj(args[3], &radius))
                return IRAISE(PyExc_TypeError,
                              "Invalid type for paramenter: radius");
            if (radius < 0)
                return IRAISE(PyExc_ValueError, "radius can't be negative");
            break;
    }

    group->particles = PyMem_New(Particle, group->n_size);
    if (!group->particles)
        return IRAISE(PyExc_MemoryError, "Memory Error");
    for (Py_ssize_t k = 0; k < group->n_size; k++) {
        group->particles[k].x = (float)x;
        group->particles[k].y = (float)y;
        group->particles[k].vx = (float)vx_min;
        group->particles[k].vy = (float)vy_min;
        group->particles[k].img_ix = 0;
    }

    PyObject **list_items = PySequence_Fast_ITEMS(args[2]);
    group->n_images = PyList_GET_SIZE(args[2]);
    if (group->n_images < 1)
        return IRAISE(PyExc_TypeError, "Images list can't be empty");
    group->images = PyMem_New(PyObject *, group->n_images);
    if (!group->images)
        return IRAISE(PyExc_MemoryError, "Memory Error");

    for (Py_ssize_t k = 0; k < group->n_images; k++) {
        Py_INCREF(list_items[k]);
        group->images[k] = list_items[k];
    }

    return 1;
}

static int
_pm_internal_add_group(ParticleGroup *group, PyObject *const *args,
                       Py_ssize_t nargs)
{
    int kind;
    if (!IntFromObj(args[0], &kind)) {
        PyErr_SetString(PyExc_TypeError, "Invalid spawn_type type");
        return 0;
    }
    nargs--;

    switch (kind) {
        case GroupKind_POINT:
            if (nargs < 3 || nargs > 8) {
                PyErr_SetString(PyExc_TypeError,
                                "GROUP_POINT spawn_type requires between 3 "
                                "and 8 arguments.");
                return 0;
            }
            return _pm_g_add_point(group, args + 1, nargs);
        default:
            PyErr_SetString(PyExc_NotImplementedError,
                            "The supplied spawn_type doesn't exist.");
            return 0;
    }

    return 1;
}

/* ======================================================================== */

static PyObject *
pm_str(ParticleManager *self)
{
    return PyUnicode_FromFormat("ParticleManager(occupied groups: %d/%d)",
                                self->g_used, self->g_alloc);
}

static PyObject *
pm_add_group(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs)
{
    if (nargs <= 1) {
        PyErr_SetString(PyExc_TypeError, "Invalid number of arguments");
        return NULL;
    }

    if (self->g_used + 1 > self->g_alloc) {
        self->g_alloc *= 2;
        self->groups =
            PyMem_Resize(self->groups, ParticleGroup, self->g_alloc);
        if (!self->groups)
            return PyErr_NoMemory();
    }

    ParticleGroup *group = &self->groups[self->g_used++];

    if (!_pm_internal_add_group(group, args, nargs))
        return NULL;

    Py_RETURN_NONE;
}

static PyObject *
pm_update(ParticleManager *self, PyObject *arg)
{
    double dt;
    if (!DoubleFromObj(arg, &dt))
        return RAISE(PyExc_TypeError,
                     "Invalid dt parameter, must be nmumeric");

    float dtf = (float)dt;

    for (Py_ssize_t j = 0; j < self->g_used; j++) {
        ParticleGroup *group = &self->groups[j];
        for (Py_ssize_t i = 0; i < group->n_size; i++)
            particle_move(&group->particles[i], dtf);
    }

    Py_RETURN_NONE;
}

static PyObject *
pm_get_num_particles(ParticleManager *self, void *closure)
{
    Py_ssize_t n = 0;

    for (Py_ssize_t i = 0; i < self->g_used; i++)
        n += self->groups[i].n_size;

    return PyLong_FromSsize_t(n);
}

static PyObject *
pm_get_groups(ParticleManager *self, void *closure)
{
    PyObject *groups = PyList_New(self->g_used);
    if (!groups)
        return NULL;

    for (Py_ssize_t i = 0; i < self->g_used; i++) {
        PyObject *g = pythonify_group(&self->groups[i]);
        if (!g) {
            Py_DECREF(groups);
            return NULL;
        }
        PyList_SET_ITEM(groups, i, g);
    }

    return groups;
}

/* ===================================================================== */

static PyMethodDef PM_methods[] = {
    {"update", (PyCFunction)pm_update, METH_O, NULL},
    {"add_group", (PyCFunction)pm_add_group, METH_FASTCALL, NULL},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef PM_attributes[] = {
    {"num_particles", (getter)pm_get_num_particles, NULL, NULL, NULL},
    {"groups", (getter)pm_get_groups, NULL, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}};

static PyTypeObject ParticleManagerType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name =
        "particle_manager.ParticleManager",
    .tp_doc = "Particle Manager",
    .tp_basicsize = sizeof(ParticleManager),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = (newfunc)pm_new,
    .tp_init = (initproc)pm_init,
    .tp_str = (reprfunc)pm_str,
    .tp_repr = (reprfunc)pm_str,
    .tp_dealloc = (destructor)pm_dealloc,
    .tp_methods = PM_methods,
    .tp_getset = PM_attributes,
};

static struct PyModuleDef PM_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "particle_manager",
    .m_doc = "Module that provides a fast Particle Manager",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_particle_manager(void)
{
    if (PyType_Ready(&ParticleManagerType) < 0)
        return NULL;

    PyObject *module = PyModule_Create(&PM_module);
    if (!module)
        return NULL;

    Py_INCREF(&ParticleManagerType);
    PyModule_AddObject(module, "ParticleManager",
                       (PyObject *)&ParticleManagerType);

    if (PyModule_AddIntConstant(module, "GROUP_POINT", GroupKind_POINT) == -1)
        return NULL;

    if (PyModule_AddIntConstant(module, "GROUP_RECT_AREA",
                                GroupKind_RECT_AREA) == -1)
        return NULL;

    return module;
}