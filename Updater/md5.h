// Minimal, self-contained MD5 (RFC 1321), header-only.
//
// Deliberately NOT reusing Client/md5.cpp's CMd5 class here: that file
// pulls in Client_PCH.h/StdAfx.h, which drags along a large chunk of the
// main client's dependency graph - overkill and risky for this small,
// standalone Updater binary. This is a plain, dependency-free
// reimplementation of the same well-known algorithm, kept in one file.
//
// Usage: Md5File(path, outHex33) fills outHex33 with a 32-char lowercase
// hex digest + null terminator, or returns false if the file can't be read.

#ifndef DARKEDEN_UPDATER_MD5_H
#define DARKEDEN_UPDATER_MD5_H

#include <cstdio>
#include <cstdint>
#include <cstring>

namespace md5_detail {

struct Md5Context {
	uint32_t state[4];
	uint64_t count; // bits processed
	unsigned char buffer[64];
};

inline uint32_t LeftRotate(uint32_t x, int c) { return (x << c) | (x >> (32 - c)); }

inline void Transform(uint32_t state[4], const unsigned char block[64])
{
	static const uint32_t K[64] = {
		0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
		0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
		0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
		0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
		0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
		0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
		0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
		0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
	};
	static const int S[64] = {
		7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
		5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
		4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
		6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
	};

	uint32_t M[16];
	for (int i = 0; i < 16; i++) {
		M[i] = (uint32_t)block[i*4] | ((uint32_t)block[i*4+1] << 8) |
			((uint32_t)block[i*4+2] << 16) | ((uint32_t)block[i*4+3] << 24);
	}

	uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

	for (int i = 0; i < 64; i++) {
		uint32_t f;
		int g;
		if (i < 16) { f = (b & c) | (~b & d); g = i; }
		else if (i < 32) { f = (d & b) | (~d & c); g = (5*i + 1) % 16; }
		else if (i < 48) { f = b ^ c ^ d; g = (3*i + 5) % 16; }
		else { f = c ^ (b | ~d); g = (7*i) % 16; }

		uint32_t temp = d;
		d = c;
		c = b;
		b = b + LeftRotate(a + f + K[i] + M[g], S[i]);
		a = temp;
	}

	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

inline void Md5Init(Md5Context& ctx)
{
	ctx.state[0] = 0x67452301;
	ctx.state[1] = 0xefcdab89;
	ctx.state[2] = 0x98badcfe;
	ctx.state[3] = 0x10325476;
	ctx.count = 0;
}

inline void Md5Update(Md5Context& ctx, const unsigned char* data, size_t len)
{
	size_t bufferUsed = (size_t)((ctx.count / 8) % 64);
	ctx.count += (uint64_t)len * 8;

	size_t i = 0;
	if (bufferUsed > 0) {
		size_t toCopy = 64 - bufferUsed;
		if (toCopy > len) toCopy = len;
		memcpy(ctx.buffer + bufferUsed, data, toCopy);
		if (bufferUsed + toCopy == 64) {
			Transform(ctx.state, ctx.buffer);
		}
		i = toCopy;
	}

	for (; i + 64 <= len; i += 64) {
		Transform(ctx.state, data + i);
	}

	if (i < len) {
		memcpy(ctx.buffer, data + i, len - i);
	}
}

inline void Md5Final(Md5Context& ctx, unsigned char digest[16])
{
	unsigned char padding[64] = { 0x80 };
	uint64_t bitCount = ctx.count;

	size_t bufferUsed = (size_t)((ctx.count / 8) % 64);
	size_t padLen = (bufferUsed < 56) ? (56 - bufferUsed) : (120 - bufferUsed);
	Md5Update(ctx, padding, padLen);

	unsigned char lengthBytes[8];
	for (int i = 0; i < 8; i++) {
		lengthBytes[i] = (unsigned char)((bitCount >> (8 * i)) & 0xff);
	}
	Md5Update(ctx, lengthBytes, 8);

	for (int i = 0; i < 4; i++) {
		digest[i*4]   = (unsigned char)(ctx.state[i] & 0xff);
		digest[i*4+1] = (unsigned char)((ctx.state[i] >> 8) & 0xff);
		digest[i*4+2] = (unsigned char)((ctx.state[i] >> 16) & 0xff);
		digest[i*4+3] = (unsigned char)((ctx.state[i] >> 24) & 0xff);
	}
}

} // namespace md5_detail

// Computes the MD5 of a file, streaming it in chunks. outHex must have
// room for 33 bytes (32 hex chars + null terminator). Returns false if
// the file couldn't be opened.
inline bool Md5File(const char* path, char outHex[33])
{
	FILE* f = fopen(path, "rb");
	if (!f) return false;

	md5_detail::Md5Context ctx;
	md5_detail::Md5Init(ctx);

	unsigned char buf[8192];
	size_t n;
	while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
		md5_detail::Md5Update(ctx, buf, n);
	}
	fclose(f);

	unsigned char digest[16];
	md5_detail::Md5Final(ctx, digest);

	static const char* hexChars = "0123456789abcdef";
	for (int i = 0; i < 16; i++) {
		outHex[i*2]   = hexChars[(digest[i] >> 4) & 0xf];
		outHex[i*2+1] = hexChars[digest[i] & 0xf];
	}
	outHex[32] = '\0';
	return true;
}

#endif // DARKEDEN_UPDATER_MD5_H
