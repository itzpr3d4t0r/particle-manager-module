#pragma once

#include <stdint.h>
#include <Python.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#ifndef FORCEINLINE
#if defined(__clang__)
#define FORCEINLINE __inline__ __attribute__((__unused__))
#elif defined(__GNUC__)
#define FORCEINLINE __inline__
#elif defined(_MSC_VER)
#define FORCEINLINE __forceinline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define FORCEINLINE inline
#else
#define FORCEINLINE
#endif
#endif /* ~FORCEINLINE */

#ifndef SequenceFast_Check
#define SequenceFast_Check(op) (PyList_Check(op) || PyTuple_Check(op))
#endif

#ifndef RAISE
#define RAISE(Exc, msg) (PyErr_SetString(Exc, msg), NULL)
#endif

#ifndef IRAISE
#define IRAISE(Exc, msg) (PyErr_SetString(Exc, msg), 0)
#endif

#ifndef M_PI_180
#define M_PI_180 0.017453292519943295769236907684886
#endif

#ifndef M_180_PI
#define M_180_PI 57.295779513082320876798154814105
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_PI2
#define M_PI2 (M_PI * 2)
#endif

#define RADTODEG(x) ((x) * 57.295779513082320876798154814105)

static FORCEINLINE int
IntFromObj(PyObject *obj, int *val)
{
    if (PyFloat_Check(obj)) {
        /* Python3.8 complains with deprecation warnings if we pass
         * floats to PyLong_AsLong.
         */
        *val = (int)PyFloat_AS_DOUBLE(obj);
        return 1;
    }

    *val = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
    }

    return 1;
}

static FORCEINLINE int
IntFromObjIndex(PyObject *obj, int _index, int *val)
{
    int result = 0;
    PyObject *item = PySequence_GetItem(obj, _index);

    if (!item) {
        PyErr_Clear();
        return 0;
    }
    result = IntFromObj(item, val);
    Py_DECREF(item);
    return result;
}

static FORCEINLINE int
TwoIntsFromObj(PyObject *obj, int *val1, int *val2)
{
    if (!obj)
        return 0;
    if (PyTuple_Check(obj) && PyTuple_Size(obj) == 1) {
        return TwoIntsFromObj(PyTuple_GET_ITEM(obj, 0), val1, val2);
    }
    if (!PySequence_Check(obj) || PySequence_Length(obj) != 2) {
        return 0;
    }
    if (!IntFromObjIndex(obj, 0, val1) || !IntFromObjIndex(obj, 1, val2)) {
        return 0;
    }
    return 1;
}

static FORCEINLINE int
DoubleFromObj(PyObject *obj, double *val)
{
    if (PyFloat_Check(obj)) {
        *val = PyFloat_AS_DOUBLE(obj);
        return 1;
    }

    *val = (double)PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
    }

    return 1;
}

/*Assumes obj is a Sequence, internal or conscious use only*/
static FORCEINLINE int
_DoubleFromObjIndex(PyObject *obj, int index, double *val)
{
    int result = 0;

    PyObject *item = PySequence_ITEM(obj, index);
    if (!item) {
        PyErr_Clear();
        return 0;
    }
    result = DoubleFromObj(item, val);
    Py_DECREF(item);

    return result;
}

static FORCEINLINE int
DoubleFromObjIndex(PyObject *obj, int index, double *val)
{
    int result = 0;

    if ((PyTuple_Check(obj) || PyList_Check(obj)) &&
        index < PySequence_Fast_GET_SIZE(obj)) {
        result = DoubleFromObj(PySequence_Fast_GET_ITEM(obj, index), val);
    }
    else {
        PyObject *item = PySequence_GetItem(obj, index);

        if (!item) {
            PyErr_Clear();
            return 0;
        }
        result = DoubleFromObj(item, val);
        Py_DECREF(item);
    }

    return result;
}

