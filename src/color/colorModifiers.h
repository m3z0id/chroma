#pragma once

#include <array>
#include <cstdint>
#include "../datatypes/Modifier.h"

void modifyRGB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3> & mods);
void modifyHSL(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods);
void modifyOKLAB(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods);
void modifyOKLCh(uint8_t* r, uint8_t* g, uint8_t* b, const std::array<Modifier, 3>& mods);