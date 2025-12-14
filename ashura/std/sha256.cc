#include "ashura/std/sha256.h"
#include <string.h>

namespace ash
{

ISha256State::ISha256State() :
  state_{0x6a09'e667ul, 0xbb67'ae85ul, 0x3c6e'f372ul, 0xa54f'f53aul,
         0x510e'527ful, 0x9b05'688cul, 0x1f83'd9abul, 0x5be0'cd19ul},
  size_{0},
  offset_{0},
  buf_{}
{
}

static inline u32 ror(u32 x, unsigned n)
{
  return (x >> n) | (x << (32 - n));
}

static inline u32 ch(u32 x, u32 y, u32 z)
{
  return z ^ (x & (y ^ z));
}

static inline u32 maj(u32 x, u32 y, u32 z)
{
  return ((x | y) & z) | (x & y);
}

static inline u32 sigma0(u32 x)
{
  return ror(x, 2) ^ ror(x, 13) ^ ror(x, 22);
}

static inline u32 sigma1(u32 x)
{
  return ror(x, 6) ^ ror(x, 11) ^ ror(x, 25);
}

static inline u32 gamma0(u32 x)
{
  return ror(x, 7) ^ ror(x, 18) ^ (x >> 3);
}

static inline u32 gamma1(u32 x)
{
  return ror(x, 17) ^ ror(x, 19) ^ (x >> 10);
}

static inline void put_be32(void * ptr, u32 value)
{
  unsigned char * p = (unsigned char *) ptr;
  p[0]              = (value >> 24) & 0xff;
  p[1]              = (value >> 16) & 0xff;
  p[2]              = (value >> 8) & 0xff;
  p[3]              = (value >> 0) & 0xff;
}

static inline u32 get_be32(const void * ptr)
{
  const unsigned char * p = (const unsigned char *) ptr;
  return (u32) p[0] << 24 | (u32) p[1] << 16 | (u32) p[2] << 8 |
         (u32) p[3] << 0;
}

void ISha256State::transform_(u8 const * buf)
{
  u32 S[8], W[64], t0, t1;
  int i;

  /* copy state into S */
  for (i = 0; i < 8; i++)
    S[i] = state_[i];

  /* copy the state into 512-bits into W[0..15] */
  for (i = 0; i < 16; i++, buf += sizeof(u32))
    W[i] = get_be32(buf);

  /* fill W[16..63] */
  for (i = 16; i < 64; i++)
    W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];

#define RND(a, b, c, d, e, f, g, h, i, ki)      \
  t0 = h + sigma1(e) + ch(e, f, g) + ki + W[i]; \
  t1 = sigma0(a) + maj(a, b, c);                \
  d += t0;                                      \
  h = t0 + t1;

  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 0, 0x428a'2f98);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 1, 0x7137'4491);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 2, 0xb5c0'fbcf);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 3, 0xe9b5'dba5);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 4, 0x3956'c25b);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 5, 0x59f1'11f1);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 6, 0x923f'82a4);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 7, 0xab1c'5ed5);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 8, 0xd807'aa98);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 9, 0x1283'5b01);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 10, 0x2431'85be);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 11, 0x550c'7dc3);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 12, 0x72be'5d74);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 13, 0x80de'b1fe);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 14, 0x9bdc'06a7);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 15, 0xc19b'f174);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 16, 0xe49b'69c1);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 17, 0xefbe'4786);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 18, 0x0fc1'9dc6);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 19, 0x240c'a1cc);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 20, 0x2de9'2c6f);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 21, 0x4a74'84aa);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 22, 0x5cb0'a9dc);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 23, 0x76f9'88da);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 24, 0x983e'5152);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 25, 0xa831'c66d);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 26, 0xb003'27c8);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 27, 0xbf59'7fc7);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 28, 0xc6e0'0bf3);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 29, 0xd5a7'9147);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 30, 0x06ca'6351);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 31, 0x1429'2967);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 32, 0x27b7'0a85);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 33, 0x2e1b'2138);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 34, 0x4d2c'6dfc);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 35, 0x5338'0d13);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 36, 0x650a'7354);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 37, 0x766a'0abb);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 38, 0x81c2'c92e);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 39, 0x9272'2c85);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 40, 0xa2bf'e8a1);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 41, 0xa81a'664b);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 42, 0xc24b'8b70);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 43, 0xc76c'51a3);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 44, 0xd192'e819);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 45, 0xd699'0624);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 46, 0xf40e'3585);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 47, 0x106a'a070);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 48, 0x19a4'c116);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 49, 0x1e37'6c08);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 50, 0x2748'774c);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 51, 0x34b0'bcb5);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 52, 0x391c'0cb3);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 53, 0x4ed8'aa4a);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 54, 0x5b9c'ca4f);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 55, 0x682e'6ff3);
  RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], 56, 0x748f'82ee);
  RND(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], 57, 0x78a5'636f);
  RND(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], 58, 0x84c8'7814);
  RND(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], 59, 0x8cc7'0208);
  RND(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], 60, 0x90be'fffa);
  RND(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], 61, 0xa450'6ceb);
  RND(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], 62, 0xbef9'a3f7);
  RND(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], 63, 0xc671'78f2);

  for (i = 0; i < 8; i++)
    state_[i] += S[i];
}

void ISha256State::update(Span<u8 const> span)
{
  u32  len_buf = size_ & 63;
  auto data    = span.data();
  auto len     = span.size();

  size_ += len;

  /* Read the data into buf and process blocks as they get full */
  if (len_buf)
  {
    u32 left = 64 - len_buf;
    if (len < left)
      left = len;
    memcpy(len_buf + buf_, data, left);
    len_buf = (len_buf + left) & 63;
    len -= left;
    data = (data + left);
    if (len_buf)
      return;
    transform_(buf_);
  }
  while (len >= 64)
  {
    transform_(data);
    data = data + 64;
    len -= 64;
  }
  if (len)
    memcpy(buf_, data, len);
}

Sha256 ISha256State::finalize()
{
  static const u8 pad[64] = {0x80};
  u32             padlen[2];
  int             i;

  /* Pad with a binary 1 (ie 0x80), then zeroes, then length */
  padlen[0] = __builtin_bswap32((u32) (size_ >> 29));
  padlen[1] = __builtin_bswap32((u32) (size_ << 3));

  i = size_ & 63;
  update(Span{pad, (usize) (1 + (63 & (55 - i)))});
  update(Span{padlen}.as_u8());

  Sha256 out;
  u8 *   digest = out.data();

  /* copy output */
  for (i = 0; i < 8; i++, digest += sizeof(u32))
    put_be32(digest, state_[i]);
  return out;
}

}    // namespace ash