static FORCEINLINE int
TwoDoublesFromObj(PyObject *obj, double *val1, double *val2)
{
    Py_ssize_t length;
    /*Faster path for tuples and lists*/
    if (PyTuple_Check(obj) || PyList_Check(obj)) {
        length = PySequence_Fast_GET_SIZE(obj);
        PyObject **f_arr = PySequence_Fast_ITEMS(obj);
        if (length == 2) {
            if (!DoubleFromObj(f_arr[0], val1) || !DoubleFromObj(f_arr[1], val2)) {
                return 0;
            }
        }
        else if (length == 1) {
            /* Handle case of ((x, y), ) 'nested sequence' */
            return TwoDoublesFromObj(f_arr[0], val1, val2);
        }
        else {
            return 0;
        }
    }
    else if (PySequence_Check(obj)) {
        length = PySequence_Length(obj);
        if (length == 2) {
            if (!_DoubleFromObjIndex(obj, 0, val1)) {
                return 0;
            }
            if (!_DoubleFromObjIndex(obj, 1, val2)) {
                return 0;
            }
        }
        else if (length == 1 && !PyUnicode_Check(obj)) {
            /* Handle case of ((x, y), ) 'nested sequence' */
            PyObject *tmp = PySequence_ITEM(obj, 0);
            int ret = TwoDoublesFromObj(tmp, val1, val2);
            Py_DECREF(tmp);
            return ret;
        }
        else {
            PyErr_Clear();
            return 0;
        }
    }
    else {
        return 0;
    }

    return 1;
}

static FORCEINLINE int
TwoDoublesFromTuple(PyObject *obj, double *val1, double *val2)
{
    Py_ssize_t length;

    length = PyTuple_GET_SIZE(obj);
    if (length != 2) {
        return 0;
    }

    if (!DoubleFromObj(PyTuple_GET_ITEM(obj, 0), val1) ||
        !DoubleFromObj(PyTuple_GET_ITEM(obj, 1), val2)) {
        return 0;
    }

    return 1;
}

static FORCEINLINE int
TwoDoublesFromFastcallArgs(PyObject *const *args, Py_ssize_t nargs, double *val1,
                           double *val2)
{
    if (nargs == 1 && TwoDoublesFromObj(args[0], val1, val2)) {
        return 1;
    }
    else if (nargs == 2 && DoubleFromObj(args[0], val1) &&
             DoubleFromObj(args[1], val2)) {
        return 1;
    }
    return 0;
}

static FORCEINLINE int
FloatFromObj(PyObject *obj, float *val)
{
    if (PyFloat_Check(obj)) {
        *val = (float)PyFloat_AS_DOUBLE(obj);
        return 1;
    }

    *val = (float)PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
    }

    return 1;
}

/*Assumes obj is a Sequence, internal or conscious use only*/
static FORCEINLINE int
_FloatFromObjIndex(PyObject *obj, int index, float *val)
{
    int result = 0;

    PyObject *item = PySequence_ITEM(obj, index);
    if (!item) {
        PyErr_Clear();
        return 0;
    }
    result = FloatFromObj(item, val);
    Py_DECREF(item);

    return result;
}

static FORCEINLINE int
FloatFromObjIndex(PyObject *obj, int index, float *val)
{
    int result = 0;

    if ((PyTuple_Check(obj) || PyList_Check(obj)) &&
        index < PySequence_Fast_GET_SIZE(obj)) {
        result = FloatFromObj(PySequence_Fast_GET_ITEM(obj, index), val);
    }
    else {
        PyObject *item = PySequence_GetItem(obj, index);

        if (!item) {
            PyErr_Clear();
            return 0;
        }
        result = FloatFromObj(item, val);
        Py_DECREF(item);
    }

    return result;
}

static FORCEINLINE int
TwoFloatsFromObj(PyObject *obj, float *val1, float *val2)
{
    Py_ssize_t length;
    /*Faster path for tuples and lists*/
    if (PyTuple_Check(obj) || PyList_Check(obj)) {
        length = PySequence_Fast_GET_SIZE(obj);
        PyObject **f_arr = PySequence_Fast_ITEMS(obj);
        if (length == 2) {
            if (!FloatFromObj(f_arr[0], val1) || !FloatFromObj(f_arr[1], val2)) {
                return 0;
            }
        }
        else if (length == 1) {
            /* Handle case of ((x, y), ) 'nested sequence' */
            return TwoFloatsFromObj(f_arr[0], val1, val2);
        }
        else {
            return 0;
        }
    }
    else if (PySequence_Check(obj)) {
        length = PySequence_Length(obj);
        if (length == 2) {
            if (!_FloatFromObjIndex(obj, 0, val1)) {
                return 0;
            }
            if (!_FloatFromObjIndex(obj, 1, val2)) {
                return 0;
            }
        }
        else if (length == 1 && !PyUnicode_Check(obj)) {
            /* Handle case of ((x, y), ) 'nested sequence' */
            PyObject *tmp = PySequence_ITEM(obj, 0);
            int ret = TwoFloatsFromObj(tmp, val1, val2);
            Py_DECREF(tmp);
            return ret;
        }
        else {
            PyErr_Clear();
            return 0;
        }
    }
    else {
        return 0;
    }

    return 1;
}

