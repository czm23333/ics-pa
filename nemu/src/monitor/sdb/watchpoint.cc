#include "watchpoint.h"

#include <cstdio>
#include <cpu/watchpoint.h>

std::list<watchpoint> watchpoints;

bool check_watchpoints() {
    bool flag = false;
    size_t id = 1;
    for (auto& watchpoint : watchpoints) {
        bool res = watchpoint.check();
        flag |= res;
        if (res) printf("Watchpoint %lu hit\n", id);
        ++id;
    }
    return flag;
}
