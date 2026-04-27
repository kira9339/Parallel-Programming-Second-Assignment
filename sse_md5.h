#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <xmmintrin.h>
#include <emmintrin.h>

using namespace std;

// 定义了Byte，便于使用
typedef unsigned char Byte;
// 定义了32比特
typedef unsigned int bit32;

// MD5 常量
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

void MD5Hash(string input, bit32* state);

void MD5Hash_SSE_4(const std::vector<std::string>& batch, uint32_t* results);


#define SSE_F(x, y, z) _mm_or_si128(_mm_and_si128(x, y), _mm_andnot_si128(x, z))
#define SSE_G(x, y, z) _mm_or_si128(_mm_and_si128(x, z), _mm_andnot_si128(z, y))
#define SSE_H(x, y, z) _mm_xor_si128(x, _mm_xor_si128(y, z))
#define SSE_I(x, y, z) _mm_xor_si128(y, _mm_or_si128(x, _mm_xor_si128(z, _mm_set1_epi32(0xFFFFFFFF))))

#define ROTATELEFT_SSE(num, n) \
    _mm_or_si128(_mm_slli_epi32(num, n), _mm_srli_epi32(num, 32 - (n)))

#define FF_SSE(a, b, c, d, x, s, ac) { \
    a = _mm_add_epi32(a, _mm_add_epi32(SSE_F(b, c, d), _mm_add_epi32(x, _mm_set1_epi32(ac)))); \
    a = ROTATELEFT_SSE(a, s); \
    a = _mm_add_epi32(a, b); \
}

#define GG_SSE(a, b, c, d, x, s, ac) { \
    a = _mm_add_epi32(a, _mm_add_epi32(SSE_G(b, c, d), _mm_add_epi32(x, _mm_set1_epi32(ac)))); \
    a = ROTATELEFT_SSE(a, s); \
    a = _mm_add_epi32(a, b); \
}

#define HH_SSE(a, b, c, d, x, s, ac) { \
    a = _mm_add_epi32(a, _mm_add_epi32(SSE_H(b, c, d), _mm_add_epi32(x, _mm_set1_epi32(ac)))); \
    a = ROTATELEFT_SSE(a, s); \
    a = _mm_add_epi32(a, b); \
}

#define II_SSE(a, b, c, d, x, s, ac) { \
    a = _mm_add_epi32(a, _mm_add_epi32(SSE_I(b, c, d), _mm_add_epi32(x, _mm_set1_epi32(ac)))); \
    a = ROTATELEFT_SSE(a, s); \
    a = _mm_add_epi32(a, b); \
}
