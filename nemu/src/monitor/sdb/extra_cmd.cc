#include "extra_cmd.h"
#include "parser.h"
#include <cpu/cpu.h>

extern "C" int cmd_si(char* args) {
    auto [num] = parse<int>(args);
    cpu_exec(num.value_or(1));
    return 0;
}