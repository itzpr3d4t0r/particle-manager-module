#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "SDL.h"

#define MODPREFIX ""
#define IMPPREFIX "pygame."

#ifdef __SYMBIAN32__
#undef MODPREFIX
#undef IMPPREFIX
#define MODPREFIX "pygame_"
#define IMPPREFIX "pygame_"

#endif /* ~__SYMBIAN32__ */

#define PYGAMEAPI_LOCAL_ENTRY "_PYGAME_C_API"
#define PG_CAPSULE_NAME(m) (IMPPREFIX m "." PYGAMEAPI_LOCAL_ENTRY)

#define _LOAD_SLOTS_FROM_PYGAME(module)                                       \
    {                                                                         \
        PyObject *_mod_##module = PyImport_ImportModule(IMPPREFIX #module);   \
                                                                              \
        if (_mod_##module != NULL) {                                          \
            PyObject *_c_api =                                                \
                PyObject_GetAttrString(_mod_##module, PYGAMEAPI_LOCAL_ENTRY); \
                                                                              \
            Py_DECREF(_mod_##module);                                         \
            if (_c_api != NULL && PyCapsule_CheckExact(_c_api)) {             \
                void **localptr = (void **)PyCapsule_GetPointer(              \
                    _c_api, PG_CAPSULE_NAME(#module));                        \
                _PGSLOTS_##module = localptr;                                 \
            }                                                                 \
            Py_XDECREF(_c_api);                                               \
        }                                                                     \
    }

#define PYGAMEAPI_GET_SLOT(module, index) _PGSLOTS_##module[(index)]

extern void **_PGSLOTS_surface;

struct pgSubSurface_Data;

/**
 * \brief A pygame object that wraps an SDL_Surface. A `pygame.Surface`
 * instance.
 */
typedef struct {
    PyObject_HEAD struct SDL_Surface *surf;
    int owner;
    struct pgSubSurface_Data *subsurface;
    PyObject *weakreflist;
    PyObject *locklist;
    PyObject *dependency;
} pgSurfaceObject;

#define pgSurface_Type (*(PyTypeObject *)PYGAMEAPI_GET_SLOT(surface, 0))

#define pgSurface_Check(x) (PyObject_IsInstance((x), (PyObject *)&pgSurface_Type))
#define pgSurface_New2 \
    (*(pgSurfaceObject * (*)(SDL_Surface *, int)) PYGAMEAPI_GET_SLOT(surface, 1))

#define pgSurface_SetSurface \
    (*(int (*)(pgSurfaceObject *, SDL_Surface *, int))PYGAMEAPI_GET_SLOT(surface, 3))

#define pgSurface_Blit                                                       \
    (*(int (*)(pgSurfaceObject *, pgSurfaceObject *, SDL_Rect *, SDL_Rect *, \
               int))PYGAMEAPI_GET_SLOT(surface, 2))

#define import_pygame_surface() _LOAD_SLOTS_FROM_PYGAME(surface)

#define pgSurface_New(surface) pgSurface_New2((surface), 1)
#define pgSurface_NewNoOwn(surface) pgSurface_New2((surface), 0)
#define pgSurface_AsSurface(x) (((pgSurfaceObject *)(x))->surf)

#define SURF_INIT_CHECK(surf)                                              \
    if (!surf) {                                                           \
        PyErr_SetString(PyExc_RuntimeError, "Surface is not initialized"); \
        return NULL;                                                       \
    }
