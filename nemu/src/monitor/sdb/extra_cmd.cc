#include "extra_cmd.h"
#include "parser.h"
#include <cpu/cpu.h>

#include "isa.h"
#include "watchpoint.h"
#include "memory/vaddr.h"

EXTERNC int cmd_si(char *args) {
    auto [num] = parse_arg<int>(args);
    cpu_exec(num.value_or(1));
    return 0;
}

EXTERNC int cmd_info(char *args) {
    auto [type] = parse_arg<std::string>(args);
    if (type == "r")
        isa_reg_display();
    else if (type == "w") {
    } else printf("Unknown subcommand\n");
    return 0;
}

EXTERNC int cmd_x(char *args) {
    auto [oN, oExpr] = parse_arg<int, std::unique_ptr<expression>>(args);

    if (!oN) {
        printf("Missing N\n");
        return 0;
    }
    int N = *oN;

    auto expr = std::move(*oExpr);
    if (!expr) {
        printf("Invalid expression\n");
        return 0;
    }

    auto addr = expr->exec();
    for (int i = 0; i < N; i++)
        printf("0x%08X ", vaddr_read(addr + i * 4, 4));
    printf("\n");
    return 0;
}

EXTERNC int cmd_p(char *args) {
    auto [oExpr] = parse_arg<std::unique_ptr<expression>>(args);
    auto expr = std::move(*oExpr);
    if (!expr) {
        printf("Invalid expression\n");
        return 0;
    }
    printf("%u\n", expr->exec());
    return 0;
}

EXTERNC int cmd_w(char *args) {
    auto [oExpr] = parse_arg<std::unique_ptr<expression>>(args);
    auto expr = std::move(*oExpr);
    if (!expr) {
        printf("Invalid expression\n");
        return 0;
    }
    watchpoints.emplace_back(std::move(expr));
    printf("Added watchpoint %lu", watchpoints.size());
    return 0;
}

EXTERNC int cmd_d(char *args) {
    auto [oN] = parse_arg<int>(args);
    if (!oN) {
        printf("Missing N\n");
        return 0;
    }

    auto N = *oN;
    if (N <= 0 || static_cast<unsigned>(N) > watchpoints.size()) {
        printf("Invalid N\n");
        return 0;
    }

    --N;
    auto iter = watchpoints.begin();
    while (N) {
        ++iter;
        --N;
    }
    watchpoints.erase(iter);

    printf("Removed watchpoint %d", N);
    return 0;
}