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
  switch (err)
  {
    case IoErr::None:
      return "None"_span;
    case IoErr::PermissionDenied:
      return "PermissionDenied"_span;
    case IoErr::AddressInUse:
      return "AddressInUse"_span;
    case IoErr::Again:
      return "Again"_span;
    case IoErr::Already:
      return "Already"_span;
    case IoErr::BadFileDescriptor:
      return "BadFileDescriptor"_span;
    case IoErr::Busy:
      return "Busy"_span;
    case IoErr::Canceled:
      return "Canceled"_span;
    case IoErr::DeadlockAvoided:
      return "DeadlockAvoided"_span;
    case IoErr::Exists:
      return "Exists"_span;
    case IoErr::BadAddress:
      return "BadAddress"_span;
    case IoErr::FileTooBig:
      return "FileTooBig"_span;
    case IoErr::IllegalCharSequence:
      return "IllegalCharSequence"_span;
    case IoErr::OpInProgress:
      return "OpInProgress"_span;
    case IoErr::SysCallInterrupted:
      return "SysCallInterrupted"_span;
    case IoErr::InvalidArg:
      return "InvalidArg"_span;
    case IoErr::IOErr:
      return "IOErr"_span;
    case IoErr::IsDirectory:
      return "IsDirectory"_span;
    case IoErr::TooManySymLinks:
      return "TooManySymLinks"_span;
    case IoErr::TooManyOpenFiles:
      return "TooManyOpenFiles"_span;
    case IoErr::TooManyLinks:
      return "TooManyLinks"_span;
    case IoErr::MsgTooLong:
      return "MsgTooLong"_span;
    case IoErr::FileNameTooLong:
      return "FileNameTooLong"_span;
    case IoErr::TooManyOpenSysFiles:
      return "TooManyOpenSysFiles"_span;
    case IoErr::NoBufferSpace:
      return "NoBufferSpace"_span;
    case IoErr::NoData:
      return "NoData"_span;
    case IoErr::InvalidDev:
      return "InvalidDev"_span;
    case IoErr::InvalidFileOrDir:
      return "InvalidFileOrDir"_span;
    case IoErr::ExecFormat:
      return "ExecFormat"_span;
    case IoErr::NoLocksAvailable:
      return "NoLocksAvailable"_span;
    case IoErr::NoLink:
      return "NoLink"_span;
    case IoErr::OutOfMemory:
      return "OutOfMemory"_span;
    case IoErr::OutOfSpace:
      return "OutOfSpace"_span;
    case IoErr::OutOfStreamRes:
      return "OutOfStreamRes"_span;
    case IoErr::NotStream:
      return "NotStream"_span;
    case IoErr::UnImplemented:
      return "UnImplemented"_span;
    case IoErr::NotDir:
      return "NotDir"_span;
    case IoErr::DirectoryNotEmpty:
      return "DirectoryNotEmpty"_span;
    case IoErr::Unsupported:
      return "Unsupported"_span;
    case IoErr::InvalidDeviceOrAddr:
      return "InvalidDeviceOrAddr"_span;
    case IoErr::OpUnsupported:
      return "OpUnsupported"_span;
    case IoErr::Overflow:
      return "Overflow"_span;
    case IoErr::OwnerDead:
      return "OwnerDead"_span;
    case IoErr::UnpermittedOp:
      return "UnpermittedOp"_span;
    case IoErr::BrokenPipe:
      return "BrokenPipe"_span;
    case IoErr::OutOfRange:
      return "OutOfRange"_span;
    case IoErr::ReadOnlyFileSys:
      return "ReadOnlyFileSys"_span;
    case IoErr::IllegalSeek:
      return "IllegalSeek"_span;
    case IoErr::NoSuchProcess:
      return "NoSuchProcess"_span;
    case IoErr::TextFileBusy:
      return "TextFileBusy"_span;
    case IoErr::TemporarilyUnavailable:
      return "TemporarilyUnavailable"_span;
    default:
      return "Unidentified Filesystem Error"_span;
  }
}

namespace fmt
{

inline bool push(Context const &ctx, Spec const &spec, IoErr const &err)
{
  return push(ctx, spec, to_string(err));
}

}        // namespace fmt

Result<Void, IoErr> read_file(Span<char const> path, Vec<u8> &buff);

inline Result<> path_append(Vec<char> &path, Span<char const> tail)
{
  if (!path.is_empty() && path[path.size() - 1] != '/' &&
      path[path.size() - 1] != '\\')
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