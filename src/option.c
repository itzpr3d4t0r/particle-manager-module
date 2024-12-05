#include "include/option.h"

int
init_option_from_pyobj(PyObject *obj, Option *opt)
{
    if (!obj || obj == Py_None) {
        opt->state = OPTION_UNSET;
        return 1;
    }

    if (PyFloat_Check(obj)) {
        opt->value = (float)PyFloat_AS_DOUBLE(obj);
        opt->state = OPTION_VALUE;
        return 1;
    }
    else if (PyLong_Check(obj)) {
        opt->value = (float)PyLong_AsDouble(obj);
        opt->state = OPTION_VALUE;

        return PyErr_Occurred() ? 0 : 1;
    }

    Py_ssize_t size;

    if (PyTuple_Check(obj)) {
        size = PyTuple_GET_SIZE(obj);
        if (size != 2) {
            PyErr_SetString(PyExc_ValueError, "Tuple must have exactly 2 elements");
            return 0;
        }

        if (!FloatFromObj(PyTuple_GET_ITEM(obj, 0), &opt->min) ||
            !FloatFromObj(PyTuple_GET_ITEM(obj, 1), &opt->max)) {
            PyErr_SetString(PyExc_ValueError, "Invalid float value in tuple");
            return 0;
        }
    }
    else if (PyList_Check(obj)) {
        size = PyList_GET_SIZE(obj);
        if (size != 2) {
            PyErr_SetString(PyExc_ValueError, "List must have exactly 2 elements");
            return 0;
        }

        if (!FloatFromObj(PyList_GET_ITEM(obj, 0), &opt->min) ||
            !FloatFromObj(PyList_GET_ITEM(obj, 1), &opt->max)) {
            PyErr_SetString(PyExc_ValueError, "Invalid float value in list");
            return 0;
        }
    }
    else if (PySequence_Check(obj)) {
        size = PySequence_Length(obj);
        if (size != 2) {
            PyErr_SetString(PyExc_ValueError,
                            "Sequence must have exactly 2 elements");
            return 0;
        }

        if (!_FloatFromObjIndex(obj, 0, &opt->min) ||
            !_FloatFromObjIndex(obj, 1, &opt->max)) {
            PyErr_SetString(PyExc_ValueError, "Invalid float value in sequence");
            return 0;
        }
    }
    else {
        PyErr_SetString(PyExc_TypeError, "Invalid type for option");
        return 0;
    }

    if (opt->min > opt->max) {
        float tmp = opt->min;
        opt->min = opt->max;
        opt->max = tmp;
    }

    opt->state = OPTION_RANDOMIZED;

    return 1;
}
