#pragma once
#include <string>
#include <cpu/cpu.h>

#include "isa.h"

class expression {
public:
    virtual ~expression() = default;

    virtual int exec();
};

class number : public expression {
public:
    const int num;
    explicit number(const int& num) : num(num) {}

    int exec() override {
        return num;
    }
};

class reg_access : public expression {
    public:
    const std::string name;
    explicit reg_access(const std::string& name) : name(name) {}

    int exec() override {
        bool tmp;
        return isa_reg_str2val(name.c_str(), &tmp);
    }
};


