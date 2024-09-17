#include "watchpoint.h"

#include <cstdio>
#include <cpu/watchpoint.h>

#include "debug.h"

std::list<watchpoint> watchpoints;

bool check_watchpoints() {
    bool flag = false;
    size_t id = 1;
    for (auto& watchpoint : watchpoints) {
        bool res = watchpoint.check();
        flag |= res;
        if (res) Log("Watchpoint %lu hit", id);
        ++id;
    }
    return flag;
}
