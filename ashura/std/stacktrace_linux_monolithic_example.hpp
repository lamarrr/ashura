/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/fn.hpp"
#include "ashura/std/result.hpp"
#include "ashura/std/span.hpp"
#include "ashura/std/vec.hpp"

#include <cstring>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ash
{

// Monolithic Linux ELF/DWARF stacktrace example for builds where frame pointers
// are omitted. This uses:
// - .eh_frame_hdr for PC -> FDE lookup
// - .eh_frame CFI instructions to recover caller context
// - .symtab/.strtab with fallback to .dynsym/.dynstr for symbol names
#if defined(__x86_64__)
inline Result<> walk_stack_functions_linux_monolithic(Fn<void(Str, usize)> callback)
{
    // Resolve /proc/self/exe path to find this binary's mapping in /proc/self/maps.
    char    exe_path_buffer[4096];
    ssize_t exe_path_size =
      ::readlink("/proc/self/exe", exe_path_buffer, sizeof(exe_path_buffer));
    if (exe_path_size <= 0 ||
        exe_path_size >= static_cast<ssize_t>(sizeof(exe_path_buffer)))
    {
        return Err{};
    }
    Str exe_path{exe_path_buffer, static_cast<usize>(exe_path_size)};

    // Map executable ELF into memory.
    int fd = ::open("/proc/self/exe", O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        return Err{};
    }

    struct stat st{};
    if (::fstat(fd, &st) != 0 || st.st_size <= 0)
    {
        (void) ::close(fd);
        return Err{};
    }

    void * elf_data =
      ::mmap(nullptr, static_cast<usize>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    (void) ::close(fd);

    if (elf_data == MAP_FAILED)
    {
        return Err{};
    }

    defer unmap_{[&] { ::munmap(elf_data, static_cast<usize>(st.st_size)); }};

    Span<u8 const> elf{static_cast<u8 const *>(elf_data), static_cast<usize>(st.st_size)};

    if (elf.size() < sizeof(Elf64_Ehdr))
    {
        return Err{};
    }

    auto const * ehdr = reinterpret_cast<Elf64_Ehdr const *>(elf.data());
    if (!(::memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0 &&
          ehdr->e_ident[EI_CLASS] == ELFCLASS64 &&
          ehdr->e_ident[EI_DATA] == ELFDATA2LSB))
    {
        return Err{};
    }

    usize shoff    = static_cast<usize>(ehdr->e_shoff);
    usize shnum    = static_cast<usize>(ehdr->e_shnum);
    usize shentsz  = static_cast<usize>(ehdr->e_shentsize);
    usize shstrndx = static_cast<usize>(ehdr->e_shstrndx);

    if (!(shoff <= elf.size() && (shoff + shnum * shentsz) <= elf.size() &&
          shstrndx < shnum))
    {
        return Err{};
    }

    Span<Elf64_Shdr const> sections{
      reinterpret_cast<Elf64_Shdr const *>(elf.data() + shoff), shnum};

    auto const & shstr = sections[shstrndx];
    if (!(shstr.sh_offset <= elf.size() &&
          (shstr.sh_offset + shstr.sh_size) <= elf.size()))
    {
        return Err{};
    }

    auto const * section_names =
      reinterpret_cast<char const *>(elf.data() + shstr.sh_offset);

    auto find_section = [&](char const * name) -> Option<Elf64_Shdr const &> {
        for (auto const & s : sections)
        {
            if (s.sh_name >= shstr.sh_size)
            {
                continue;
            }
            if (::strcmp(section_names + s.sh_name, name) == 0)
            {
                return s;
            }
        }
        return none;
    };

    auto eh_frame_hdr_sec = find_section(".eh_frame_hdr");
    auto eh_frame_sec     = find_section(".eh_frame");
    auto text_sec         = find_section(".text");

    if (eh_frame_hdr_sec.is_none() || eh_frame_sec.is_none() || text_sec.is_none())
    {
        return Err{};
    }

    auto sym_sec = find_section(".symtab");
    auto str_sec = find_section(".strtab");
    if (sym_sec.is_none() || str_sec.is_none())
    {
        sym_sec = find_section(".dynsym");
        str_sec = find_section(".dynstr");
    }
    if (sym_sec.is_none() || str_sec.is_none())
    {
        return Err{};
    }

    if (!(eh_frame_hdr_sec->sh_offset <= elf.size() &&
          (eh_frame_hdr_sec->sh_offset + eh_frame_hdr_sec->sh_size) <= elf.size() &&
          eh_frame_sec->sh_offset <= elf.size() &&
          (eh_frame_sec->sh_offset + eh_frame_sec->sh_size) <= elf.size() &&
          sym_sec->sh_offset <= elf.size() &&
          (sym_sec->sh_offset + sym_sec->sh_size) <= elf.size() &&
          str_sec->sh_offset <= elf.size() &&
          (str_sec->sh_offset + str_sec->sh_size) <= elf.size() &&
          (sym_sec->sh_size % sizeof(Elf64_Sym) == 0)))
    {
        return Err{};
    }

    Span<u8 const> eh_frame_hdr{
      elf.data() + eh_frame_hdr_sec->sh_offset,
      static_cast<usize>(eh_frame_hdr_sec->sh_size)};

    Span<u8 const> eh_frame{
      elf.data() + eh_frame_sec->sh_offset,
      static_cast<usize>(eh_frame_sec->sh_size)};

    Span<Elf64_Sym const> symbols{
      reinterpret_cast<Elf64_Sym const *>(elf.data() + sym_sec->sh_offset),
      static_cast<usize>(sym_sec->sh_size / sizeof(Elf64_Sym))};

    Span<char const> strings{
      reinterpret_cast<char const *>(elf.data() + str_sec->sh_offset),
      static_cast<usize>(str_sec->sh_size)};

    // Parse /proc/self/maps for module load bias and stack bounds.
    int maps_fd = ::open("/proc/self/maps", O_RDONLY | O_CLOEXEC);
    if (maps_fd < 0)
    {
        return Err{};
    }
    defer close_maps_{[&] { ::close(maps_fd); }};

    Vec<char> maps_data{default_allocator};
    char      read_buffer[4096];

    for (;;)
    {
        ssize_t n = ::read(maps_fd, read_buffer, sizeof(read_buffer));
        if (n == 0)
        {
            break;
        }
        if (n < 0)
        {
            return Err{};
        }

        usize old = maps_data.size();
        if (!maps_data.extend_uninit(static_cast<usize>(n)))
        {
            return Err{};
        }
        ::memcpy(maps_data.data() + old, read_buffer, static_cast<usize>(n));
    }

    auto parse_hex = [&](Str txt, usize & out) -> bool {
        if (txt.is_empty())
        {
            return false;
        }
        usize value = 0;
        for (char c : txt)
        {
            u8 n = 0;
            if (c >= '0' && c <= '9')
            {
                n = static_cast<u8>(c - '0');
            }
            else if (c >= 'a' && c <= 'f')
            {
                n = static_cast<u8>(10 + (c - 'a'));
            }
            else if (c >= 'A' && c <= 'F')
            {
                n = static_cast<u8>(10 + (c - 'A'));
            }
            else
            {
                return false;
            }
            value = (value << 4) | n;
        }
        out = value;
        return true;
    };

    usize module_base = 0;
    usize stack_begin = 0;
    usize stack_end   = 0;

    usize line_begin = 0;
    while (line_begin < maps_data.size())
    {
        usize line_end = line_begin;
        while (line_end < maps_data.size() && maps_data[line_end] != '\n')
        {
            line_end++;
        }

        Str line{maps_data.data() + line_begin, line_end - line_begin};
        line_begin = line_end + 1;

        usize dash = 0;
        while (dash < line.size() && line[dash] != '-')
        {
            dash++;
        }
        if (dash == line.size())
        {
            continue;
        }

        usize start = 0;
        usize end   = 0;
        if (!parse_hex(line.slice(0, dash), start))
        {
            continue;
        }

        usize i = dash + 1;
        usize j = i;
        while (j < line.size() && line[j] != ' ')
        {
            j++;
        }
        if (!parse_hex(line.slice(i, j - i), end))
        {
            continue;
        }

        i = j;
        while (i < line.size() && line[i] == ' ')
        {
            i++;
        }
        if ((i + 4) > line.size())
        {
            continue;
        }
        Str perms = line.slice(i, 4);
        i += 4;

        while (i < line.size() && line[i] == ' ')
        {
            i++;
        }

        usize off_begin = i;
        while (i < line.size() && line[i] != ' ')
        {
            i++;
        }
        usize file_offset = 0;
        if (!parse_hex(line.slice(off_begin, i - off_begin), file_offset))
        {
            continue;
        }

        u32 spaces = 0;
        while (i < line.size() && spaces < 2)
        {
            if (line[i] == ' ')
            {
                while (i < line.size() && line[i] == ' ')
                {
                    i++;
                }
                spaces++;
            }
            else
            {
                i++;
            }
        }

        while (i < line.size() && line[i] == ' ')
        {
            i++;
        }

        Str path = line.slice(i);

        if (path.size() == 7 && ::memcmp(path.data(), "[stack]", 7) == 0)
        {
            stack_begin = start;
            stack_end   = end;
            continue;
        }

        if (path.size() == exe_path.size() &&
            ::memcmp(path.data(), exe_path.data(), exe_path.size()) == 0 &&
            perms[2] == 'x')
        {
            module_base = start - file_offset;
        }
    }

    if (module_base == 0 || stack_begin >= stack_end)
    {
        return Err{};
    }

    // DWARF encoded pointer decoding.
    auto read_uleb128 = [&](u8 const *& p, u8 const * endp) -> usize {
        usize v = 0;
        u32   s = 0;
        while (p < endp)
        {
            u8 b = *p++;
            v |= (static_cast<usize>(b & 0x7F) << s);
            if ((b & 0x80) == 0)
            {
                break;
            }
            s += 7;
        }
        return v;
    };

    auto read_sleb128 = [&](u8 const *& p, u8 const * endp) -> isize {
        isize v = 0;
        u32   s = 0;
        u8    b = 0;
        while (p < endp)
        {
            b = *p++;
            v |= (static_cast<isize>(b & 0x7F) << s);
            s += 7;
            if ((b & 0x80) == 0)
            {
                break;
            }
        }
        if ((s < (sizeof(isize) * 8)) && (b & 0x40))
        {
            v |= ~((static_cast<isize>(1) << s) - 1);
        }
        return v;
    };

    auto read_u16 = [&](u8 const * src) -> u16 {
        u16 x;
        ::memcpy(&x, src, sizeof(x));
        return x;
    };

    auto read_u32 = [&](u8 const * src) -> u32 {
        u32 x;
        ::memcpy(&x, src, sizeof(x));
        return x;
    };

    auto read_u64 = [&](u8 const * src) -> u64 {
        u64 x;
        ::memcpy(&x, src, sizeof(x));
        return x;
    };

    auto read_i16 = [&](u8 const * src) -> i16 {
        i16 x;
        ::memcpy(&x, src, sizeof(x));
        return x;
    };

    auto read_i32 = [&](u8 const * src) -> i32 {
        i32 x;
        ::memcpy(&x, src, sizeof(x));
        return x;
    };

    auto read_i64 = [&](u8 const * src) -> i64 {
        i64 x;
        ::memcpy(&x, src, sizeof(x));
        return x;
    };

    auto read_encoded = [&](u8 encoding, u8 const *& p, u8 const * endp,
                            usize field_runtime, usize datarel_base,
                            usize & out) -> bool {
        constexpr u8 DW_EH_PE_OMIT     = 0xFF;
        constexpr u8 DW_EH_PE_PTR      = 0x00;
        constexpr u8 DW_EH_PE_ULEB128  = 0x01;
        constexpr u8 DW_EH_PE_UDATA2   = 0x02;
        constexpr u8 DW_EH_PE_UDATA4   = 0x03;
        constexpr u8 DW_EH_PE_UDATA8   = 0x04;
        constexpr u8 DW_EH_PE_SLEB128  = 0x09;
        constexpr u8 DW_EH_PE_SDATA2   = 0x0A;
        constexpr u8 DW_EH_PE_SDATA4   = 0x0B;
        constexpr u8 DW_EH_PE_SDATA8   = 0x0C;
        constexpr u8 DW_EH_PE_PCREL    = 0x10;
        constexpr u8 DW_EH_PE_DATAREL  = 0x30;
        constexpr u8 DW_EH_PE_INDIRECT = 0x80;

        if (encoding == DW_EH_PE_OMIT)
        {
            return false;
        }

        isize raw = 0;

        switch (encoding & 0x0F)
        {
            case DW_EH_PE_PTR:
                if ((p + sizeof(usize)) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(*reinterpret_cast<usize const *>(p));
                p += sizeof(usize);
                break;
            case DW_EH_PE_ULEB128:
                raw = static_cast<isize>(read_uleb128(p, endp));
                break;
            case DW_EH_PE_UDATA2:
                if ((p + 2) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(read_u16(p));
                p += 2;
                break;
            case DW_EH_PE_UDATA4:
                if ((p + 4) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(read_u32(p));
                p += 4;
                break;
            case DW_EH_PE_UDATA8:
                if ((p + 8) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(read_u64(p));
                p += 8;
                break;
            case DW_EH_PE_SLEB128:
                raw = read_sleb128(p, endp);
                break;
            case DW_EH_PE_SDATA2:
                if ((p + 2) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(read_i16(p));
                p += 2;
                break;
            case DW_EH_PE_SDATA4:
                if ((p + 4) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(read_i32(p));
                p += 4;
                break;
            case DW_EH_PE_SDATA8:
                if ((p + 8) > endp)
                {
                    return false;
                }
                raw = static_cast<isize>(read_i64(p));
                p += 8;
                break;
            default:
                return false;
        }

        isize base = 0;
        switch (encoding & 0x70)
        {
            case 0:
                base = 0;
                break;
            case DW_EH_PE_PCREL:
                base = static_cast<isize>(field_runtime);
                break;
            case DW_EH_PE_DATAREL:
                base = static_cast<isize>(datarel_base);
                break;
            default:
                return false;
        }

        out = static_cast<usize>(base + raw);

        if ((encoding & DW_EH_PE_INDIRECT) != 0)
        {
            out = *reinterpret_cast<usize const *>(out);
        }

        return true;
    };

    // Parse .eh_frame_hdr.
    if (eh_frame_hdr.size() < 4)
    {
        return Err{};
    }

    usize eh_frame_hdr_runtime = module_base + eh_frame_hdr_sec->sh_addr;
    usize eh_frame_runtime     = module_base + eh_frame_sec->sh_addr;

    u8 const * hdrp   = eh_frame_hdr.data();
    u8 const * hdrend = eh_frame_hdr.data() + eh_frame_hdr.size();

    u8 version       = *hdrp++;
    u8 eh_ptr_enc    = *hdrp++;
    u8 fde_count_enc = *hdrp++;
    u8 table_enc     = *hdrp++;

    if (version != 1)
    {
        return Err{};
    }

    usize ignored_eh = 0;
    usize fde_count  = 0;

    usize field_runtime = eh_frame_hdr_runtime + static_cast<usize>(hdrp - eh_frame_hdr.data());
    if (!read_encoded(eh_ptr_enc, hdrp, hdrend, field_runtime, eh_frame_hdr_runtime,
                      ignored_eh))
    {
        return Err{};
    }

    field_runtime = eh_frame_hdr_runtime + static_cast<usize>(hdrp - eh_frame_hdr.data());
    if (!read_encoded(fde_count_enc, hdrp, hdrend, field_runtime,
                      eh_frame_hdr_runtime, fde_count))
    {
        return Err{};
    }

    u8 const * table_begin = hdrp;

    // Current unwind context (DWARF register numbers for x86_64).
    // 6=rbp, 7=rsp, 16=rip(return-address pseudo-reg).
    usize regs[64] = {};
    {
        usize rsp = 0;
        usize rbp = 0;
        asm volatile("mov %%rsp, %0" : "=r"(rsp));
        asm volatile("mov %%rbp, %0" : "=r"(rbp));
        regs[7]  = rsp;
        regs[6]  = rbp;
        regs[16] = reinterpret_cast<usize>(__builtin_return_address(0));
    }

    static constexpr char UNKNOWN[] = "<unknown>";

    // Unwind up to a fixed depth.
    for (usize frame_index = 0; frame_index < 128; frame_index++)
    {
        usize pc = regs[16];
        if (pc == 0)
        {
            break;
        }

        // Symbolize current pc.
        usize link_pc = pc > module_base ? (pc - module_base) : 0;
        Elf64_Sym const * best = nullptr;
        for (auto const & sym : symbols)
        {
            bool is_func = ELF64_ST_TYPE(sym.st_info) == STT_FUNC &&
                           sym.st_name != 0 && sym.st_shndx != SHN_UNDEF;
            bool has_name = sym.st_name < strings.size() && strings[sym.st_name] != '\0';
            if (!is_func || !has_name || sym.st_value == 0)
            {
                continue;
            }

            usize s0 = static_cast<usize>(sym.st_value);
            usize s1 =
              sym.st_size == 0 ? USIZE_MAX : (s0 + static_cast<usize>(sym.st_size));

            if (!(s0 <= link_pc && link_pc < s1))
            {
                continue;
            }

            if (best == nullptr || s0 > static_cast<usize>(best->st_value))
            {
                best = &sym;
            }
        }

        if (best == nullptr)
        {
            callback(Str{UNKNOWN, sizeof(UNKNOWN) - 1}, pc);
        }
        else
        {
            char const * name_ptr = strings.data() + best->st_name;
            usize        name_len = 0;
            while ((best->st_name + name_len) < strings.size() &&
                   name_ptr[name_len] != '\0')
            {
                name_len++;
            }
            callback(Str{name_ptr, name_len}, pc);
        }

        // Find best row in .eh_frame_hdr table: greatest initial_location <= pc.
        u8 const * rowp = table_begin;
        usize      best_initial_location = 0;
        usize      best_fde_pointer      = 0;
        bool       found_entry           = false;

        for (usize row = 0; row < fde_count && rowp < hdrend; row++)
        {
            usize row_runtime =
              eh_frame_hdr_runtime + static_cast<usize>(rowp - eh_frame_hdr.data());
            usize initial_location = 0;
            if (!read_encoded(table_enc, rowp, hdrend, row_runtime,
                              eh_frame_hdr_runtime, initial_location))
            {
                break;
            }

            row_runtime =
              eh_frame_hdr_runtime + static_cast<usize>(rowp - eh_frame_hdr.data());
            usize fde_pointer = 0;
            if (!read_encoded(table_enc, rowp, hdrend, row_runtime,
                              eh_frame_hdr_runtime, fde_pointer))
            {
                break;
            }

            if (initial_location <= pc &&
                (!found_entry || initial_location > best_initial_location))
            {
                found_entry           = true;
                best_initial_location = initial_location;
                best_fde_pointer      = fde_pointer;
            }
        }

        if (!found_entry || best_fde_pointer < eh_frame_runtime ||
            best_fde_pointer >= (eh_frame_runtime + eh_frame.size()))
        {
            break;
        }

        // Parse selected FDE and its CIE from .eh_frame.
        u8 const * fde = eh_frame.data() + (best_fde_pointer - eh_frame_runtime);
        u8 const * fde_end_limit = eh_frame.data() + eh_frame.size();

        if ((fde + 8) > fde_end_limit)
        {
            break;
        }

        u32 fde_len = read_u32(fde);
        if (fde_len == 0 || (fde + 4 + fde_len) > fde_end_limit)
        {
            break;
        }

        u8 const * fde_payload      = fde + 4;
        u8 const * fde_payload_end  = fde_payload + fde_len;
        u8 const * cie_ptr_location = fde_payload;

        u32 cie_offset_back = read_u32(cie_ptr_location);
        if (cie_offset_back == 0)
        {
            break;
        }

        if (cie_ptr_location < eh_frame.data() + cie_offset_back)
        {
            break;
        }

        u8 const * cie = cie_ptr_location - cie_offset_back;
        if ((cie + 8) > fde_end_limit)
        {
            break;
        }

        u32 cie_len = read_u32(cie);
        if (cie_len == 0 || (cie + 4 + cie_len) > fde_end_limit)
        {
            break;
        }

        u8 const * cie_payload     = cie + 4;
        u8 const * cie_payload_end = cie_payload + cie_len;

        // Validate CIE id in .eh_frame format (must be 0 for 32-bit length form).
        if (read_u32(cie_payload) != 0)
        {
            break;
        }

        u8 const * cp = cie_payload + 4;
        if (cp >= cie_payload_end)
        {
            break;
        }

        u8 cie_version = *cp++;
        (void) cie_version;

        // Augmentation string.
        char augmentation[32] = {};
        usize augmentation_len = 0;
        while (cp < cie_payload_end && *cp != '\0')
        {
            if (augmentation_len < (sizeof(augmentation) - 1))
            {
                augmentation[augmentation_len++] = static_cast<char>(*cp);
            }
            cp++;
        }
        if (cp >= cie_payload_end)
        {
            break;
        }
        cp++;    // skip NUL

        usize code_align = read_uleb128(cp, cie_payload_end);
        isize data_align = read_sleb128(cp, cie_payload_end);
        usize ra_reg     = read_uleb128(cp, cie_payload_end);

        // Defaults for FDE encoding if no augmentation says otherwise.
        u8 fde_pointer_encoding = 0x00;    // DW_EH_PE_absptr

        // Parse CIE augmentation data when present.
        if (augmentation_len > 0 && augmentation[0] == 'z')
        {
            usize aug_size = read_uleb128(cp, cie_payload_end);
            u8 const * aug_end = cp + aug_size;
            if (aug_end > cie_payload_end)
            {
                break;
            }

            for (usize a = 1; a < augmentation_len; a++)
            {
                char ch = augmentation[a];
                if (ch == 'R')
                {
                    if (cp >= aug_end)
                    {
                        break;
                    }
                    fde_pointer_encoding = *cp++;
                }
                else if (ch == 'P')
                {
                    if (cp >= aug_end)
                    {
                        break;
                    }
                    u8 enc = *cp++;
                    usize ignored = 0;
                    usize addr = eh_frame_runtime + static_cast<usize>(cp - eh_frame.data());
                    if (!read_encoded(enc, cp, aug_end, addr, eh_frame_runtime, ignored))
                    {
                        break;
                    }
                }
                else if (ch == 'L')
                {
                    if (cp >= aug_end)
                    {
                        break;
                    }
                    cp++;
                }
            }

            cp = aug_end;
        }

        // CIE initial instructions start at cp and run to cie_payload_end.
        u8 const * cie_instructions = cp;

        // Parse FDE payload fields.
        u8 const * fp = fde_payload + 4;    // skip CIE pointer

        usize fde_pc_begin_addr = eh_frame_runtime + static_cast<usize>(fp - eh_frame.data());
        usize fde_pc_begin      = 0;
        if (!read_encoded(fde_pointer_encoding, fp, fde_payload_end, fde_pc_begin_addr,
                          eh_frame_runtime, fde_pc_begin))
        {
            break;
        }

        usize fde_pc_range_addr = eh_frame_runtime + static_cast<usize>(fp - eh_frame.data());
        usize fde_pc_range      = 0;
        if (!read_encoded(fde_pointer_encoding & 0x0F, fp, fde_payload_end,
                          fde_pc_range_addr, eh_frame_runtime, fde_pc_range))
        {
            break;
        }

        if (!(fde_pc_begin <= pc && pc < (fde_pc_begin + fde_pc_range)))
        {
            // Rare but possible with nearest-lower table row fallback.
            break;
        }

        if (augmentation_len > 0 && augmentation[0] == 'z')
        {
            usize fde_aug_size = read_uleb128(fp, fde_payload_end);
            if ((fp + fde_aug_size) > fde_payload_end)
            {
                break;
            }
            fp += fde_aug_size;
        }

        u8 const * fde_instructions     = fp;
        u8 const * fde_instructions_end = fde_payload_end;

        // Rule machine for a single unwind step.
        // Reg rules: 0=undefined,1=same_value,2=offset(cfa+off),3=register(regno),4=val_offset(cfa+off)
        u8    rule_kind[64]      = {};
        isize rule_offset[64]    = {};
        u16   rule_reg[64]       = {};
        u8    init_rule_kind[64] = {};
        isize init_rule_offset[64] = {};
        u16   init_rule_reg[64]  = {};

        usize cfa_reg    = 0;
        isize cfa_offset = 0;

        auto apply_stream = [&](u8 const * begin, u8 const * end,
                                bool stop_on_pc, usize stream_pc_begin,
                                usize target_pc,
                                u8 * rk, isize * ro, u16 * rr,
                                usize & cfa_r, isize & cfa_off,
                                u8 const *& consumed_until) -> bool {
            u8 const * p = begin;
            usize loc = stream_pc_begin;

            while (p < end)
            {
                u8 op = *p++;
                u8 primary = op & 0xC0;

                if (primary == 0x40)
                {
                    usize delta = static_cast<usize>(op & 0x3F) * code_align;
                    usize next_loc = loc + delta;
                    if (stop_on_pc && next_loc > target_pc)
                    {
                        consumed_until = p;
                        return true;
                    }
                    loc = next_loc;
                    continue;
                }
                else if (primary == 0x80)
                {
                    usize reg = static_cast<usize>(op & 0x3F);
                    usize factored = read_uleb128(p, end);
                    if (reg < 64)
                    {
                        rk[reg] = 2;
                        ro[reg] = static_cast<isize>(factored) * data_align;
                    }
                    continue;
                }
                else if (primary == 0xC0)
                {
                    usize reg = static_cast<usize>(op & 0x3F);
                    if (reg < 64)
                    {
                        rk[reg] = init_rule_kind[reg];
                        ro[reg] = init_rule_offset[reg];
                        rr[reg] = init_rule_reg[reg];
                    }
                    continue;
                }

                switch (op)
                {
                    case 0x00:    // DW_CFA_nop
                        break;
                    case 0x01:    // DW_CFA_set_loc
                    {
                        if ((p + sizeof(usize)) > end)
                        {
                            return false;
                        }
                        usize next_loc = *reinterpret_cast<usize const *>(p);
                        p += sizeof(usize);
                        if (stop_on_pc && next_loc > target_pc)
                        {
                            consumed_until = p;
                            return true;
                        }
                        loc = next_loc;
                        break;
                    }
                    case 0x02:    // DW_CFA_advance_loc1
                    {
                        if ((p + 1) > end)
                        {
                            return false;
                        }
                        usize next_loc = loc + (static_cast<usize>(*p++) * code_align);
                        if (stop_on_pc && next_loc > target_pc)
                        {
                            consumed_until = p;
                            return true;
                        }
                        loc = next_loc;
                        break;
                    }
                    case 0x03:    // DW_CFA_advance_loc2
                    {
                        if ((p + 2) > end)
                        {
                            return false;
                        }
                        usize next_loc =
                          loc + (static_cast<usize>(read_u16(p)) * code_align);
                        p += 2;
                        if (stop_on_pc && next_loc > target_pc)
                        {
                            consumed_until = p;
                            return true;
                        }
                        loc = next_loc;
                        break;
                    }
                    case 0x04:    // DW_CFA_advance_loc4
                    {
                        if ((p + 4) > end)
                        {
                            return false;
                        }
                        usize next_loc =
                          loc + (static_cast<usize>(read_u32(p)) * code_align);
                        p += 4;
                        if (stop_on_pc && next_loc > target_pc)
                        {
                            consumed_until = p;
                            return true;
                        }
                        loc = next_loc;
                        break;
                    }
                    case 0x05:    // DW_CFA_offset_extended
                    {
                        usize reg = read_uleb128(p, end);
                        usize factored = read_uleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = 2;
                            ro[reg] = static_cast<isize>(factored) * data_align;
                        }
                        break;
                    }
                    case 0x06:    // DW_CFA_restore_extended
                    {
                        usize reg = read_uleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = init_rule_kind[reg];
                            ro[reg] = init_rule_offset[reg];
                            rr[reg] = init_rule_reg[reg];
                        }
                        break;
                    }
                    case 0x07:    // DW_CFA_undefined
                    {
                        usize reg = read_uleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = 0;
                        }
                        break;
                    }
                    case 0x08:    // DW_CFA_same_value
                    {
                        usize reg = read_uleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = 1;
                        }
                        break;
                    }
                    case 0x09:    // DW_CFA_register
                    {
                        usize reg1 = read_uleb128(p, end);
                        usize reg2 = read_uleb128(p, end);
                        if (reg1 < 64 && reg2 < 64)
                        {
                            rk[reg1] = 3;
                            rr[reg1] = static_cast<u16>(reg2);
                        }
                        break;
                    }
                    case 0x0C:    // DW_CFA_def_cfa
                    {
                        cfa_r   = read_uleb128(p, end);
                        cfa_off = static_cast<isize>(read_uleb128(p, end));
                        break;
                    }
                    case 0x0D:    // DW_CFA_def_cfa_register
                    {
                        cfa_r = read_uleb128(p, end);
                        break;
                    }
                    case 0x0E:    // DW_CFA_def_cfa_offset
                    {
                        cfa_off = static_cast<isize>(read_uleb128(p, end));
                        break;
                    }
                    case 0x11:    // DW_CFA_offset_extended_sf
                    {
                        usize reg = read_uleb128(p, end);
                        isize factored = read_sleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = 2;
                            ro[reg] = factored * data_align;
                        }
                        break;
                    }
                    case 0x12:    // DW_CFA_def_cfa_sf
                    {
                        cfa_r   = read_uleb128(p, end);
                        cfa_off = read_sleb128(p, end) * data_align;
                        break;
                    }
                    case 0x13:    // DW_CFA_def_cfa_offset_sf
                    {
                        cfa_off = read_sleb128(p, end) * data_align;
                        break;
                    }
                    case 0x14:    // DW_CFA_val_offset
                    {
                        usize reg = read_uleb128(p, end);
                        usize factored = read_uleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = 4;
                            ro[reg] = static_cast<isize>(factored) * data_align;
                        }
                        break;
                    }
                    case 0x15:    // DW_CFA_val_offset_sf
                    {
                        usize reg = read_uleb128(p, end);
                        isize factored = read_sleb128(p, end);
                        if (reg < 64)
                        {
                            rk[reg] = 4;
                            ro[reg] = factored * data_align;
                        }
                        break;
                    }
                    case 0x2E:    // DW_CFA_GNU_args_size
                    {
                        (void) read_uleb128(p, end);
                        break;
                    }
                    default:
                        return false;
                }
            }

            consumed_until = p;
            return true;
        };

        // Apply CIE initial instructions fully.
        u8 const * consumed = nullptr;
        if (!apply_stream(cie_instructions, cie_payload_end, false, fde_pc_begin,
                          pc, rule_kind, rule_offset, rule_reg,
                          cfa_reg, cfa_offset, consumed))
        {
            break;
        }

        for (usize r = 0; r < 64; r++)
        {
            init_rule_kind[r]   = rule_kind[r];
            init_rule_offset[r] = rule_offset[r];
            init_rule_reg[r]    = rule_reg[r];
        }

        // Apply FDE instructions only up to target PC.
        if (!apply_stream(fde_instructions, fde_instructions_end, true, fde_pc_begin,
                          pc, rule_kind, rule_offset, rule_reg,
                          cfa_reg, cfa_offset, consumed))
        {
            break;
        }

        if (cfa_reg >= 64)
        {
            break;
        }

        usize cfa = static_cast<usize>(static_cast<isize>(regs[cfa_reg]) + cfa_offset);
        if (!(cfa >= stack_begin && (cfa + sizeof(usize)) <= stack_end))
        {
            break;
        }

        usize next_regs[64] = {};

        for (usize r = 0; r < 64; r++)
        {
            switch (rule_kind[r])
            {
                case 0:    // undefined
                    next_regs[r] = 0;
                    break;
                case 1:    // same_value
                    next_regs[r] = regs[r];
                    break;
                case 2:    // offset
                {
                    usize addr =
                      static_cast<usize>(static_cast<isize>(cfa) + rule_offset[r]);
                    if (addr >= stack_begin && (addr + sizeof(usize)) <= stack_end)
                    {
                        next_regs[r] = *reinterpret_cast<usize const *>(addr);
                    }
                    else
                    {
                        next_regs[r] = 0;
                    }
                    break;
                }
                case 3:    // register
                    if (rule_reg[r] < 64)
                    {
                        next_regs[r] = regs[rule_reg[r]];
                    }
                    else
                    {
                        next_regs[r] = 0;
                    }
                    break;
                case 4:    // val_offset
                    next_regs[r] =
                      static_cast<usize>(static_cast<isize>(cfa) + rule_offset[r]);
                    break;
                default:
                    next_regs[r] = 0;
                    break;
            }
        }

        // DWARF canonical expectation for caller stack pointer is CFA.
        next_regs[7] = cfa;

        // Ensure caller PC is populated even if rules did not explicitly provide it.
        if (ra_reg < 64 && next_regs[16] == 0)
        {
            next_regs[16] = next_regs[ra_reg];
        }

        if (next_regs[16] == 0 || next_regs[16] == regs[16])
        {
            break;
        }

        for (usize r = 0; r < 64; r++)
        {
            regs[r] = next_regs[r];
        }
    }

    return Ok{};
}

#else

inline Result<> walk_stack_functions_linux_monolithic(Fn<void(Str, usize)> callback)
{
    (void) callback;
    return Err{};
}

#endif

}    // namespace ash
