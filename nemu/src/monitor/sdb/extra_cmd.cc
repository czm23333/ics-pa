#include "extra_cmd.h"
#include "parser.h"

extern "C" int cmd_si(char* args) {
    parse<int>(std::string_view(args));
    return 0;
}