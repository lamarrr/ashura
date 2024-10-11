
/// SPDX-License-Identifier: MIT
#pragma once

/// configuration macro
#define ASH_CFG(param, value) ASH_##param##_##value

/*********************** COMPILERS ***********************/

#if defined(__GNUC__)        //  any compiler that implements the GNU compiler
                             //  extensions
#  define ASH_COMPILER_GNUC 1
#else
#  define ASH_COMPILER_GNUC 0
#endif

#if defined(__clang__)
#  define ASH_COMPILER_CLANG 1
#else
#  define ASH_COMPILER_CLANG 0
#endif

#if defined(_MSC_VER)
#  define ASH_COMPILER_MSVC 1
#else
#  define ASH_COMPILER_MSVC 0
#endif

#if defined(__EMSCRIPTEN__)
#  define ASH_COMPILER_EMSCRIPTEN 1
#else
#  define ASH_COMPILER_EMSCRIPTEN 0
#endif

#if defined(__NVCC__)
#  define ASH_COMPILER_NVCC 1
#else
#  define ASH_COMPILER_NVCC 0
#endif

#if defined(__CC_ARM)
#  define ASH_COMPILER_ARM 1
#else
#  define ASH_COMPILER_ARM 0
#endif

#if defined(__INTEL_COMPILER) || defined(__ICL)
#  define ASH_COMPILER_INTEL 1
#else
#  define ASH_COMPILER_INTEL 0
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#  define ASH_COMPILER_MINGW 1
#else
#  define ASH_COMPILER_MINGW 0
#endif

/*********************** OPERATING SYSTEMS ***********************/

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || \
    defined(WIN32)        // Any Windows
#  define ASH_OS_WINDOWS 1
#else
#  define ASH_OS_WINDOWS 0
#endif

#if defined(__unix__) || defined(unix)        // UNIX
#  define ASH_OS_UNIX 1
#else
#  define ASH_OS_UNIX 0
#endif

#if __has_include(<unistd.h>)        // Posix-compliant operating system
#  define ASH_OS_POSIX 1
#else
#  define ASH_OS_POSIX 0
#endif

#if defined(__linux__) || defined(__linux) || \
    defined(linux)        // Linux and variants like Android
#  define ASH_OS_LINUX 1
#else
#  define ASH_OS_LINUX 0
#endif

#if defined(__gnu_linux__)        // Linux OS with GNU facilities
#  define ASH_OS_GNU_LINUX 1
#else
#  define ASH_OS_GNU_LINUX 0
#endif

#if defined(__ANDROID__)        // Android, Also infers ASH_OS_LINUX
#  define ASH_OS_ANDROID 1
#else
#  define ASH_OS_ANDROID 0
#endif

#if defined(__APPLE__)        // All Apple OSs
#  define ASH_OS_APPLE 1

#  include <Availability.h>
#  include <TargetConditionals.h>

#else
#  define ASH_OS_APPLE 0
#endif

#if defined(__APPLE__) && defined(__MACH__)        // Mac OS X
#  define ASH_OS_MACOS 1
#else
#  define ASH_OS_MACOS 0
#endif

#if defined(__wasi__)        // WebAssembly System Interface
#  define ASH_OS_WASI 1
#else
#  define ASH_OS_WASI 0
#endif

#if defined(__CYGWIN__)        // Cygwin environment
#  define ASH_OS_CYGWIN 1
#else
#  define ASH_OS_CYGWIN 0
#endif

#if defined(__Fuchsia__)        // Fuchsia
#  define ASH_OS_FUCHSIA 1
#else
#  define ASH_OS_FUCHSIA 0
#endif

/*********************** ARCHITECTURES ***********************/

#if defined(__i386__) || defined(__i386) || defined(_X86_) || \
    defined(_M_IX86) || defined(_M_I86)        // X86
#  define ASH_ARCH_X86 1
#else
#  define ASH_ARCH_X86 0
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(_M_AMD64) || defined(__amd64) ||                       \
    defined(__amd64__)        // X86_64
#  define ASH_ARCH_X86_64 1
#else
#  define ASH_ARCH_X86_64 0
#endif

