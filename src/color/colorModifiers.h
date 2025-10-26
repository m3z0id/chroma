#pragma once

#include <cstdint>

#include "../datatypes/Modifier.h"

void modifyRGB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, bool overflowAllowed);
void modifyHSL(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, bool overflowAllowed);
void modifyOKLAB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods, bool overflowAllowed);