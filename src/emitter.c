#include "include/emitter.h"

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

static FORCEINLINE int
initGen_FromObj(PyObject *obj, generator *gen)
{
    /* Tries to extract either one or two floats from an object.
     * If the object is a tuple, it must have one or two floats.
     * If the object is a float, it is used as the minimum value. */

    if (PyFloat_Check(obj)) {
        gen->min = (float)PyFloat_AS_DOUBLE(obj);
        gen->in_use = true;

        if (PyErr_Occurred()) {
            PyErr_Clear();
            return 0;
        }

        return 1;
    }
    else if (PyLong_Check(obj)) {
        gen->min = (float)PyLong_AsDouble(obj);
        gen->in_use = true;

        if (PyErr_Occurred()) {
            PyErr_Clear();
            return 0;
        }

        return 1;
    }

    Py_ssize_t size;

    if (PyTuple_Check(obj)) {
        size = PyTuple_GET_SIZE(obj);
        if (size < 1 || size > 2)
            return 0;

        if (!FloatFromObj(PyTuple_GET_ITEM(obj, 0), &gen->min))
            return 0;

        if (size == 2) {
            if (!FloatFromObj(PyTuple_GET_ITEM(obj, 1), &gen->max))
                return 0;

            gen->randomize = 1;
        }
    }
    else if (PyList_Check(obj)) {
        size = PyList_GET_SIZE(obj);
        if (size < 1 || size > 2)
            return 0;

        if (!FloatFromObj(PyList_GET_ITEM(obj, 0), &gen->min))
            return 0;

        if (size == 2) {
            if (!FloatFromObj(PyList_GET_ITEM(obj, 1), &gen->max))
                return 0;
            gen->randomize = 1;
        }
    }
    else if (PySequence_Check(obj)) {
        size = PySequence_Length(obj);
        if (size < 1 || size > 2)
            return 0;

        if (!_FloatFromObjIndex(obj, 0, &gen->min))
            return 0;

        if (size == 2) {
            if (!_FloatFromObjIndex(obj, 1, &gen->max))
                return 0;
            gen->randomize = 1;
        }
    }
    else {
        return 0;
    }

    if (gen->min > gen->max) {
        float tmp = gen->min;
        gen->min = gen->max;
        gen->max = tmp;
    }

    gen->in_use = true;

    return 1;
}

