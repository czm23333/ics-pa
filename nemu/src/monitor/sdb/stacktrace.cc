#include <string>
#include <utility>
#include <vector>
#include <cpu/stacktrace.h>
#include <elf_parser.h>

struct stacktrace_entry {
    std::string functionName;
    vaddr_t address;

    stacktrace_entry(std::string functionName, vaddr_t address) : functionName(std::move(functionName)), address(address) {}
};
std::vector<stacktrace_entry> stacktrace;

EXTERNC void onCall(vaddr_t pc, vaddr_t addr) {
    if (!stacktrace.empty()) stacktrace.back().address = pc;
    stacktrace.emplace_back(get_function_name(addr), addr);
}

EXTERNC void onRet() {
    if (!stacktrace.empty()) stacktrace.pop_back();
}

EXTERNC void printStackTrace() {
    for (const auto& entry : stacktrace) {
        Log("%s: " FMT_PADDR "\n", entry.functionName.c_str(), entry.address);
    }
}
