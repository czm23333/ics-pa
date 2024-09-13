#include "expr.h"

#include <charconv>
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include "isa.h"
#include "memory/vaddr.h"

using parser_type = std::function<std::unique_ptr<expression>(unsigned currentLevel, std::string_view sv)>;
std::vector<parser_type> parsers[PRECEDENCE_LIMIT + 1];

std::unique_ptr<expression> parse_expr(unsigned levelLimit, std::string_view sv) {
    while (!sv.empty() && sv.front() == ' ') sv.remove_prefix(1);
    while (!sv.empty() && sv.back() == ' ') sv.remove_suffix(1);
    for (unsigned level = 0; level <= levelLimit; ++level)
        for (auto &parser : parsers[level]) {
            auto res = parser(level, sv);
            if (res) return res;
        }
    return {};
}

class number : public expression {
public:
    const unsigned num;

    explicit number(const unsigned num) : num(num) {
    }

    unsigned exec() override {
        return num;
    }
};

static const unsigned brace_reg = [] {
    parsers[0].emplace_back([](unsigned, std::string_view sv) -> std::unique_ptr<expression> {
        if (sv.size() < 2) return {};
        if (sv.front() == '(' && sv.back() == ')') {
            sv.remove_prefix(1);
            sv.remove_suffix(1);
            return parse_expr(PRECEDENCE_LIMIT, sv);
        }
        return {};
    });
    return 0;
}();

static const unsigned number_reg = [] {
    parsers[0].emplace_back([](unsigned, std::string_view sv) -> std::unique_ptr<expression> {
        if (sv.empty()) return {};
        int base = 10;
        if (sv.size() >= 2 && sv[0] == '0' && (sv[1] == 'x' || sv[1] == 'X')) {
            sv.remove_prefix(2);
            base = 16;
        }
        unsigned res = 0;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), res, base);
        if (ec == std::errc() && ptr == sv.data() + sv.size()) return std::make_unique<number>(res);
        return {};
    });
    return 0;
}();

class register_access : public expression {
public:
    const std::string name;

    explicit register_access(const std::string &name) : name(name) {
    }

    unsigned exec() override {
        bool tmp;
        return isa_reg_str2val(name.c_str(), &tmp);
    }
};

static const unsigned register_access_reg = [] {
    parsers[0].emplace_back([](unsigned, std::string_view sv) -> std::unique_ptr<expression> {
        if (sv.empty() || sv.front() != '$') return {};
        sv.remove_prefix(1);
        return std::make_unique<register_access>(std::string(sv));
    });
    return 0;
}();

class binary_operation : public expression {
public:
    const std::unique_ptr<expression> left, right;

    binary_operation(std::unique_ptr<expression> &&left, std::unique_ptr<expression> &&right) : left(std::move(left)),
        right(std::move(right)) {
    }
};

#define BINARY_REG(op, level, c) \
static const unsigned op##_reg = [] { \
    parsers[level].emplace_back([](unsigned currentLevel, std::string_view sv) -> std::unique_ptr<expression> { \
        if (sv.empty()) return {}; \
        constexpr std::string_view toFind = c; \
        auto pos = sv.rfind(toFind); \
        if (pos == std::string_view::npos) return {}; \
        auto right = parse_expr(currentLevel - 1, sv.substr(pos + toFind.size())); \
        if (!right) return {}; \
        auto left = parse_expr(currentLevel, sv.substr(pos)); \
        if (!left) return {}; \
        return std::make_unique<op>(std::move(left), std::move(right)); \
    }); \
    return 0; \
}()

class addition : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() + right->exec();
    }
};

static const unsigned addition_reg = [] {
    parsers[3].emplace_back([](unsigned currentLevel, std::string_view sv) -> std::unique_ptr<expression> {
        if (sv.empty()) return {};
        constexpr std::string_view toFind = "+";
        auto pos = sv.rfind(toFind);
        if (pos == std::string_view::npos) return {};
        auto right = parse_expr(currentLevel - 1, sv.substr(pos + toFind.size()));
        if (!right) return {};
        auto left = parse_expr(currentLevel, sv.substr(pos));
        if (!left) return {};
        return std::make_unique<addition>(std::move(left), std::move(right));
    });
    return 0;
}();

class subtraction : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() - right->exec();
    }
};

BINARY_REG(subtraction, 3, "-");

class multiplication : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() * right->exec();
    }
};

BINARY_REG(multiplication, 2, "*");

class division : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() / right->exec();
    }
};

BINARY_REG(division, 2, "/");

class equal : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() == right->exec();
    }
};

BINARY_REG(equal, 5, "==");

class not_equal : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() != right->exec();
    }
};

BINARY_REG(not_equal, 5, "!=");

class greater : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() > right->exec();
    }
};

BINARY_REG(greater, 4, ">");

class less : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() < right->exec();
    }
};

BINARY_REG(less, 4, "<");

class less_equal : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() <= right->exec();
    }
};

BINARY_REG(less_equal, 4, "<=");

class greater_equal : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() >= right->exec();
    }
};

BINARY_REG(greater_equal, 4, ">=");

class bool_and : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() && right->exec();
    }
};

BINARY_REG(bool_and, 6, "&&");

class bool_or : public binary_operation {
public:
    using binary_operation::binary_operation;

    unsigned exec() override {
        return left->exec() || right->exec();
    }
};

BINARY_REG(bool_or, 7, "||");

class unary_operation : public expression {
public:
    const std::unique_ptr<expression> operand;

    unary_operation(std::unique_ptr<expression> &&operand) : operand(std::move(operand)) {
    }
};

#define UNARY_REG(op, level, c) \
static const unsigned op##_reg = [] { \
    parsers[level].emplace_back([](unsigned currentLevel, std::string_view sv) -> std::unique_ptr<expression> { \
        if (sv.empty()) return {}; \
        if (sv.front() == c) { \
            sv.remove_prefix(1); \
            if (auto res = parse_expr(currentLevel - 1, sv)) return std::make_unique<op>(std::move(res)); \
            return {}; \
        } \
        return {}; \
    }); \
    return 0; \
}()

class opposite : public unary_operation {
public:
    using unary_operation::unary_operation;

    unsigned exec() override {
        return -operand->exec();
    }
};

UNARY_REG(opposite, 1, '-');

class negate : public unary_operation {
public:
    using unary_operation::unary_operation;

    unsigned exec() override {
        return !operand->exec();
    }
};

UNARY_REG(negate, 1, '!');

class dereference : public unary_operation {
public:
    using unary_operation::unary_operation;

    unsigned exec() override {
        return vaddr_read(operand->exec(), sizeof(unsigned));
    }
};

UNARY_REG(dereference, 1, '*');