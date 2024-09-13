#include "extra_cmd.h"
#include "parser.h"
#include <cpu/cpu.h>

#include "isa.h"

extern "C" int cmd_si(char *args) {
    auto [num] = parse<int>(args);
    cpu_exec(num.value_or(1));
    return 0;
}

extern "C" int cmd_info(char *args) {
    auto [type] = parse<std::string>(args);
    if (type == "r") {
        for (size_t i = 0; i < std::size(cpu.gpr); ++i) printf("Register %lu: %u", i, cpu.gpr[i]);
    } else if (type == "w") {
    } else printf("Unknown subcommand");
    return 0;
}
