#pragma once
#include <externc.h>
#include <common.h>

EXTERNC void onCall(vaddr_t pc, vaddr_t addr);
EXTERNC void onRet();
EXTERNC void printStackTrace();
