/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

/// https://datatracker.ietf.org/doc/html/rfc3986

/// URI syntax
///
/// URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
/// hier-part = "//" authority path-abempty
///           / path-absolute
///           / path-rootless
///           / path-empty
///
///
/// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
///
/// authority = [ userinfo "@" ] host [ ":" port ]
///
/// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
///
/// host = IP-literal / IPv4address / reg-name
///
/// IP-literal = "[" ( IPv6address / IPvFuture ) "]"
/// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
/// IPv6address =                            6( h16 ":" ) ls32
///             /                       "::" 5( h16 ":" ) ls32
///             / [               h16 ] "::" 4( h16 ":" ) ls32
///             / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
///             / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
///             / [ *3( h16 ":" ) h16 ] "::"    1( h16 ":" ) ls32
///             / [ *4( h16 ":" ) h16 ] "::"              ls32
///             / [ *5( h16 ":" ) h16 ] "::"              h16
///             / [ *6( h16 ":" ) h16 ] "::"
///
/// ls32 = ( h16 ":" h16 ) / IPv4address
/// h16 = 1*4HEXDIG
///
/// IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
/// dec-octet = DIGIT                 ; 0-9
///            / %x31-39 DIGIT         ; 10-99
///            / "1" 2DIGIT            ; 100-199
///            / "2" %x30-34 DIGIT    ; 200-249
///            / "25" %x30-35         ; 250-255
///
/// reg-name = *( unreserved / pct-encoded / sub-delims )
///
///
/// port = *DIGIT
///
///
/// path = path-abempty    ; begins with "/" or is empty
///      / path-absolute   ; begins with "/" but not "//"
///      / path-noscheme   ; begins with a non-colon segment
///      / path-rootless   ; begins with a segment
///      / path-empty      ; zero characters
///
/// path-abempty  = *( "/" segment )
/// path-absolute = "/" [ segment-nz *( "/" segment ) ]
/// path-noscheme = segment-nz-nc *( "/" segment )
/// path-rootless = segment-nz *( "/" segment )
/// path-empty    = 0<pchar>
///
/// segment       = *pchar
/// segment-nz    = 1*pchar
/// segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
///                 ; non-zero-length segment without any colon ":"
/// pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
///
///
/// query = *( pchar / "/" / "?" )
///
/// fragment = *( pchar / "/" / "?" )
struct UriView
{
    Str         scheme    = {};
    Str         hier_part = {};
    Option<Str> queries   = none;
    Option<Str> fragments = none;

    static Result<UriView> parse(Str uri)
    {
        auto iter  = 0uz;
        auto size  = uri.size();
        auto p_uri = uri.pbegin();

        auto scheme_begin = iter;

        while (iter < size && p_uri[iter] != ':')
        {
            iter++;
        }

        auto scheme_end = iter;
        auto scheme     = uri.slice(Slice::offsets(scheme_begin, scheme_end));

        if (iter >= size)
        {
            return Err{};
        }

        iter++;

        if (iter >= size)
        {
            return Err{};
        }

        auto hier_part_begin = iter;

        while (iter < size && p_uri[iter] != '?' && p_uri[iter] != '#')
        {
            iter++;
        }

        auto hier_part_end = iter;

        auto hier_part =
          uri.slice(Slice::offsets(hier_part_begin, hier_part_end));

        if (iter >= size)
        {
            return Ok{
              UriView{.scheme = scheme, .hier_part = hier_part}
            };
        }
        else if (p_uri[iter] == '?')
        {
            iter++;

            auto queries_begin = iter;

            while (iter < size && p_uri[iter] != '#')
            {
                iter++;
            }

            auto queries_end = iter;
            auto queries =
              uri.slice(Slice::offsets(queries_begin, queries_end));

            if (iter >= size)
            {
                return Ok{
                  UriView{.scheme    = scheme,
                          .hier_part = hier_part,
                          .queries   = queries,
                          .fragments = none}
                };
            }

            if (p_uri[iter] == '#')
            {
                iter++;

                auto fragments_begin = iter;
                auto fragments_end   = size;
                auto fragments =
                  uri.slice(Slice::offsets(fragments_begin, fragments_end));

                return Ok{
                  UriView{.scheme    = scheme,
                          .hier_part = hier_part,
                          .queries   = queries,
                          .fragments = fragments}
                };
            }
        }
        else if (p_uri[iter] == '#')
        {
            iter++;

            auto fragments_begin = iter;
            auto fragments_end   = size;
            auto fragments =
              uri.slice(Slice::offsets(fragments_begin, fragments_end));

            return Ok{
              UriView{.scheme    = scheme,
                      .hier_part = hier_part,
                      .queries   = none,
                      .fragments = fragments}
            };
        }
        else
        {
            ASH_UNREACHABLE;
        }

        ASH_UNREACHABLE;
    }
};

}    // namespace ash