int
emitter_init(EmitterObject *self, PyObject *args, PyObject *kwds)
{
    Emitter *emitter = &self->emitter;

    static char *kwlist[] = {"emit_shape",
                             "emit_number",
                             "looping",
                             "emit_interval",
                             "emit_time",
                             "animation",
                             "particle_lifetime",
                             "speed_x",
                             "speed_y",
                             "acceleration_x",
                             "acceleration_y",
                             "angle",
                             "align_speed_to_angle",
                             "align_acceleration_to_angle",
                             "blend_mode",
                             NULL};

    PyObject *animation = NULL;
    PyObject *lifetime_obj = NULL, *speedx_obj = NULL, *speedy_obj = NULL,
             *accx_obj = NULL, *accy_obj = NULL, *angle_obj = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "i|iiffOOOOOOOiii", kwlist, &emitter->spawn_shape,
            &emitter->emission_number, &emitter->looping,
            &emitter->emission_interval, &emitter->emission_time, &animation,
            &lifetime_obj, &speedx_obj, &speedy_obj, &accx_obj, &accy_obj,
            &angle_obj, &emitter->align_speed_to_angle,
            &emitter->align_acceleration_to_angle, &emitter->blend_mode)) {
        return -1;
    }

    switch (emitter->spawn_shape) {
        case _POINT:
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Invalid emitter spawn area shape");
            return -1;
    }

    switch (emitter->blend_mode) {
        case 0:
        case 1:
            break;
        default:
            PyErr_SetString(PyExc_ValueError,
                            "Invalid blend mode, supported blend modes are:"
                            " pygame.BLEND_ADD and pygame.BLENDMODE_NONE");
            return -1;
    }

    if (PyTuple_Check(animation)) {
        int len = PyTuple_GET_SIZE(animation);
        if (len == 0) {
            PyErr_SetString(PyExc_ValueError, "animation tuple is empty");
            return -1;
        }

        emitter->num_frames = len;

        Py_INCREF(animation);
        emitter->animation = animation;

        PyObject **items = PySequence_Fast_ITEMS(animation);
        for (int i = 0; i < len; i++) {
            PyObject *img = items[i];
            if (!pgSurface_Check(img)) {
                PyErr_SetString(
                    PyExc_TypeError,
                    "Invalid image in animation, must be a pygame.Surface");
                return -1;
            }

            SDL_Surface *surf = ((pgSurfaceObject *)img)->surf;
            Uint8 alpha;

            /* Rule out unsupported image formats and flags */
            if (surf->format->BytesPerPixel != 4 || SDL_HasColorKey(surf) ||
                SDL_HasSurfaceRLE(surf) || (surf->flags & SDL_RLEACCEL) ||
                SDL_ISPIXELFORMAT_ALPHA(surf->format->format) ||
                (SDL_GetSurfaceAlphaMod(surf, &alpha) == 0 && alpha != 255)) {
                PyErr_SetString(
                    PyExc_ValueError,
                    "Image must be 32-bit, non-RLE, non-alpha-modulated");
                return -1;
            }
        }
    }
    else {
        PyErr_SetString(
            PyExc_TypeError,
            "Invalid animation argument, must be a tuple of pygame.Surface objects");
        return -1;
    }

    if (lifetime_obj && !initGen_FromObj(lifetime_obj, &emitter->lifetime)) {
        PyErr_SetString(PyExc_TypeError, "Invalid lifetime argument");
        return -1;
    }
    else {
        emitter->lifetime.min = 60.0f;
    }

    if (speedx_obj && !initGen_FromObj(speedx_obj, &emitter->speed_x)) {
        PyErr_SetString(PyExc_TypeError, "Invalid speed_x argument");
        return -1;
    }

    if (speedy_obj && !initGen_FromObj(speedy_obj, &emitter->speed_y)) {
        PyErr_SetString(PyExc_TypeError, "Invalid speed_y argument");
        return -1;
    }

    if (accx_obj && !initGen_FromObj(accx_obj, &emitter->acceleration_x)) {
        PyErr_SetString(PyExc_TypeError, "Invalid acceleration_x argument");
        return -1;
    }

    if (accy_obj && !initGen_FromObj(accy_obj, &emitter->acceleration_y)) {
        PyErr_SetString(PyExc_TypeError, "Invalid acceleration_y argument");
        return -1;
    }

    if (angle_obj && !initGen_FromObj(angle_obj, &emitter->angle)) {
        PyErr_SetString(PyExc_TypeError, "Invalid angle argument");
        return -1;
    }

    if (!emitter->angle.in_use) {
        if (emitter->align_speed_to_angle) {
            PyErr_SetString(
                PyExc_ValueError,
                "align_speed_to_angle argument requires an angle to be set");
            return -1;
        }
        else if (emitter->align_acceleration_to_angle) {
            PyErr_SetString(
                PyExc_ValueError,
                "align_acceleration_to_angle argument requires an angle to be set");
            return -1;
        }
    }

    return 0;
}

