#include "include/particle_manager.h"
#include <math.h>

/* =======================| INTERNAL FUNCTIONALITY |======================= */

static int
_pm_g_add_point(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    float x, y;
    float vx_min = 0, vx_max = 0;
    float vy_min = 0, vy_max = 0;
    float gx = 0, gy = 0;
    int rand_x = 0, rand_y = 0;
    int n_particles;
    Py_ssize_t imgs_list_size;

    if (!IntFromObj(args[0], &n_particles) || n_particles <= 0)
        return IRAISE(PyExc_TypeError, "Invalid number of particles");

    if (!TwoFloatsFromObj(args[1], &x, &y))
        return IRAISE(PyExc_TypeError, "Invalid type for paramenter: pos");

    if (!PyList_Check(args[2]) || (imgs_list_size = PyList_GET_SIZE(args[2])) == 0)
        return IRAISE(PyExc_TypeError, "Invalid images list");

    switch (nargs) {
        case 6:
            if (!TwoFloatsFromObj(args[5], &gx, &gy))
                return IRAISE(PyExc_TypeError,
                              "Invalid gravity. Must be a tuple of 2 floats");
        case 5:
            if (!TwoFloatsAndBoolFromTuple(args[4], &vy_min, &vy_max, &rand_y))
                return IRAISE(
                    PyExc_TypeError,
                    "Invalid vy settings, must be a tuple of 2 floats and a bool");
        case 4:
            if (!TwoFloatsAndBoolFromTuple(args[3], &vx_min, &vx_max, &rand_x))
                return IRAISE(
                    PyExc_TypeError,
                    "Invalid vx settings, must be a tuple of 2 floats and a bool");
            break;
    }

    group->blend_flag = 0;
    group->grav_x = gx;
    group->grav_y = gy;
    group->n_images = imgs_list_size;
    group->n_particles = n_particles;

    if (!(group->particles = PyMem_New(Particle, n_particles)))
        return IRAISE(PyExc_MemoryError, "Could not allocate memory for particles");

    Particle *const particles = group->particles;

    for (Py_ssize_t k = 0; k < group->n_particles; k++) {
        Particle *const p = &particles[k];
        p->x = x;
        p->y = y;
        p->vx = rand_x ? rand_between(vx_min, vx_max) : vx_min;
        p->vy = rand_y ? rand_between(vy_min, vy_max) : vy_min;
        p->energy = group->n_images - 1;
    }

    if (!(group->images = PyMem_New(PyObject *, group->n_images)))
        return IRAISE(PyExc_MemoryError, "Could not allocate memory for images");

    PyObject **list_items = PySequence_Fast_ITEMS(args[2]);
    for (Py_ssize_t k = 0; k < group->n_images; k++) {
        Py_INCREF(list_items[k]);
        group->images[k] = list_items[k];
    }

    return 1;
}

static int
_pm_add_group(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    int kind;
    if (!IntFromObj(args[0], &kind)) {
        PyErr_SetString(PyExc_TypeError, "Invalid spawn_type type");
        return 0;
    }
    nargs--;

    switch (kind) {
        case SPAWN_POINT:
            if (nargs < 3 || nargs > 6) {
                PyErr_SetString(PyExc_TypeError,
                                "SPAWN_POINT spawn_type requires between 3 "
                                "and 6 arguments.");
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

static int
_pm_remove_group(ParticleManager *self, int index)
{
    if (index < 0 || index >= self->g_used)
        return 0;

    ParticleGroup *group = &self->groups[index];
    dealloc_group(group);

    for (int i = index; i < self->g_used - 1; i++)
        self->groups[i] = self->groups[i + 1];

    self->g_used--;
    return 1;
}

/* ======================================================================== */

static PyObject *
pm_str(ParticleManager *self)
{
    PyObject *groups_list = get_group_str_list(self->groups, self->g_used);
    if (!groups_list)
        return NULL;

    PyObject *str = PyUnicode_FromFormat(
        "ParticleManager(%d,\n"
        "%R\n)",
        self->g_used, groups_list);

    Py_DECREF(groups_list);

    return str;
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
        self->groups = PyMem_Resize(self->groups, ParticleGroup, self->g_alloc);
        if (!self->groups)
            return PyErr_NoMemory();
    }

    ParticleGroup *group = &self->groups[self->g_used++];

    if (!_pm_add_group(group, args, nargs))
        return NULL;

    Py_RETURN_NONE;
}

static PyObject *
pm_remove_group(ParticleManager *self, PyObject *arg)
{
    int index;
    if (!IntFromObj(arg, &index))
        return RAISE(PyExc_TypeError, "Invalid index parameter, must be numeric");

    if (!_pm_remove_group(self, index)) {
        PyErr_Format(
            PyExc_IndexError,
            "Invalid index. Expected an index between 0 and %zd but got: %zd",
            self->g_used - 1, index);
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *
pm_update(ParticleManager *self, PyObject *arg)
{
    float dt;
    if (!FloatFromObj(arg, &dt))
        return RAISE(PyExc_TypeError, "Invalid dt parameter, must be nmumeric");

    for (Py_ssize_t j = 0; j < self->g_used; j++) {
        ParticleGroup *group = &self->groups[j];
        for (Py_ssize_t i = 0; i < group->n_particles; i++) {
            Particle *p = &group->particles[i];
            particle_move(p, dt);
            if (group->grav_x)
                p->x += group->grav_x * dt;
            if (group->grav_y)
                p->y += group->grav_y * dt;
            p->energy = MAX(0.0f, p->energy - dt);
        }
    }

    Py_RETURN_NONE;
}

static PyObject *
pm_get_num_particles(ParticleManager *self, void *closure)
{
    Py_ssize_t n = 0;

    for (Py_ssize_t i = 0; i < self->g_used; i++)
        n += self->groups[i].n_particles;

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

static PyObject *
pm_get_num_groups(ParticleManager *self, void *closure)
{
    return PyLong_FromSsize_t(self->g_used);
}

/* ===================================================================== */

static PyMethodDef PM_methods[] = {
    {"update", (PyCFunction)pm_update, METH_O, NULL},
    {"add_group", (PyCFunction)pm_add_group, METH_FASTCALL, NULL},
    {"remove_group", (PyCFunction)pm_remove_group, METH_O, NULL},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef PM_attributes[] = {
    {"num_particles", (getter)pm_get_num_particles, NULL, NULL, NULL},
    {"groups", (getter)pm_get_groups, NULL, NULL, NULL},
    {"num_groups", (getter)pm_get_num_groups, NULL, NULL, NULL},
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
    PyModule_AddObject(module, "ParticleManager", (PyObject *)&ParticleManagerType);

    if (PyModule_AddIntConstant(module, "SPAWN_POINT", SPAWN_POINT) == -1)
        return NULL;

    init_genrand(time(NULL));

    return module;
}