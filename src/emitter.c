#include "include/emitter.h"

PyObject *
emitter_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    EmitterObject *self = (EmitterObject *)type->tp_alloc(type, 0);
    if (self == NULL) {
        return NULL;
    }

    memset(&self->emitter, 0, sizeof(Emitter));

    return (PyObject *)self;
}

int
emitter_init(EmitterObject *self, PyObject *args, PyObject *kwds)
{
    Emitter *emitter = &self->emitter;

    static char *kwlist[] = {"emit_type",
                             "emit_number",
                             "looping",
                             "emit_interval",
                             "emit_time",
                             "images",
                             "particle_lifetime",
                             "speed_x",
                             "speed_y",
                             "acceleration_x",
                             "acceleration_y",
                             "angle",
                             "align_speed_to_angle",
                             "align_acceleration_to_angle",
                             NULL};

    PyObject *images = NULL;
    PyObject *lifetime_obj = NULL, *speed_x_obj = NULL, *speed_y_obj = NULL,
             *acceleration_x_obj = NULL, *acceleration_y_obj = NULL,
             *angle_obj = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "i|iiffOOOOOOOii", kwlist, &emitter->type,
            &emitter->emission_number, &emitter->looping,
            &emitter->emission_interval, &emitter->emission_time, &images,
            &lifetime_obj, &speed_x_obj, &speed_y_obj, &acceleration_x_obj,
            &acceleration_y_obj, &angle_obj, &emitter->align_speed_to_angle,
            &emitter->align_acceleration_to_angle)) {
        return -1;
    }

    switch (emitter->type) {
        case POINT:
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Invalid emitter type");
            return -1;
    }

    if (PyList_Check(images)) {
        int len = PyList_GET_SIZE(images);
        if (len == 0) {
            PyErr_SetString(PyExc_ValueError, "images list is empty");
            return -1;
        }

        PyObject **list_items = PySequence_Fast_ITEMS(images);

        emitter->animations = PyMem_New(PyObject **, len);
        if (!emitter->animations) {
            PyErr_NoMemory();
            return -1;
        }

        memset(emitter->animations, 0, sizeof(PyObject **) * len);

        emitter->num_frames = PyMem_New(int, len);
        if (!emitter->num_frames) {
            PyErr_NoMemory();
            return -1;
        }

        emitter->animations_count = len;

        for (int i = 0; i < len; i++) {
            PyObject *item = list_items[i];
            if (!PyList_Check(item)) {
                PyErr_SetString(PyExc_TypeError,
                                "Invalid animation, must be a list of surfaces");
                return -1;
            }

            Py_ssize_t item_len = PyList_GET_SIZE(item);
            if (item_len == 0) {
                PyErr_SetString(PyExc_ValueError, "animation list is empty");
                return -1;
            }

            PyObject **item_list = PySequence_Fast_ITEMS(item);

            emitter->animations[i] = PyMem_New(PyObject *, item_len);
            if (!emitter->animations[i]) {
                PyErr_NoMemory();
                return -1;
            }

            memset(emitter->animations[i], 0, sizeof(PyObject *) * item_len);

            emitter->num_frames[i] = item_len;

            for (int j = 0; j < item_len; j++) {
                PyObject *img = item_list[j];
                if (!pgSurface_Check(img)) {
                    PyErr_SetString(PyExc_TypeError,
                                    "Invalid image in animation list");
                    return -1;
                }

                Py_INCREF(img);
                emitter->animations[i][j] = img;
            }
        }
    }
    else {
        PyErr_SetString(
            PyExc_TypeError,
            "Invalid images argument, must be a list of image sequences");
        return -1;
    }

    if (lifetime_obj) {
        generator *lifetime = &emitter->lifetime;
        if (!RandRange_FromTupleOrNum(lifetime_obj, &lifetime->min, &lifetime->max,
                                      &lifetime->randomize)) {
            PyErr_SetString(PyExc_TypeError, "Invalid lifetime argument");
            return -1;
        }
    }
    else {
        emitter->lifetime.min = 100.0f;
    }

    if (speed_x_obj) {
        generator *speed_x = &emitter->speed_x;
        if (!RandRange_FromTupleOrNum(speed_x_obj, &speed_x->min, &speed_x->max,
                                      &speed_x->randomize)) {
            PyErr_SetString(PyExc_TypeError, "Invalid speed_x argument");
            return -1;
        }
    }
    else {
        emitter->speed_x.min = 0.0f;
    }

    if (speed_y_obj) {
        generator *speed_y = &emitter->speed_y;
        if (!RandRange_FromTupleOrNum(speed_y_obj, &speed_y->min, &speed_y->max,
                                      &speed_y->randomize)) {
            PyErr_SetString(PyExc_TypeError, "Invalid speed_y argument");
            return -1;
        }
    }
    else {
        emitter->speed_y.min = 0.0f;
    }

    if (acceleration_x_obj) {
        generator *acceleration_x = &emitter->acceleration_x;
        if (!RandRange_FromTupleOrNum(acceleration_x_obj, &acceleration_x->min,
                                      &acceleration_x->max,
                                      &acceleration_x->randomize)) {
            PyErr_SetString(PyExc_TypeError, "Invalid acceleration_x argument");
            return -1;
        }
    }
    else {
        emitter->acceleration_x.min = 0.0f;
    }

    if (acceleration_y_obj) {
        generator *acceleration_y = &emitter->acceleration_y;
        if (!RandRange_FromTupleOrNum(acceleration_y_obj, &acceleration_y->min,
                                      &acceleration_y->max,
                                      &acceleration_y->randomize)) {
            PyErr_SetString(PyExc_TypeError, "Invalid acceleration_y argument");
            return -1;
        }
    }
    else {
        emitter->acceleration_y.min = 0.0f;
    }

    if (angle_obj) {
        generator *angle = &emitter->angle;
        if (!RandRange_FromTupleOrNum(angle_obj, &angle->min, &angle->max,
                                      &angle->randomize)) {
            PyErr_SetString(PyExc_TypeError, "Invalid angle argument");
            return -1;
        }
        emitter->angled = true;
    }
    else {
        emitter->angled = false;
    }

    if (!emitter->angled) {
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

    emitter->update_function(emitter);

    return 0;
}

