#include "extra_cmd.h"
#include "parser.h"

extern "C" int cmd_si(char* args) {
    auto [num] = parse<int>(args);
    printf("%d\n", num.value_or(-1));
    return 0;
}