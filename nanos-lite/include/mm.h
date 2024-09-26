#pragma once
#include <externc.h>

EXTERNC void map_range(AddrSpace* space, uintptr_t begin, uintptr_t end, uint8_t priv);