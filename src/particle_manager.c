#include "include/particle_manager.h"
#include "include/pygame.h"

PyObject *
pm_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ParticleManager *self = (ParticleManager *)type->tp_alloc(type, 0);
    if (self) {
        self->groups = PyMem_New(ParticleGroup, PM_BASE_GROUP_SIZE);
        if (!self->groups) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }

        memset(self->groups, 0, sizeof(ParticleGroup) * PM_BASE_GROUP_SIZE);
        self->g_alloc = PM_BASE_GROUP_SIZE;
        self->g_used = 0;
    }

    return (PyObject *)self;
}

void
pm_dealloc(ParticleManager *self)
{
    for (Py_ssize_t i = 0; i < self->g_used; i++)
        dealloc_group(&self->groups[i]);

    PyMem_Free(self->groups);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* =======================| INTERNAL FUNCTIONALITY |======================= */

int
_pm_g_add_point(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    int n_particles;
    float x, y;
    Py_ssize_t imgs_list_size;

    generator vel_x_g = {0};
    generator vel_y_g = {0};
    generator acc_x_g = {0};
    generator acc_y_g = {0};
    generator uspeed_g = {1.0, 0.0, 0};
    generator time_g = {0};

    vec2 gravity = {0};

    if (!IntFromObj(args[0], &n_particles) || n_particles <= 0)
        return IRAISE(PyExc_TypeError, "Invalid number of particles");

    if (!TwoFloatsFromObj(args[1], &x, &y))
        return IRAISE(PyExc_TypeError, "Invalid type for paramenter: pos");

    if (!PyList_Check(args[2]) || (imgs_list_size = PyList_GET_SIZE(args[2])) == 0)
        return IRAISE(PyExc_TypeError, "Invalid images list");

    switch (nargs) {
        case 10:
            if (!TwoFloatsFromObj(args[9], &gravity.x, &gravity.y))
                return IRAISE(PyExc_TypeError,
                              "Invalid gravity. Must be a tuple of 2 floats");
        case 9:
            if (!TwoFloatsAndBoolFromTuple(args[8], &time_g.min, &time_g.max,
                                           &time_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid time settings, must be a tuple of 2 floats "
                              "and a bool");
        case 8:
            if (!TwoFloatsAndBoolFromTuple(args[7], &uspeed_g.min, &uspeed_g.max,
                                           &uspeed_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid update_speed settings, must be a tuple of 2 "
                              "floats and a bool");
        case 7:
            if (!TwoFloatsAndBoolFromTuple(args[6], &acc_y_g.min, &acc_y_g.max,
                                           &acc_y_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid acc_y settings, must be a tuple of 2 floats "
                              "and a bool");
        case 6:
            if (!TwoFloatsAndBoolFromTuple(args[5], &acc_x_g.min, &acc_x_g.max,
                                           &acc_x_g.randomize))
                return IRAISE(PyExc_TypeError,
                              "Invalid acc_x settings, must be a tuple of 2 floats "
                              "and a bool");
        case 5:
            if (!TwoFloatsAndBoolFromTuple(args[4], &vel_y_g.min, &vel_y_g.max,
                                           &vel_y_g.randomize))
                return IRAISE(
                    PyExc_TypeError,
                    "Invalid vy settings, must be a tuple of 2 floats and a bool");
        case 4:
            if (!TwoFloatsAndBoolFromTuple(args[3], &vel_x_g.min, &vel_x_g.max,
                                           &vel_x_g.randomize))
                return IRAISE(
                    PyExc_TypeError,
                    "Invalid vx settings, must be a tuple of 2 floats and a bool");
    }

    PyObject **list_items = PySequence_Fast_ITEMS(args[2]);
    if (!init_group(group, n_particles, list_items, imgs_list_size))
        return 0;

    setup_particles_point(group, x, y, &vel_x_g, &vel_y_g, &acc_x_g, &acc_y_g,
                          &time_g, &uspeed_g);

    group->gravity = gravity;

    return 1;
}

int
_pm_add_group(ParticleGroup *group, PyObject *const *args, Py_ssize_t nargs)
{
    if (!IntFromObj(args[0], &group->blend_flag)) {
        PyErr_SetString(PyExc_TypeError, "Invalid blend_flag type");
        return 0;
    }

    int kind;
    if (!IntFromObj(args[1], &kind)) {
        PyErr_SetString(PyExc_TypeError, "Invalid spawn_type type");
        return 0;
    }

    nargs -= 2;

    switch (kind) {
        case SPAWN_POINT:
            if (nargs < 3 || nargs > 10) {
                PyErr_SetString(PyExc_TypeError,
                                "SPAWN_POINT spawn_type requires between 3 "
                                "and 10 arguments");
                return 0;
            }
            return _pm_g_add_point(group, args + 2, nargs);
        default:
            PyErr_SetString(PyExc_NotImplementedError,
                            "The supplied spawn_type doesn't exist.");
            return 0;
    }

    return 1;
}

/* ======================================================================== */

PyObject *
pm_add_group(ParticleManager *self, PyObject *const *args, Py_ssize_t nargs)
{
    if (nargs <= 2) {
        PyErr_SetString(PyExc_TypeError, "Invalid number of arguments");
        return NULL;
    }

    if (self->g_used + 1 > self->g_alloc) {
        self->g_alloc *= 2;
        self->groups = PyMem_Resize(self->groups, ParticleGroup, self->g_alloc);
        if (!self->groups)
            return PyErr_NoMemory();
    }

    ParticleGroup *group = &self->groups[self->g_used];

    if (!_pm_add_group(group, args, nargs))
        return NULL;

    self->g_used++;

    Py_RETURN_NONE;
}

PyObject *
pm_update(ParticleManager *self, PyObject *arg)
{
    float dt;
    if (!FloatFromObj(arg, &dt))
        return RAISE(PyExc_TypeError, "Invalid dt parameter, must be nmumeric");

    Py_ssize_t i;
    for (i = 0; i < self->g_used; i++)
        update_group(&self->groups[i], dt);

    Py_RETURN_NONE;
}

PyObject *
pm_draw(ParticleManager *self, PyObject *arg)
{
    if (!pgSurface_Check(arg))
        return RAISE(PyExc_TypeError, "Invalid surface object");

    pgSurfaceObject *src;
    pgSurfaceObject *dest = (pgSurfaceObject *)arg;
    SURF_INIT_CHECK((&dest->surf));

    Py_ssize_t i, j;

    for (i = 0; i < self->g_used; i++) {
        ParticleGroup *g = &self->groups[i];
        for (j = 0; j < g->n_particles; j++) {
            const Py_ssize_t img_ix = (Py_ssize_t)g->p_time.data[j];
            if (img_ix > g->n_img_frames[0] - 1) {
                continue;
            }

            const SDL_Surface *img =
                pgSurface_AsSurface((g->images[g->p_img_ix[j]][img_ix]));

            SDL_Rect rect = {(int)g->p_pos.data[j * 2],
                             (int)g->p_pos.data[j * 2 + 1], img->w, img->h};

            src = (pgSurfaceObject *)(g->images[g->p_img_ix[j]][img_ix]);
            SURF_INIT_CHECK((&src->surf));

            if (pgSurface_Blit(dest, src, &rect, NULL, g->blend_flag))
                return NULL;
        }
    }

    Py_RETURN_NONE;
}

PyObject *
pm_str(ParticleManager *self)
{
    Py_ssize_t n = 0, i;
    for (i = 0; i < self->g_used; i++)
        n += self->groups[i].n_particles;

    return PyUnicode_FromFormat("ParticleManager(groups: %d, tot_particles: %d)",
                                self->g_used, n);
}

PyObject *
pm_get_num_particles(ParticleManager *self, void *closure)
{
    Py_ssize_t n = 0, i;
    for (i = 0; i < self->g_used; i++)
        n += self->groups[i].n_particles;

    return PyLong_FromSsize_t(n);
}

PyObject *
pm_get_num_groups(ParticleManager *self, void *closure)
{
    return PyLong_FromSsize_t(self->g_used);
}

PyObject *
pm_get_groups(ParticleManager *self, void *closure)
{
    PyObject *list = PyList_New(self->g_used);
    if (!list)
        return PyErr_NoMemory();

    for (Py_ssize_t i = 0; i < self->g_used; i++) {
        PyObject *group = pythonify_group(&self->groups[i]);
        if (!group) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, group);
    }

    return list;
}

/* ===================================================================== */