// these return PyObject * on success and NULL on failure.

static FORCEINLINE PyObject *
TupleFromDoublePair(double val1, double val2)
{
    /*this is demonstrated to be faster than Py_BuildValue*/
    PyObject *tuple = PyTuple_New(2);
    if (!tuple)
        return NULL;

    PyObject *tmp = PyFloat_FromDouble(val1);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 0, tmp);

    tmp = PyFloat_FromDouble(val2);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 1, tmp);

    return tuple;
}

static FORCEINLINE PyObject *
TupleTripleFromDoublePair(double val1, double val2, double val3)
{
    PyObject *tuple = PyTuple_New(3);
    if (!tuple)
        return NULL;

    PyObject *tmp = PyFloat_FromDouble(val1);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 0, tmp);

    tmp = PyFloat_FromDouble(val2);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 1, tmp);

    tmp = PyFloat_FromDouble(val3);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 2, tmp);

    return tuple;
}

static FORCEINLINE PyObject *
TupleFromIntPair(int val1, int val2)
{
    /*this is demonstrated to be faster than Py_BuildValue*/
    PyObject *tuple = PyTuple_New(2);
    if (!tuple)
        return NULL;

    PyObject *tmp = PyLong_FromLong(val1);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 0, tmp);

    tmp = PyLong_FromLong(val2);
    if (!tmp) {
        Py_DECREF(tuple);
        return NULL;
    }
    PyTuple_SET_ITEM(tuple, 1, tmp);

    return tuple;
}

static FORCEINLINE PyObject *
PointList_FromArrayDouble(double *array, int arr_length)
{
    /*Takes an even length double array [1, 2, 3, 4, 5, 6, 7, 8] and returns
     * a list of points:
     * C_arr[1, 2, 3, 4, 5, 6, 7, 8] -> List((1, 2), (3, 4), (5, 6), (7, 8))*/

    if (arr_length % 2) {
        return RAISE(PyExc_ValueError, "array length must be even");
    }

    int num_points = arr_length / 2;
    PyObject *sequence = PyList_New(num_points);
    if (!sequence) {
        return NULL;
    }

    int i;
    PyObject *point = NULL;
    for (i = 0; i < num_points; i++) {
        point = TupleFromDoublePair(array[i * 2], array[i * 2 + 1]);
        if (!point) {
            Py_DECREF(sequence);
            return NULL;
        }
        PyList_SET_ITEM(sequence, i, point);
    }

    return sequence;
}

static FORCEINLINE PyObject *
PointTuple_FromArrayDouble(double *array, int arr_length)
{
    /*Takes an even length double array [1, 2, 3, 4, 5, 6, 7, 8] and returns
     * a tuple of points:
     * C_arr[1, 2, 3, 4, 5, 6, 7, 8] -> Tuple((1, 2), (3, 4), (5, 6), (7, 8))*/

    if (arr_length % 2) {
        return RAISE(PyExc_ValueError, "array length must be even");
    }

    int num_points = arr_length / 2;
    PyObject *sequence = PyTuple_New(num_points);
    if (!sequence) {
        return NULL;
    }

    int i;
    PyObject *point = NULL;
    for (i = 0; i < num_points; i++) {
        point = TupleFromDoublePair(array[i * 2], array[i * 2 + 1]);
        if (!point) {
            Py_DECREF(sequence);
            return NULL;
        }
        PyTuple_SET_ITEM(sequence, i, point);
    }

    return sequence;
}

static FORCEINLINE int
TupleToIntPair(PyObject *obj, int *val1, int *val2)
{
    if (!IntFromObj(PyTuple_GET_ITEM(obj, 0), val1) ||
        !IntFromObj(PyTuple_GET_ITEM(obj, 1), val2)) {
        return 0;
    }

    return 1;
}

static FORCEINLINE int
TwoFloatsAndBoolFromTuple(PyObject *tup, float *a, float *b, int *c)
{
    Py_ssize_t size;
    if (!PyTuple_Check(tup) || (size = PyTuple_GET_SIZE(tup)) < 1)
        return 0;

    if (!FloatFromObj(PyTuple_GET_ITEM(tup, 0), a))
        return 0;

    if (size >= 2 && !FloatFromObj(PyTuple_GET_ITEM(tup, 1), b))
        return 0;

    if (size == 3 && (*c = PyObject_IsTrue(PyTuple_GET_ITEM(tup, 2))) == -1) {
        PyErr_Clear();
        return 0;
    }

    return 1;
}