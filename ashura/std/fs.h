/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class [[nodiscard]] IoError : i32
{
  None                   = 0,
  PermissionDenied       = EACCES,
  AddressInUse           = EADDRINUSE,
  Again                  = EAGAIN,
  Already                = EALREADY,
  BadFileDescriptor      = EBADF,
  Busy                   = EBUSY,
  Canceled               = ECANCELED,
  DeadlockAvoided        = EDEADLK,
  Exists                 = EEXIST,
  BadAddress             = EFAULT,
  FileTooBig             = EFBIG,
  IllegalCharSequence    = EILSEQ,
  OpInProgress           = EINPROGRESS,
  SysCallInterrupted     = EINTR,
  InvalidArg             = EINVAL,
  IOErr                  = EIO,
  IsDirectory            = EISDIR,
  TooManySymLinks        = ELOOP,
  TooManyOpenFiles       = EMFILE,
  TooManyLinks           = EMLINK,
  MsgTooLong             = EMSGSIZE,
  FileNameTooLong        = ENAMETOOLONG,
  TooManyOpenSysFiles    = ENFILE,
  NoBufferSpace          = ENOBUFS,
  NoData                 = ENODATA,
  InvalidDev             = ENODEV,
  InvalidFileOrDir       = ENOENT,
  ExecFormat             = ENOEXEC,
  NoLocksAvailable       = ENOLCK,
  NoLink                 = ENOLINK,
  OutOfMemory            = ENOMEM,
  OutOfSpace             = ENOSPC,
  OutOfStreamRes         = ENOSR,
  NotStream              = ENOSTR,
  UnImplemented          = ENOSYS,
  NotDir                 = ENOTDIR,
  DirectoryNotEmpty      = ENOTEMPTY,
  Unsupported            = ENOTSUP,
  InvalidDeviceOrAddr    = ENXIO,
  OpUnsupported          = EOPNOTSUPP,
  Overflow               = EOVERFLOW,
  OwnerDead              = EOWNERDEAD,
  UnpermittedOp          = EPERM,
  BrokenPipe             = EPIPE,
  OutOfRange             = ERANGE,
  ReadOnlyFileSys        = EROFS,
  IllegalSeek            = ESPIPE,
  NoSuchProcess          = ESRCH,
  TextFileBusy           = ETXTBSY,
  TemporarilyUnavailable = EWOULDBLOCK
};

IoError read_file(Span<char const> path, Vec<u8> &buff);

inline bool path_append(Vec<char> &path, Span<char const> tail)
{
  if (!path.is_empty() && path[path.size() - 1] != '/' &&
      path[path.size() - 1] != '\\')
  {
    if (!path.push('/'))
    {
      return false;
    }
  }
  if (!path.extend_copy(tail))
  {
    return false;
  }
  return true;
}

}        // namespace ash