PyObject *
emitter_str(EmitterObject *self)
{
    Emitter *e = &self->emitter;

    PyObject *py_emission_interval = NULL, *py_emission_time = NULL,
             *lifetime_min = NULL, *lifetime_max = NULL, *speed_x_min = NULL,
             *speed_x_max = NULL, *speed_y_min = NULL, *speed_y_max = NULL,
             *acceleration_x_min = NULL, *acceleration_x_max = NULL,
             *acceleration_y_min = NULL, *acceleration_y_max = NULL,
             *angle_min = NULL, *angle_max = NULL;

    py_emission_interval = PyFloat_FromDouble(e->emission_interval);
    if (PyErr_Occurred())
        return NULL;

    py_emission_time = PyFloat_FromDouble(e->emission_time);
    if (PyErr_Occurred())
        goto on_error;

    lifetime_min = PyFloat_FromDouble(e->lifetime.min);
    if (PyErr_Occurred())
        goto on_error;

    lifetime_max = PyFloat_FromDouble(e->lifetime.max);
    if (PyErr_Occurred())
        goto on_error;

    speed_x_min = PyFloat_FromDouble(e->speed_x.min);
    if (PyErr_Occurred())
        goto on_error;

    speed_x_max = PyFloat_FromDouble(e->speed_x.max);
    if (PyErr_Occurred())
        goto on_error;

    speed_y_min = PyFloat_FromDouble(e->speed_y.min);
    if (PyErr_Occurred())
        goto on_error;

    speed_y_max = PyFloat_FromDouble(e->speed_y.max);
    if (PyErr_Occurred())
        goto on_error;

    acceleration_x_min = PyFloat_FromDouble(e->acceleration_x.min);
    if (PyErr_Occurred())
        goto on_error;

    acceleration_x_max = PyFloat_FromDouble(e->acceleration_x.max);
    if (PyErr_Occurred())
        goto on_error;

    acceleration_y_min = PyFloat_FromDouble(e->acceleration_y.min);
    if (PyErr_Occurred())
        goto on_error;

    acceleration_y_max = PyFloat_FromDouble(e->acceleration_y.max);
    if (PyErr_Occurred())
        goto on_error;

    angle_min = PyFloat_FromDouble(e->angle.min);
    if (PyErr_Occurred())
        goto on_error;

    angle_max = PyFloat_FromDouble(e->angle.max);
    if (PyErr_Occurred())
        goto on_error;

    char *spawn_shape_str;
    switch (e->spawn_shape) {
        case _POINT:
            spawn_shape_str = "POINT";
            break;
        default:
            spawn_shape_str = "UNKNOWN";
            break;
    }

    generator *angle = &e->angle;
    generator *lifetime = &e->lifetime;
    generator *speed_x = &e->speed_x;
    generator *speed_y = &e->speed_y;
    generator *accel_x = &e->acceleration_x;
    generator *accel_y = &e->acceleration_y;

    PyObject *str = PyUnicode_FromFormat(
        "Emitter("
        "\n    spawn_shape=%s,"
        "\n    emission_number=%d,"
        "\n    looping=%s,"
        "\n    emission_interval=%R,"
        "\n    emission_time=%R,"
        "\n    animation=(%d images),"
        "\n    lifetime=(%R, %R, random=%s),"
        "\n    speed_x=(%R, %R, random=%s),"
        "\n    speed_y=(%R, %R, random=%s),"
        "\n    acceleration_x=(%R, %R, random=%s),"
        "\n    acceleration_y=(%R, %R, random=%s),"
        "\n    angle=(%R, %R, random=%s),"
        "\n    align_speed_to_angle=%s,"
        "\n    align_acceleration_to_angle=%s"
        "\n)",
        spawn_shape_str, e->emission_number, e->looping ? "True" : "False",
        py_emission_interval, py_emission_time, e->num_frames, lifetime_min,
        lifetime_max, lifetime->randomize ? "True" : "False", speed_x_min,
        speed_x_max, speed_x->randomize ? "True" : "False", speed_y_min, speed_y_max,
        speed_y->randomize ? "True" : "False", acceleration_x_min,
        acceleration_x_max, accel_x->randomize ? "True" : "False",
        acceleration_y_min, acceleration_y_max,
        accel_y->randomize ? "True" : "False", angle_min, angle_max,
        angle->randomize ? "True" : "False",
        e->align_speed_to_angle ? "True" : "False",
        e->align_acceleration_to_angle ? "True" : "False");

    Py_DECREF(py_emission_interval);
    Py_DECREF(py_emission_time);
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
    Py_DECREF(angle_min);
    Py_DECREF(angle_max);

    return str;

on_error:
    Py_XDECREF(py_emission_interval);
    Py_XDECREF(py_emission_time);
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
    Py_XDECREF(angle_min);
    Py_XDECREF(angle_max);

    return NULL;
}

void
emitter_dealloc(EmitterObject *self)
{
    Py_XDECREF(self->emitter.animation);

    Py_TYPE(self)->tp_free((PyObject *)self);
}
