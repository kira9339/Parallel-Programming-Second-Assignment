#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <immintrin.h>

using namespace std;

// 定义了Byte，便于使用
typedef unsigned char Byte;
// 定义了32比特
typedef unsigned int bit32;

// MD5的一系列参数。参数是固定的，其实你不需要看懂这些
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

// 串行 MD5 函数
void MD5Hash(string input, bit32* state);

// AVX2 8 路并行 MD5 函数
void MD5Hash_AVX2_8(const std::vector<std::string>& batch, uint32_t* results);

/**
 * @Basic MD5 functions.
 *
 * @param there bit32.
 *
 * @return one bit32.
 */
 // 定义了一系列MD5中的具体函数
 // 这四个计算函数是需要你进行SIMD并行化的
 // 可以看到，FGHI四个函数都涉及一系列位运算，在数据上是对齐的，非常容易实现SIMD的并行化
#define AVX2_F(x, y, z) _mm256_or_si256(_mm256_and_si256(x, y), _mm256_andnot_si256(x, z))
#define AVX2_G(x, y, z) _mm256_or_si256(_mm256_and_si256(x, z), _mm256_andnot_si256(z, y))
#define AVX2_H(x, y, z) _mm256_xor_si256(x, _mm256_xor_si256(y, z))
#define AVX2_I(x, y, z) _mm256_xor_si256(y, _mm256_or_si256(x, _mm256_xor_si256(z, _mm256_set1_epi32(0xFFFFFFFF))))




/**
 * @Rotate Left.
 *
 * @param {num} the raw number.
 *
 * @param {n} rotate left n.
 *
 * @return the number after rotated left.
 */
 // 定义了一系列MD5中的具体函数
 // 这五个计算函数（ROTATELEFT/FF/GG/HH/II）和之前的FGHI一样，都是需要你进行SIMD并行化的
 // 但是你需要注意的是#define的功能及其效果，可以发现这里的FGHI是没有返回值的，为什么呢？你可以查询#define的含义和用法
#define ROTATELEFT_AVX2(num, n) \
    _mm256_or_si256(_mm256_slli_epi32(num, n), _mm256_srli_epi32(num, 32 - (n)))

#define FF_AVX2(a, b, c, d, x, s, ac) { \
    a = _mm256_add_epi32(a, _mm256_add_epi32(AVX2_F(b, c, d), _mm256_add_epi32(x, _mm256_set1_epi32(ac)))); \
    a = ROTATELEFT_AVX2(a, s); \
    a = _mm256_add_epi32(a, b); \
}

#define GG_AVX2(a, b, c, d, x, s, ac) { \
    a = _mm256_add_epi32(a, _mm256_add_epi32(AVX2_G(b, c, d), _mm256_add_epi32(x, _mm256_set1_epi32(ac)))); \
    a = ROTATELEFT_AVX2(a, s); \
    a = _mm256_add_epi32(a, b); \
}

#define HH_AVX2(a, b, c, d, x, s, ac) { \
    a = _mm256_add_epi32(a, _mm256_add_epi32(AVX2_H(b, c, d), _mm256_add_epi32(x, _mm256_set1_epi32(ac)))); \
    a = ROTATELEFT_AVX2(a, s); \
    a = _mm256_add_epi32(a, b); \
}

#define II_AVX2(a, b, c, d, x, s, ac) { \
    a = _mm256_add_epi32(a, _mm256_add_epi32(AVX2_I(b, c, d), _mm256_add_epi32(x, _mm256_set1_epi32(ac)))); \
    a = ROTATELEFT_AVX2(a, s); \
    a = _mm256_add_epi32(a, b); \
}