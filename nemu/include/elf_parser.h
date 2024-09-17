#pragma once
#include <common.h>
#include <externc.h>
#ifdef __cplusplus
std::string get_function_name(vaddr_t addr);
#endif

EXTERNC void load_elf(char* filePath);