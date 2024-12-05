#include "include/emitter.h"

/* ==================| Public facing EmitterObject functions |================== */
PyObject *
emitter_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    EmitterObject *self = (EmitterObject *)type->tp_alloc(type, 0);
    if (self == NULL) {
        return NULL;
    }

    memset(&self->emitter, 0, sizeof(Emitter));
    self->emitter.blend_mode = 1;

    return (PyObject *)self;
}

int
emitter_init(EmitterObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {
        "emit_shape", "emit_number", "animation",      "particle_lifetime",
        "speed_x",    "speed_y",     "acceleration_x", "acceleration_y",
        "blend_mode", NULL};

    PyObject *py_animation = NULL, *py_lifetime = NULL, *py_speed_x = NULL,
             *py_speed_y = NULL, *py_acceleration_x = NULL,
             *py_acceleration_y = NULL;
    Emitter *emitter = &self->emitter;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "iiOO|OOOOi", kwlist, &emitter->spawn_shape,
            &emitter->emission_number, &py_animation, &py_lifetime, &py_speed_x,
            &py_speed_y, &py_acceleration_x, &py_acceleration_y,
            &emitter->blend_mode)) {
        return -1;
    }

    if (!validate_emitter_shape(emitter->spawn_shape) ||
        !validate_blend_mode(emitter->blend_mode) ||
        !validate_animation(py_animation, emitter)) {
        return -1;
    }

    PyObject *py_options[] = {py_lifetime, py_speed_x, py_speed_y, py_acceleration_x,
                              py_acceleration_y};

    Option *options[] = {&emitter->lifetime, &emitter->speed_x, &emitter->speed_y,
                         &emitter->acceleration_x, &emitter->acceleration_y};

    if (!init_emitter_options(py_options, options, 5))
        return -1;

    return 0;
}

#define YN(x) (((x).state == OPTION_RANDOMIZED) ? "Yes" : "No")
#define CREATE_PYFLOAT(var, value)   \
    var = PyFloat_FromDouble(value); \
    if (PyErr_Occurred())            \
    goto on_error

PyObject *
emitter_str(EmitterObject *self)
{
    Emitter *e = &self->emitter;

    PyObject *lifetime_min = NULL;
    PyObject *lifetime_max = NULL;
    PyObject *speed_x_min = NULL;
    PyObject *speed_x_max = NULL;
    PyObject *speed_y_min = NULL;
    PyObject *speed_y_max = NULL;
    PyObject *acceleration_x_min = NULL;
    PyObject *acceleration_x_max = NULL;
    PyObject *acceleration_y_min = NULL;
    PyObject *acceleration_y_max = NULL;

    Option lifetime = e->lifetime;
    Option speed_x = e->speed_x;
    Option speed_y = e->speed_y;
    Option acceleration_x = e->acceleration_x;
    Option acceleration_y = e->acceleration_y;

    CREATE_PYFLOAT(lifetime_min, lifetime.min);
    CREATE_PYFLOAT(lifetime_max, lifetime.max);
    CREATE_PYFLOAT(speed_x_min, speed_x.min);
    CREATE_PYFLOAT(speed_x_max, speed_x.max);
    CREATE_PYFLOAT(speed_y_min, speed_y.min);
    CREATE_PYFLOAT(speed_y_max, speed_y.max);
    CREATE_PYFLOAT(acceleration_x_min, acceleration_x.min);
    CREATE_PYFLOAT(acceleration_x_max, acceleration_x.max);
    CREATE_PYFLOAT(acceleration_y_min, acceleration_y.min);
    CREATE_PYFLOAT(acceleration_y_max, acceleration_y.max);

    char *spawn_shape_str;
    switch (e->spawn_shape) {
        case _POINT:
            spawn_shape_str = "POINT";
            break;
        default:
            spawn_shape_str = "UNKNOWN";
            break;
    }

    PyObject *str = PyUnicode_FromFormat(
        "Emitter("
        "\n    spawn_shape:      %s"
        "\n    emission_number:  %d"
        "\n    animation:        %d images,"
        "\n    lifetime:         %R to %R   rng: %s"
        "\n    speed_x:          %R to %R   rng: %s"
        "\n    speed_y:          %R to %R   rng: %s"
        "\n    acceleration_x:   %R to %R   rng: %s"
        "\n    acceleration_y:   %R to %R   rng: %s"
        "\n)",
        spawn_shape_str, e->emission_number, e->num_frames, lifetime_min,
        lifetime_max, YN(lifetime), speed_x_min, speed_x_max, YN(speed_x),
        speed_y_min, speed_y_max, YN(speed_y), acceleration_x_min,
        acceleration_x_max, YN(acceleration_x), acceleration_y_min,
        acceleration_y_max, YN(acceleration_y));

    Py_DECREF(lifetime_min);
    Py_DECREF(lifetime_max);
    Py_DECREF(speed_x_min);
    Py_DECREF(speed_x_max);
    Py_DECREF(speed_y_min);
    Py_DECREF(speed_y_max);
    Py_DECREF(acceleration_x_min);
    Py_DECREF(acceleration_x_max);
    Py_DECREF(acceleration_y_min);
    Py_DECREF(acceleration_y_max);

    return str;

on_error:
    Py_XDECREF(lifetime_min);
    Py_XDECREF(lifetime_max);
    Py_XDECREF(speed_x_min);
    Py_XDECREF(speed_x_max);
    Py_XDECREF(speed_y_min);
    Py_XDECREF(speed_y_max);
    Py_XDECREF(acceleration_x_min);
    Py_XDECREF(acceleration_x_max);
    Py_XDECREF(acceleration_y_min);
    Py_XDECREF(acceleration_y_max);

    return NULL;
}