PyObject *
emitter_str(EmitterObject *self)
{
    Emitter *e = &self->emitter;

    PyObject *py_emission_interval, *py_emission_time, *lifetime_min, *lifetime_max,
        *speed_x_min, *speed_x_max, *speed_y_min, *speed_y_max, *acceleration_x_min,
        *acceleration_x_max, *acceleration_y_min, *acceleration_y_max, *angle_min,
        *angle_max;

    py_emission_interval = PyFloat_FromDouble(e->emission_interval);
    if (PyErr_Occurred())
        return NULL;

    py_emission_time = PyFloat_FromDouble(e->emission_time);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        return NULL;
    }

    lifetime_min = PyFloat_FromDouble(e->lifetime.min);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        return NULL;
    }

    lifetime_max = PyFloat_FromDouble(e->lifetime.max);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        return NULL;
    }

    speed_x_min = PyFloat_FromDouble(e->speed_x.min);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        Py_DECREF(lifetime_max);
        return NULL;
    }

    speed_x_max = PyFloat_FromDouble(e->speed_x.max);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        Py_DECREF(lifetime_max);
        Py_DECREF(speed_x_min);
        return NULL;
    }

    speed_y_min = PyFloat_FromDouble(e->speed_y.min);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        Py_DECREF(lifetime_max);
        Py_DECREF(speed_x_min);
        Py_DECREF(speed_x_max);
        return NULL;
    }

    speed_y_max = PyFloat_FromDouble(e->speed_y.max);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        Py_DECREF(lifetime_max);
        Py_DECREF(speed_x_min);
        Py_DECREF(speed_x_max);
        Py_DECREF(speed_y_min);
        return NULL;
    }

    acceleration_x_min = PyFloat_FromDouble(e->acceleration_x.min);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        Py_DECREF(lifetime_max);
        Py_DECREF(speed_x_min);
        Py_DECREF(speed_x_max);
        Py_DECREF(speed_y_min);
        Py_DECREF(speed_y_max);
        return NULL;
    }

    acceleration_x_max = PyFloat_FromDouble(e->acceleration_x.max);
    if (PyErr_Occurred()) {
        Py_DECREF(py_emission_interval);
        Py_DECREF(py_emission_time);
        Py_DECREF(lifetime_min);
        Py_DECREF(lifetime_max);
        Py_DECREF(speed_x_min);
        Py_DECREF(speed_x_max);
        Py_DECREF(speed_y_min);
        Py_DECREF(speed_y_max);
        Py_DECREF(acceleration_x_min);
        return NULL;
    }

    acceleration_y_min = PyFloat_FromDouble(e->acceleration_y.min);
    if (PyErr_Occurred()) {
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
        return NULL;
    }

    acceleration_y_max = PyFloat_FromDouble(e->acceleration_y.max);
    if (PyErr_Occurred()) {
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
        return NULL;
    }

    angle_min = PyFloat_FromDouble(e->angle.min);
    if (PyErr_Occurred()) {
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
        return NULL;
    }

    angle_max = PyFloat_FromDouble(e->angle.max);
    if (PyErr_Occurred()) {
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
        return NULL;
    }

    char *type_str;
    switch (e->type) {
        case POINT:
            type_str = "POINT";
            break;
        default:
            type_str = "UNKNOWN";
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
        "\n    type=%s,"
        "\n    emission_number=%d,"
        "\n    looping=%s,"
        "\n    emission_interval=%R,"
        "\n    emission_time=%R,"
        "\n    images=(%d animations),"
        "\n    lifetime=(%R, %R, random=%s),"
        "\n    speed_x=(%R, %R, random=%s),"
        "\n    speed_y=(%R, %R, random=%s),"
        "\n    acceleration_x=(%R, %R, random=%s),"
        "\n    acceleration_y=(%R, %R, random=%s),"
        "\n    angle=(%R, %R, random=%s),"
        "\n    align_speed_to_angle=%s,"
        "\n    align_acceleration_to_angle=%s"
        "\n)",
        type_str,
        e->emission_number,
        e->looping ? "True" : "False",
        py_emission_interval,
        py_emission_time,
        e->animations_count,
        lifetime_min, lifetime_max, lifetime->randomize ? "True" : "False",
        speed_x_min, speed_x_max, speed_x->randomize ? "True" : "False", speed_y_min,
        speed_y_max, speed_y->randomize ? "True" : "False", acceleration_x_min,
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
}

void
emitter_dealloc(EmitterObject *self)
{
    if (self->emitter.animations) {
        for (int i = 0; i < self->emitter.animations_count; i++) {
            for (int j = 0; j < self->emitter.num_frames[i]; j++)
                Py_DECREF(self->emitter.animations[i][j]);
            PyMem_Free(self->emitter.animations[i]);
        }
        PyMem_Free(self->emitter.animations);
    }

    PyMem_Free(self->emitter.num_frames);

    Py_TYPE(self)->tp_free((PyObject *)self);
}