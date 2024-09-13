#pragma once
#include <memory>
#include <string_view>

constexpr unsigned PRECEDENCE_LIMIT = 7;

class expression {
public:
    virtual ~expression() = default;

    virtual unsigned exec() = 0;
};

std::unique_ptr<expression> parse_expr(unsigned levelLimit, std::string_view sv);
