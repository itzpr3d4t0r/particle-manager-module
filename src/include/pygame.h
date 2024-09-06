#ifndef _PYGAME_H
#define _PYGAME_H

#include "base.h"
#include "SDL_surface.h"

#define MODINIT_DEFINE(mod_name) PyMODINIT_FUNC PyInit_##mod_name(void)

#define RELATIVE_MODULE(m) ("." m)

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
#define encapsulate_api(ptr, module) \
    PyCapsule_New(ptr, PG_CAPSULE_NAME(module), NULL)

// this is not meant to be used in prod
// so we can directly include the base source
#define import_pygame_base()

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

static void **_PGSLOTS_surface = NULL;

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

#define SURF_INIT_CHECK(surf)                                               \
    {                                                                       \
        if (!surf) {                                                        \
            return RAISE(PyExc_RuntimeError, "Surface is not initialized"); \
        }                                                                   \
    }

#endif /* ~_PYGAME_H */