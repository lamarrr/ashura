/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"
#include "ashura/std/vec.h"

#if defined _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#else
#  include <pthread.h>
#  include <string.h>
#  include <unistd.h>
#endif

#ifdef __linux__
#  ifdef __ANDROID__
#    include <sys/types.h>
#  else
#    include <sys/syscall.h>
#  endif
#  include <fcntl.h>
#elif defined __FreeBSD__
#  include <sys/thr.h>
#elif defined __NetBSD__
#  include <lwp.h>
#elif defined __DragonFly__
#  include <sys/lwp.h>
#elif defined __QNX__
#  include <process.h>
#  include <sys/neutrino.h>
#endif

#include <inttypes.h>
#include <stdlib.h>

namespace ash
{

template <usize MaxNumChars>
auto to_c_str(Str s)
{
  InplaceVec<char, MaxNumChars + 1> buffer;
  buffer.append(s.slice(0, MaxNumChars)).unwrap();
  buffer.push('\0').unwrap();
  return buffer;
}

static thread_local InplaceVec<char, 256> thread_name_buffer{};

void set_thread_name(Str name_str)
{
#if defined _WIN32
  auto    name_c_str = to_c_str<255>(name_str);
  wchar_t buf[256];
  mbstowcs(buf, name_c_str.data(), 256);
  SetThreadDescription(GetCurrentThread(), buf);
#elif defined _GNU_SOURCE && !defined __EMSCRIPTEN__
  auto name_c_str = to_c_str<15>(name_str);
#  if defined __APPLE__
  pthread_setname_np(name_c_str.data());
#  else
  pthread_setname_np(pthread_self(), name_c_str.data());
#  endif
#elif defined __QNX__
  auto name_c_str = to_c_str<_NTO_THREAD_NAME_MAX>(name_str);
  pthread_setname_np(pthread_self(), name_c_str.data());
#endif

  thread_name_buffer.clear();
  thread_name_buffer.append(name_str.slice(0, 255)).unwrap();
}

Str get_thread_name()
{
  return thread_name_buffer;
}

}    // namespace ash