#undef CREATE_PYFLOAT
#undef YN

void
emitter_dealloc(EmitterObject *self)
{
    Py_XDECREF(self->emitter.animation);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* ====================| Internal EmitterObject functions |==================== */
int
validate_emitter_shape(int shape)
{
    if (shape == _POINT)
        return 1;

    PyErr_SetString(PyExc_ValueError, "Invalid emitter spawn area shape");
    return 0;
}

int
validate_blend_mode(int mode)
{
    if (mode == 0 || mode == 1)
        return 1;

    PyErr_SetString(PyExc_ValueError,
                    "Invalid blend mode, supported modes are: "
                    "pygame.BLEND_ADD and pygame.BLENDMODE_NONE");
    return 0;
}

int
validate_animation(PyObject *py_animation, Emitter *emitter)
{
    if (!PyTuple_Check(py_animation)) {
        PyErr_SetString(
            PyExc_TypeError,
            "Invalid animation argument, must be a tuple of pygame.Surface objects");
        return 0;
    }

    int len = PyTuple_GET_SIZE(py_animation);
    if (len == 0) {
        PyErr_SetString(PyExc_ValueError, "Animation tuple is empty");
        return 0;
    }

    Py_INCREF(py_animation);
    emitter->animation = py_animation;
    emitter->num_frames = len;

    PyObject **items = PySequence_Fast_ITEMS(py_animation);
    for (int i = 0; i < len; i++) {
        PyObject *img = items[i];
        if (!pgSurface_Check(img)) {
            PyErr_SetString(PyExc_TypeError,
                            "Invalid image in animation, must be a pygame.Surface");
            return 0;
        }

        pgSurfaceObject *surf_obj = (pgSurfaceObject *)img;
        if (!surf_obj->surf) {
            PyErr_SetString(PyExc_RuntimeError, "Surface is not initialized");
            return 0;
        }

        SDL_Surface *surf = surf_obj->surf;
        Uint8 alpha;

        if (surf->format->BytesPerPixel != 4 || SDL_HasColorKey(surf) ||
            SDL_HasSurfaceRLE(surf) || (surf->flags & SDL_RLEACCEL) ||
            SDL_ISPIXELFORMAT_ALPHA(surf->format->format) ||
            (SDL_GetSurfaceAlphaMod(surf, &alpha) == 0 && alpha != 255)) {
            PyErr_SetString(PyExc_ValueError,
                            "Image must be 32-bit, non-RLE, non-alpha-modulated");
            return 0;
        }
    }
    return 1;
}

int
init_emitter_options(PyObject *py_objs[], Option *options[], int count)
{
    for (int i = 0; i < count; i++)
        if (!init_option_from_pyobj(py_objs[i], options[i]))
            return 0;

    return 1;
}
