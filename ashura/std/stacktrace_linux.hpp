/// SPDX-License-Identifier: MIT
#include "ashura/std/stacktrace.hpp"

#include "ashura/std/cfg.hpp"
#include "ashura/std/range.hpp"
#include "ashura/std/result.hpp"

#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ash
{

Option<Elf64_Shdr const &> get_section(char const *           name,
                                       Elf64_Shdr const &     strings_section_header,
                                       char const *           names,
                                       Span<Elf64_Shdr const> section_headers)
{
    auto is_section = [&](Elf64_Shdr const & s) {
        ASH_CHECK(s.sh_name < strings_section_header.sh_size, "");
        auto * section_name = names + s.sh_name;
        return strcmp(section_name, name) == 0;
    };

    auto section_header_s = find_if(section_headers, is_section);

    if (section_header_s.is_empty())
    {
        return none;
    }

    return section_header_s[0];
}

Result<> walk_stack_functions(Fn<void(Str, usize)> callback)
{
    auto fd = ::open("/proc/self/exe", O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        // TODO: check errno
        return Err{};
    }

    struct stat st{};
    if (::fstat(fd, &st) != 0 || st.st_size <= 0)
    {
        (void) ::close(fd);
        return Err{};
    }

    void * data =
      ::mmap(nullptr, static_cast<usize>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    (void) ::close(fd);
    if (data == MAP_FAILED)
    {
        return Err{};
    }

    defer unmap_{[&] { ::munmap(data, static_cast<usize>(st.st_size)); }};

    Span<u8 const> elf{static_cast<u8 const *>(data), static_cast<usize>(st.st_size)};

    ASH_CHECK(elf.size() >= sizeof(Elf64_Ehdr), "File too small to be a valid ELF");

    auto * hdr = reinterpret_cast<Elf64_Ehdr const *>(elf.data());
    ASH_CHECK(::memcmp(hdr->e_ident, ELFMAG, SELFMAG) == 0 &&
                hdr->e_ident[EI_CLASS] == ELFCLASS64 &&
                hdr->e_ident[EI_DATA] == ELFDATA2LSB,
              "Invalid ELF header");

    auto section_header_offset        = static_cast<usize>(hdr->e_shoff);
    auto section_header_count         = hdr->e_shnum;
    auto section_header_entry_size    = hdr->e_shentsize;
    auto section_header_strings_index = hdr->e_shstrndx;

    ASH_CHECK(section_header_offset <= elf.size(), "");
    ASH_CHECK((section_header_offset +
               section_header_count * section_header_entry_size) <= elf.size(),
              "");
    ASH_CHECK(section_header_strings_index < section_header_count, "");

    auto section_headers =
      Span{reinterpret_cast<Elf64_Shdr const *>(elf.data() + section_header_offset),
           section_header_count};
    auto & section_header_strings = section_headers[section_header_strings_index];
    ASH_CHECK(section_header_strings.sh_offset <= elf.size(), "");
    ASH_CHECK((section_header_strings.sh_offset + section_header_strings.sh_size) <=
                elf.size(),
              "");

    auto * names =
      reinterpret_cast<char const *>(elf.data() + section_header_strings.sh_offset);

    auto & eh_frame_section =
      get_section(".eh_frame", section_header_strings, names, section_headers)
        .unwrap("No .eh_frame section found");
    ASH_CHECK(eh_frame_section.sh_offset <= elf.size(), "");
    ASH_CHECK((eh_frame_section.sh_offset + eh_frame_section.sh_size) <= elf.size(),
              "");
    auto eh_frame_data = Span{elf.data() + eh_frame_section.sh_offset,
                              static_cast<usize>(eh_frame_section.sh_size)};
    auto eh_frame_addr = eh_frame_section.sh_addr;

    auto sym_section =
      get_section(".symtab", section_header_strings, names, section_headers);

    auto str_section =
      get_section(".strtab", section_header_strings, names, section_headers);

    if (sym_section.is_none() || str_section.is_none())
    {
        sym_section =
          get_section(".dynsym", section_header_strings, names, section_headers);
        str_section =
          get_section(".dynstr", section_header_strings, names, section_headers);

        ASH_CHECK(sym_section.is_some() && str_section.is_some(),
                  "No symbol table found");
    }

    ASH_CHECK(sym_section->sh_size % sizeof(Elf64_Sym) == 0,
              "Invalid symbol table section size");
    ASH_CHECK(sym_section->sh_offset <= elf.size(), "");
    ASH_CHECK((sym_section->sh_offset + sym_section->sh_size) <= elf.size(), "");
    ASH_CHECK(str_section->sh_offset <= elf.size(), "");
    ASH_CHECK((str_section->sh_offset + str_section->sh_size) <= elf.size(), "");

    auto symbols =
      Span{reinterpret_cast<Elf64_Sym const *>(elf.data() + sym_section->sh_offset),
           sym_section->sh_size / sizeof(Elf64_Sym)};
    auto strings =
      Span{reinterpret_cast<char const *>(elf.data() + str_section->sh_offset),
           static_cast<usize>(str_section->sh_size)};

    for (auto sym : symbols)
    {
        bool is_function_symbol = ELF64_ST_TYPE(sym.st_info) == STT_FUNC &&
                                  sym.st_name != 0 && sym.st_shndx != SHN_UNDEF;
        bool is_valid_symbol_name =
          sym.st_name < strings.size() && strings[sym.st_name] != '\0';

        if (!is_function_symbol || !is_valid_symbol_name)
        {
            continue;
        }
    }
}

}    // namespace ash
