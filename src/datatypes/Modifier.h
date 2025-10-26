#pragma once

#include "ColorSpace.h"

enum class Operation {
    INVALID = 0,
    ADD,
    INVERT,
    SET
};

typedef struct s_Modifier {
    ColorSpace colorSpace = ColorSpace::UNSET;
    Operation operation = Operation::INVALID;

    char editChannel = 0;
    float difference = 0;
    char modifierChannel = 0;
} Modifier;
