#pragma once
#include <memory>
#include <list>
#include "expr.h"

class watchpoint {
    unsigned previous;
public:
    const std::unique_ptr<expression> expr;

    explicit watchpoint(std::unique_ptr<expression>&& expr) : expr(std::move(expr)) {
        previous = this->expr->exec();
    }

    bool check() {
        unsigned cur = expr->exec();
        if (cur != previous) {
            previous = cur;
            return true;
        }
        return false;
    }
};

extern std::list<watchpoint> watchpoints;