#pragma once
#include "base.h"
#include "MT19937.h"

typedef enum { OPTION_UNSET = 0, OPTION_VALUE, OPTION_RANDOMIZED } OptionState;

typedef struct {
    float value;
    float min;
    float max;
    OptionState state;
} Option;

#define OPTION_IS_SET(opt) ((opt).state != OPTION_UNSET)
#define OPTION_IS_RANDOMIZED(opt) ((opt).state == OPTION_RANDOMIZED)
#define OPTION_IS_VALUE(opt) ((opt).state == OPTION_VALUE)

float FORCEINLINE
option_get_value(const Option *opt)
{
    switch (opt->state) {
        case OPTION_UNSET:
            return 0.0f;
        case OPTION_VALUE:
            return opt->value;
        case OPTION_RANDOMIZED:
            return rand_between(opt->min, opt->max);
        default:
            return 0.0f;
    }
}

int
init_option_from_pyobj(PyObject *obj, Option *opt);