#if defined(__arm__) || defined(_M_ARM)        // ARM
#  define ASH_ARCH_ARM32 1
#else
#  define ASH_ARCH_ARM32 0
#endif

#if defined(__aarch64__)        // ARM64
#  define ASH_ARCH_ARM64 1
#else
#  define ASH_ARCH_ARM64 0
#endif

#if defined(__XTENSA__)        // Xtensa
#  define ASH_ARCH_XTENSA 1
#else
#  define ASH_ARCH_XTENSA 0
#endif

#if defined(__mips__) || defined(__mips) || defined(mips) || \
    defined(__MIPS__)        // MIPS
#  define ASH_ARCH_MIPS 1
#else
#  define ASH_ARCH_MIPS 0
#endif

#if defined(__riscv) || defined(__riscv__)        // RISC-V
#  define ASH_ARCH_RISCV 1
#else
#  define ASH_ARCH_RISCV 0
#endif

/************ FEATURE AND LIBRARY REQUIREMENTS ************/

#if defined __has_builtin
#  define ASH_HAS_BUILTIN(feature) __has_builtin(__builtin_##feature)
#else
#  define ASH_HAS_BUILTIN(feature) 0
#endif

/*********************** BINARY FORMATS ***********************/

#if defined(__wasm__)        // Web Assembly
#  define ASH_BINARY_WASW 1
#else
#  define ASH_BINARY_WASW 0
#endif

#if defined(__ELF__)        // Executable and Linkable Formats
#  define ASH_BINARY_ELF 1
#else
#  define ASH_BINARY_ELF 0
#endif

#if ASH_CFG(OS, WINDOWS)        // Windows Portable Executable
#  define ASH_BINARY_EXE 1
#else
#  define ASH_BINARY_EXE 0
#endif

/*********************** TOOLCHAINS ***********************/

#if defined(__llvm__)
#  define ASH_TOOLCHAIN_LLVM 1
#else
#  define ASH_TOOLCHAIN_LLVM 0
#endif

/*********************** SHARED LIBRARY (DLL) SUPPORT ***********************/

#if ASH_CFG(OS, WINDOWS) || ASH_CFG(OS, CYGWIN)
// symbols are hidden by default on windows DLLs
#  define ASH_DLL_IMPORT __declspec(dllimport)
#  define ASH_DLL_EXPORT __declspec(dllexport)
#  define ASH_DLL_HIDDEN
#else
#  if ASH_CFG(COMPILER, GNUC)
// symbols are visible by default on GNUC DLLs
#    define ASH_DLL_IMPORT __attribute__((visibility("default")))
#    define ASH_DLL_EXPORT __attribute__((visibility("default")))
#    define ASH_DLL_HIDDEN __attribute__((visibility("hidden")))
#  else
#    define ASH_DLL_IMPORT
#    define ASH_DLL_EXPORT
#    define ASH_DLL_HIDDEN
#  endif
#endif

#define ASH_C_LINKAGE extern "C"

/*********************** INLINING MACROS ***********************/

// also used for hiding static variables and hookable functions that should not
// be touched but should reside in the binary
// GNUC doesn't mean GCC!, it's also present in clang
#if ASH_CFG(COMPILER, GNUC)
#  define ASH_FORCE_INLINE __attribute__((always_inline)) inline
#else
#  if ASH_CFG(COMPILER, MSVC)
#    define ASH_FORCE_INLINE __forceinline
#  else
#    if ASH_CFG(COMPILER, NVCC)
#      define ASH_FORCE_INLINE __forceinline__ inline
#    else
#      define ASH_FORCE_INLINE inline
#    endif
#  endif
#endif

#if ASH_CFG(COMPILER, GNUC) || ASH_CFG(COMPILER, CLANG) || \
    ASH_CFG(COMPILER, NVCC)
#  define ASH_RESTRICT __restrict__
#else
#  if ASH_CFG(COMPILER, MSVC)
#    define ASH_RESTRICT __restrict
#  else
#    define ASH_RESTRICT
#  endif
#endif

#if ASH_CFG(COMPILER, CLANG)
#  define ASH_TAILCALL [[clang::musttail]]
#else
#  define ASH_TAILCALL
#endif
