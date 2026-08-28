/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/result.hpp"
#include "ashura/std/types.hpp"
#include "ashura/std/vec.hpp"

namespace ash
{

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

constexpr Str to_str(IoErr err)
{
    if (err == IoErr::None)
    {
        return "None"_s;
    }
    else if (err == IoErr::PermissionDenied)
    {
        return "PermissionDenied"_s;
    }
    else if (err == IoErr::AddressInUse)
    {
        return "AddressInUse"_s;
    }
    else if (err == IoErr::Again)
    {
        return "Again"_s;
    }
    else if (err == IoErr::Already)
    {
        return "Already"_s;
    }
    else if (err == IoErr::BadFileDescriptor)
    {
        return "BadFileDescriptor"_s;
    }
    else if (err == IoErr::Busy)
    {
        return "Busy"_s;
    }
    else if (err == IoErr::Canceled)
    {
        return "Canceled"_s;
    }
    else if (err == IoErr::DeadlockAvoided)
    {
        return "DeadlockAvoided"_s;
    }
    else if (err == IoErr::Exists)
    {
        return "Exists"_s;
    }
    else if (err == IoErr::BadAddress)
    {
        return "BadAddress"_s;
    }
    else if (err == IoErr::FileTooBig)
    {
        return "FileTooBig"_s;
    }
    else if (err == IoErr::IllegalCharSequence)
    {
        return "IllegalCharSequence"_s;
    }
    else if (err == IoErr::OpInProgress)
    {
        return "OpInProgress"_s;
    }
    else if (err == IoErr::SysCallInterrupted)
    {
        return "SysCallInterrupted"_s;
    }
    else if (err == IoErr::InvalidArg)
    {
        return "InvalidArg"_s;
    }
    else if (err == IoErr::IOErr)
    {
        return "IOErr"_s;
    }
    else if (err == IoErr::IsDirectory)
    {
        return "IsDirectory"_s;
    }
    else if (err == IoErr::TooManySymLinks)
    {
        return "TooManySymLinks"_s;
    }
    else if (err == IoErr::TooManyOpenFiles)
    {
        return "TooManyOpenFiles"_s;
    }
    else if (err == IoErr::TooManyLinks)
    {
        return "TooManyLinks"_s;
    }
    else if (err == IoErr::MsgTooLong)
    {
        return "MsgTooLong"_s;
    }
    else if (err == IoErr::FileNameTooLong)
    {
        return "FileNameTooLong"_s;
    }
    else if (err == IoErr::TooManyOpenSysFiles)
    {
        return "TooManyOpenSysFiles"_s;
    }
    else if (err == IoErr::NoBufferSpace)
    {
        return "NoBufferSpace"_s;
    }
    else if (err == IoErr::NoData)
    {
        return "NoData"_s;
    }
    else if (err == IoErr::InvalidDev)
    {
        return "InvalidDev"_s;
    }
    else if (err == IoErr::InvalidFileOrDir)
    {
        return "InvalidFileOrDir"_s;
    }
    else if (err == IoErr::ExecFormat)
    {
        return "ExecFormat"_s;
    }
    else if (err == IoErr::NoLocksAvailable)
    {
        return "NoLocksAvailable"_s;
    }
    else if (err == IoErr::NoLink)
    {
        return "NoLink"_s;
    }
    else if (err == IoErr::OutOfMemory)
    {
        return "OutOfMemory"_s;
    }
    else if (err == IoErr::OutOfSpace)
    {
        return "OutOfSpace"_s;
    }
    else if (err == IoErr::OutOfStreamRes)
    {
        return "OutOfStreamRes"_s;
    }
    else if (err == IoErr::NotStream)
    {
        return "NotStream"_s;
    }
    else if (err == IoErr::UnImplemented)
    {
        return "UnImplemented"_s;
    }
    else if (err == IoErr::NotDir)
    {
        return "NotDir"_s;
    }
    else if (err == IoErr::DirectoryNotEmpty)
    {
        return "DirectoryNotEmpty"_s;
    }
    else if (err == IoErr::Unsupported)
    {
        return "Unsupported"_s;
    }
    else if (err == IoErr::InvalidDeviceOrAddr)
    {
        return "InvalidDeviceOrAddr"_s;
    }
    else if (err == IoErr::OpUnsupported)
    {
        return "OpUnsupported"_s;
    }
    else if (err == IoErr::Overflow)
    {
        return "Overflow"_s;
    }
    else if (err == IoErr::OwnerDead)
    {
        return "OwnerDead"_s;
    }
    else if (err == IoErr::UnpermittedOp)
    {
        return "UnpermittedOp"_s;
    }
    else if (err == IoErr::BrokenPipe)
    {
        return "BrokenPipe"_s;
    }
    else if (err == IoErr::OutOfRange)
    {
        return "OutOfRange"_s;
    }
    else if (err == IoErr::ReadOnlyFileSys)
    {
        return "ReadOnlyFileSys"_s;
    }
    else if (err == IoErr::IllegalSeek)
    {
        return "IllegalSeek"_s;
    }
    else if (err == IoErr::NoSuchProcess)
    {
        return "NoSuchProcess"_s;
    }
    else if (err == IoErr::TextFileBusy)
    {
        return "TextFileBusy"_s;
    }
    else if (err == IoErr::TemporarilyUnavailable)
    {
        return "TemporarilyUnavailable"_s;
    }
    else
    {
        return "Unidentified Filesystem Error"_s;
    }
}

inline void format(fmt::Sink sink, fmt::Spec spec, IoErr const & err)
{
    return format(sink, spec, to_str(err));
}

inline Result<> path_join(Str base, Str ext, Vec<char> & out)
{
    auto max_size     = base.size() + ext.size() + 1;
    auto initial_size = out.size();

    if (!out.extend_uninit(max_size))
    {
        return Err{};
    }

    usize pos = initial_size;
    mem::copy(base, out.view().slice(pos));

    pos += base.size();

    if (!base.is_empty() && base.last() != '/' && base.last() != '\\')
    {
        out[pos] = '/';
        pos++;
    }

    mem::copy(ext, out.view().slice(pos));

    pos += ext.size();

    out.resize_uninit(pos).unwrap();

    return Ok{};
}

Result<Void, IoErr> read_file(Str path, Vec<u8> & buff, Allocator scratch_allocator);

Result<Void, IoErr> write_to_file(Str path, Span<u8 const> buff, bool append,
                                  Allocator scratch_allocator);

}    // namespace ash
