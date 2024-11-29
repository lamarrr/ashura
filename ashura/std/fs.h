/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

static constexpr usize MAX_PATH_SIZE = 256;

enum class [[nodiscard]] IoErr : i32
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

constexpr Span<char const> to_string(IoErr err)
{
  if (err == IoErr::None)
  {
    return "None"_str;
  }
  else if (err == IoErr::PermissionDenied)
  {
    return "PermissionDenied"_str;
  }
  else if (err == IoErr::AddressInUse)
  {
    return "AddressInUse"_str;
  }
  else if (err == IoErr::Again)
  {
    return "Again"_str;
  }
  else if (err == IoErr::Already)
  {
    return "Already"_str;
  }
  else if (err == IoErr::BadFileDescriptor)
  {
    return "BadFileDescriptor"_str;
  }
  else if (err == IoErr::Busy)
  {
    return "Busy"_str;
  }
  else if (err == IoErr::Canceled)
  {
    return "Canceled"_str;
  }
  else if (err == IoErr::DeadlockAvoided)
  {
    return "DeadlockAvoided"_str;
  }
  else if (err == IoErr::Exists)
  {
    return "Exists"_str;
  }
  else if (err == IoErr::BadAddress)
  {
    return "BadAddress"_str;
  }
  else if (err == IoErr::FileTooBig)
  {
    return "FileTooBig"_str;
  }
  else if (err == IoErr::IllegalCharSequence)
  {
    return "IllegalCharSequence"_str;
  }
  else if (err == IoErr::OpInProgress)
  {
    return "OpInProgress"_str;
  }
  else if (err == IoErr::SysCallInterrupted)
  {
    return "SysCallInterrupted"_str;
  }
  else if (err == IoErr::InvalidArg)
  {
    return "InvalidArg"_str;
  }
  else if (err == IoErr::IOErr)
  {
    return "IOErr"_str;
  }
  else if (err == IoErr::IsDirectory)
  {
    return "IsDirectory"_str;
  }
  else if (err == IoErr::TooManySymLinks)
  {
    return "TooManySymLinks"_str;
  }
  else if (err == IoErr::TooManyOpenFiles)
  {
    return "TooManyOpenFiles"_str;
  }
  else if (err == IoErr::TooManyLinks)
  {
    return "TooManyLinks"_str;
  }
  else if (err == IoErr::MsgTooLong)
  {
    return "MsgTooLong"_str;
  }
  else if (err == IoErr::FileNameTooLong)
  {
    return "FileNameTooLong"_str;
  }
  else if (err == IoErr::TooManyOpenSysFiles)
  {
    return "TooManyOpenSysFiles"_str;
  }
  else if (err == IoErr::NoBufferSpace)
  {
    return "NoBufferSpace"_str;
  }
  else if (err == IoErr::NoData)
  {
    return "NoData"_str;
  }
  else if (err == IoErr::InvalidDev)
  {
    return "InvalidDev"_str;
  }
  else if (err == IoErr::InvalidFileOrDir)
  {
    return "InvalidFileOrDir"_str;
  }
  else if (err == IoErr::ExecFormat)
  {
    return "ExecFormat"_str;
  }
  else if (err == IoErr::NoLocksAvailable)
  {
    return "NoLocksAvailable"_str;
  }
  else if (err == IoErr::NoLink)
  {
    return "NoLink"_str;
  }
  else if (err == IoErr::OutOfMemory)
  {
    return "OutOfMemory"_str;
  }
  else if (err == IoErr::OutOfSpace)
  {
    return "OutOfSpace"_str;
  }
  else if (err == IoErr::OutOfStreamRes)
  {
    return "OutOfStreamRes"_str;
  }
  else if (err == IoErr::NotStream)
  {
    return "NotStream"_str;
  }
  else if (err == IoErr::UnImplemented)
  {
    return "UnImplemented"_str;
  }
  else if (err == IoErr::NotDir)
  {
    return "NotDir"_str;
  }
  else if (err == IoErr::DirectoryNotEmpty)
  {
    return "DirectoryNotEmpty"_str;
  }
  else if (err == IoErr::Unsupported)
  {
    return "Unsupported"_str;
  }
  else if (err == IoErr::InvalidDeviceOrAddr)
  {
    return "InvalidDeviceOrAddr"_str;
  }
  else if (err == IoErr::OpUnsupported)
  {
    return "OpUnsupported"_str;
  }
  else if (err == IoErr::Overflow)
  {
    return "Overflow"_str;
  }
  else if (err == IoErr::OwnerDead)
  {
    return "OwnerDead"_str;
  }
  else if (err == IoErr::UnpermittedOp)
  {
    return "UnpermittedOp"_str;
  }
  else if (err == IoErr::BrokenPipe)
  {
    return "BrokenPipe"_str;
  }
  else if (err == IoErr::OutOfRange)
  {
    return "OutOfRange"_str;
  }
  else if (err == IoErr::ReadOnlyFileSys)
  {
    return "ReadOnlyFileSys"_str;
  }
  else if (err == IoErr::IllegalSeek)
  {
    return "IllegalSeek"_str;
  }
  else if (err == IoErr::NoSuchProcess)
  {
    return "NoSuchProcess"_str;
  }
  else if (err == IoErr::TextFileBusy)
  {
    return "TextFileBusy"_str;
  }
  else if (err == IoErr::TemporarilyUnavailable)
  {
    return "TemporarilyUnavailable"_str;
  }
  else
  {
    return "Unidentified Filesystem Error"_str;
  }
}

namespace fmt
{

inline bool push(Context const & ctx, Spec const & spec, IoErr const & err)
{
  return push(ctx, spec, to_string(err));
}

}        // namespace fmt

Result<Void, IoErr> read_file(Span<char const> path, Vec<u8> & buff);

inline Result<> path_append(Vec<char> & path, Span<char const> tail)
{
  if (!path.is_empty() && path.last() != '/' && path.last() != '\\')
  {
    if (!path.push('/'))
    {
      return Err{};
    }
  }
  if (!path.extend_copy(tail))
  {
    return Err{};
  }
  return Ok{};
}

}        // namespace ash
