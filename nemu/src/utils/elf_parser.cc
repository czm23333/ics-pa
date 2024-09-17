#include <elf.h>
#include <string>
#include <utility>
#include <vector>

#include "common.h"

struct function_space {
    std::string name;
    Elf32_Addr addr;
    Elf32_Word size;

    function_space(std::string name, Elf32_Addr addr, Elf32_Word size) : name(std::move(name)), addr(addr), size(size) {}
};
std::vector<function_space> functions;

extern "C" void load_elf(char* filePath) {
    if (filePath == nullptr) return;

    FILE* file = fopen(filePath, "r");
    if (!file) return;
    Elf32_Ehdr elfHeader;
    fread(&elfHeader, sizeof(Elf32_Ehdr), 1, file);
    if (elfHeader.e_ident[EI_CLASS] != ELFCLASS32) return;

    std::vector<char> strings;
    std::vector<Elf32_Sym> symbols;
    fseek(file, elfHeader.e_shoff, SEEK_SET);
    for (Elf32_Half i = 0; i < elfHeader.e_shnum; i++) {
        Elf32_Shdr table;
        fread(&table, sizeof(Elf32_Shdr), 1, file);
        const auto cur = ftell(file);
        if (table.sh_type == SHT_SYMTAB) {
            fseek(file, table.sh_offset, SEEK_SET);
            const auto numEntries = table.sh_size / table.sh_entsize;
            for (Elf32_Word j = 0; j < numEntries; j++) {
                Elf32_Sym symbol;
                fread(&symbol, sizeof(Elf32_Sym), 1, file);
                if (ELF32_ST_TYPE(symbol.st_info) != STT_FUNC) continue;
                symbols.emplace_back(symbol);
            }
        } else if (table.sh_type == SHT_STRTAB) {
            fseek(file, table.sh_offset, SEEK_SET);
            const auto ptr = &*strings.insert(strings.end(), table.sh_size, '\0');
            fread(ptr, sizeof(char), table.sh_size, file);
        }
        fseek(file, cur, SEEK_SET);
    }

    for (auto& function : symbols) {
        std::string name;
        auto iter = strings.begin() + function.st_name;
        char c;
        while ((c = *iter++) != '\0') name += c;
        functions.emplace_back(name, function.st_value, function.st_size);
    }

    fclose(file);
}

std::string get_function_name(vaddr_t addr) {
    for (auto& function : functions)
        if (function.addr <= addr && addr < function.addr + function.size) return function.name;
    return "<unknown>";
}
