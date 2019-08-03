/* $Id: blake.c 252 2011-06-07 17:55:14Z tp $ */
/*
 * BLAKE implementation.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2007-2010  Projet RNRT SAPHIR
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@cryptolog.com>
 */

//#if defined (__SSE4_2__)

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "blake-hash-4way.h"

#ifdef __cplusplus
extern "C"{
#endif

#if SPH_SMALL_FOOTPRINT && !defined SPH_SMALL_FOOTPRINT_BLAKE
#define SPH_SMALL_FOOTPRINT_BLAKE   1
#endif

#if SPH_SMALL_FOOTPRINT_BLAKE
#define SPH_COMPACT_BLAKE_32   1
#endif

#if SPH_64 && (SPH_SMALL_FOOTPRINT_BLAKE || !SPH_64_TRUE)
#define SPH_COMPACT_BLAKE_64   1
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4146)
#endif

// Blake-256

static const uint32_t IV256[8] =
{
	0x6A09E667, 0xBB67AE85,	0x3C6EF372, 0xA54FF53A,
	0x510E527F, 0x9B05688C,	0x1F83D9AB, 0x5BE0CD19
};

#if SPH_COMPACT_BLAKE_32 || SPH_COMPACT_BLAKE_64

// Blake-256 4 & 8 way, Blake-512 4 way

static const unsigned sigma[16][16] = {
	{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
	{ 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
	{  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
	{  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
	{  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 },
	{ 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 },
	{ 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 },
	{  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 },
	{ 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13,  0 },
	{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
	{ 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
	{  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
	{  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
	{  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 }
};

#endif

#define Z00   0
#define Z01   1
#define Z02   2
#define Z03   3
#define Z04   4
#define Z05   5
#define Z06   6
#define Z07   7
#define Z08   8
#define Z09   9
#define Z0A   A
#define Z0B   B
#define Z0C   C
#define Z0D   D
#define Z0E   E
#define Z0F   F

#define Z10   E
#define Z11   A
#define Z12   4
#define Z13   8
#define Z14   9
#define Z15   F
#define Z16   D
#define Z17   6
#define Z18   1
#define Z19   C
#define Z1A   0
#define Z1B   2
#define Z1C   B
#define Z1D   7
#define Z1E   5
#define Z1F   3

#define Z20   B
#define Z21   8
#define Z22   C
#define Z23   0
#define Z24   5
#define Z25   2
#define Z26   F
#define Z27   D
#define Z28   A
#define Z29   E
#define Z2A   3
#define Z2B   6
#define Z2C   7
#define Z2D   1
#define Z2E   9
#define Z2F   4

#define Z30   7
#define Z31   9
#define Z32   3
#define Z33   1
#define Z34   D
#define Z35   C
#define Z36   B
#define Z37   E
#define Z38   2
#define Z39   6
#define Z3A   5
#define Z3B   A
#define Z3C   4
#define Z3D   0
#define Z3E   F
#define Z3F   8

#define Z40   9
#define Z41   0
#define Z42   5
#define Z43   7
#define Z44   2
#define Z45   4
#define Z46   A
#define Z47   F
#define Z48   E
#define Z49   1
#define Z4A   B
#define Z4B   C
#define Z4C   6
#define Z4D   8
#define Z4E   3
#define Z4F   D

#define Z50   2
#define Z51   C
#define Z52   6
#define Z53   A
#define Z54   0
#define Z55   B
#define Z56   8
#define Z57   3
#define Z58   4
#define Z59   D
#define Z5A   7
#define Z5B   5
#define Z5C   F
#define Z5D   E
#define Z5E   1
#define Z5F   9

#define Z60   C
#define Z61   5
#define Z62   1
#define Z63   F
#define Z64   E
#define Z65   D
#define Z66   4
#define Z67   A
#define Z68   0
#define Z69   7
#define Z6A   6
#define Z6B   3
#define Z6C   9
#define Z6D   2
#define Z6E   8
#define Z6F   B

#define Z70   D
#define Z71   B
#define Z72   7
#define Z73   E
#define Z74   C
#define Z75   1
#define Z76   3
#define Z77   9
#define Z78   5
#define Z79   0
#define Z7A   F
#define Z7B   4
#define Z7C   8
#define Z7D   6
#define Z7E   2
#define Z7F   A

#define Z80   6
#define Z81   F
#define Z82   E
#define Z83   9
#define Z84   B
#define Z85   3
#define Z86   0
#define Z87   8
#define Z88   C
#define Z89   2
#define Z8A   D
#define Z8B   7
#define Z8C   1
#define Z8D   4
#define Z8E   A
#define Z8F   5

#define Z90   A
#define Z91   2
#define Z92   8
#define Z93   4
#define Z94   7
#define Z95   6
#define Z96   1
#define Z97   5
#define Z98   F
#define Z99   B
#define Z9A   9
#define Z9B   E
#define Z9C   3
#define Z9D   C
#define Z9E   D
#define Z9F   0

#define Mx(r, i)    Mx_(Z ## r ## i)
#define Mx_(n)      Mx__(n)
#define Mx__(n)     M ## n

// Blake-256 4 & 8 way

#define CSx(r, i)   CSx_(Z ## r ## i)
#define CSx_(n)     CSx__(n)
#define CSx__(n)    CS ## n

#define CS0   SPH_C32(0x243F6A88)
#define CS1   SPH_C32(0x85A308D3)
#define CS2   SPH_C32(0x13198A2E)
#define CS3   SPH_C32(0x03707344)
#define CS4   SPH_C32(0xA4093822)
#define CS5   SPH_C32(0x299F31D0)
#define CS6   SPH_C32(0x082EFA98)
#define CS7   SPH_C32(0xEC4E6C89)
#define CS8   SPH_C32(0x452821E6)
#define CS9   SPH_C32(0x38D01377)
#define CSA   SPH_C32(0xBE5466CF)
#define CSB   SPH_C32(0x34E90C6C)
#define CSC   SPH_C32(0xC0AC29B7)
#define CSD   SPH_C32(0xC97C50DD)
#define CSE   SPH_C32(0x3F84D5B5)
#define CSF   SPH_C32(0xB5470917)

#if SPH_COMPACT_BLAKE_32

static const sph_u32 CS[16] = {
	SPH_C32(0x243F6A88), SPH_C32(0x85A308D3),
	SPH_C32(0x13198A2E), SPH_C32(0x03707344),
	SPH_C32(0xA4093822), SPH_C32(0x299F31D0),
	SPH_C32(0x082EFA98), SPH_C32(0xEC4E6C89),
	SPH_C32(0x452821E6), SPH_C32(0x38D01377),
	SPH_C32(0xBE5466CF), SPH_C32(0x34E90C6C),
	SPH_C32(0xC0AC29B7), SPH_C32(0xC97C50DD),
	SPH_C32(0x3F84D5B5), SPH_C32(0xB5470917)
};

#endif


#define GS_4WAY( m0, m1, c0, c1, a, b, c, d ) \
do { \
   a = _mm_add_epi32( _mm_add_epi32( _mm_xor_si128( \
                 _mm_set_epi32( c1, c1, c1, c1 ), m0 ), b ), a ); \
   d = mm128_ror_32( _mm_xor_si128( d, a ), 16 ); \
   c = _mm_add_epi32( c, d ); \
   b = mm128_ror_32( _mm_xor_si128( b, c ), 12 ); \
   a = _mm_add_epi32( _mm_add_epi32( _mm_xor_si128( \
                 _mm_set_epi32( c0, c0, c0, c0 ), m1 ), b ), a ); \
   d = mm128_ror_32( _mm_xor_si128( d, a ), 8 ); \
   c = _mm_add_epi32( c, d ); \
   b = mm128_ror_32( _mm_xor_si128( b, c ), 7 ); \
} while (0)

#if SPH_COMPACT_BLAKE_32

// Blake-256 4 way

#define ROUND_S_4WAY(r)   do { \
	GS_4WAY(M[sigma[r][0x0]], M[sigma[r][0x1]], \
		CS[sigma[r][0x0]], CS[sigma[r][0x1]], V0, V4, V8, VC); \
	GS_4WAY(M[sigma[r][0x2]], M[sigma[r][0x3]], \
		CS[sigma[r][0x2]], CS[sigma[r][0x3]], V1, V5, V9, VD); \
	GS_4WAY(M[sigma[r][0x4]], M[sigma[r][0x5]], \
		CS[sigma[r][0x4]], CS[sigma[r][0x5]], V2, V6, VA, VE); \
	GS_4WAY(M[sigma[r][0x6]], M[sigma[r][0x7]], \
		CS[sigma[r][0x6]], CS[sigma[r][0x7]], V3, V7, VB, VF); \
	GS_4WAY(M[sigma[r][0x8]], M[sigma[r][0x9]], \
		CS[sigma[r][0x8]], CS[sigma[r][0x9]], V0, V5, VA, VF); \
	GS_4WAY(M[sigma[r][0xA]], M[sigma[r][0xB]], \
		CS[sigma[r][0xA]], CS[sigma[r][0xB]], V1, V6, VB, VC); \
	GS_4WAY(M[sigma[r][0xC]], M[sigma[r][0xD]], \
		CS[sigma[r][0xC]], CS[sigma[r][0xD]], V2, V7, V8, VD); \
	GS_4WAY(M[sigma[r][0xE]], M[sigma[r][0xF]], \
		CS[sigma[r][0xE]], CS[sigma[r][0xF]], V3, V4, V9, VE); \
} while (0)

#else

#define ROUND_S_4WAY(r)   do { \
	GS_4WAY(Mx(r, 0), Mx(r, 1), CSx(r, 0), CSx(r, 1), V0, V4, V8, VC); \
	GS_4WAY(Mx(r, 2), Mx(r, 3), CSx(r, 2), CSx(r, 3), V1, V5, V9, VD); \
	GS_4WAY(Mx(r, 4), Mx(r, 5), CSx(r, 4), CSx(r, 5), V2, V6, VA, VE); \
	GS_4WAY(Mx(r, 6), Mx(r, 7), CSx(r, 6), CSx(r, 7), V3, V7, VB, VF); \
	GS_4WAY(Mx(r, 8), Mx(r, 9), CSx(r, 8), CSx(r, 9), V0, V5, VA, VF); \
	GS_4WAY(Mx(r, A), Mx(r, B), CSx(r, A), CSx(r, B), V1, V6, VB, VC); \
	GS_4WAY(Mx(r, C), Mx(r, D), CSx(r, C), CSx(r, D), V2, V7, V8, VD); \
	GS_4WAY(Mx(r, E), Mx(r, F), CSx(r, E), CSx(r, F), V3, V4, V9, VE); \
} while (0)

#endif

#define DECL_STATE32_4WAY \
	__m128i H0, H1, H2, H3, H4, H5, H6, H7; \
	__m128i S0, S1, S2, S3; \
        uint32_t T0, T1;

#define READ_STATE32_4WAY(state)   do { \
		H0 = casti_m128i( state->H, 0 ); \
		H1 = casti_m128i( state->H, 1 ); \
		H2 = casti_m128i( state->H, 2 ); \
		H3 = casti_m128i( state->H, 3 ); \
		H4 = casti_m128i( state->H, 4 ); \
		H5 = casti_m128i( state->H, 5 ); \
		H6 = casti_m128i( state->H, 6 ); \
		H7 = casti_m128i( state->H, 7 ); \
		S0 = casti_m128i( state->S, 0 ); \
		S1 = casti_m128i( state->S, 1 ); \
		S2 = casti_m128i( state->S, 2 ); \
		S3 = casti_m128i( state->S, 3 ); \
		T0 = (state)->T0; \
		T1 = (state)->T1; \
	} while (0)

#define WRITE_STATE32_4WAY(state)   do { \
		casti_m128i( state->H, 0 ) = H0; \
		casti_m128i( state->H, 1 ) = H1; \
		casti_m128i( state->H, 2 ) = H2; \
		casti_m128i( state->H, 3 ) = H3; \
		casti_m128i( state->H, 4 ) = H4; \
		casti_m128i( state->H, 5 ) = H5; \
		casti_m128i( state->H, 6 ) = H6; \
		casti_m128i( state->H, 7 ) = H7; \
		casti_m128i( state->S, 0 ) = S0; \
		casti_m128i( state->S, 1 ) = S1; \
		casti_m128i( state->S, 2 ) = S2; \
		casti_m128i( state->S, 3 ) = S3; \
		(state)->T0 = T0; \
		(state)->T1 = T1; \
	} while (0)

#if SPH_COMPACT_BLAKE_32
// not used

#define COMPRESS32_4WAY( rounds )   do { \
	__m128i M[16]; \
	__m128i V0, V1, V2, V3, V4, V5, V6, V7; \
	__m128i V8, V9, VA, VB, VC, VD, VE, VF; \
	unsigned r; \
	V0 = H0; \
	V1 = H1; \
	V2 = H2; \
	V3 = H3; \
	V4 = H4; \
	V5 = H5; \
	V6 = H6; \
	V7 = H7; \
        V8 = _mm_xor_si128( S0, _mm_set_epi32( CS0, CS0, CS0, CS0 ) ); \
        V9 = _mm_xor_si128( S1, _mm_set_epi32( CS1, CS1, CS1, CS1 ) ); \
        VA = _mm_xor_si128( S2, _mm_set_epi32( CS2, CS2, CS2, CS2 ) ); \
        VB = _mm_xor_si128( S3, _mm_set_epi32( CS3, CS3, CS3, CS3 ) ); \
        VC = _mm_xor_si128( _mm_set_epi32( T0, T0, T0, T0 ), \
                            _mm_set_epi32( CS4, CS4, CS4, CS4 ) ); \
        VD = _mm_xor_si128( _mm_set_epi32( T0, T0, T0, T0 ), \
                            _mm_set_epi32( CS5, CS5, CS5, CS5 ) ); \
        VE = _mm_xor_si128( _mm_set_epi32( T1, T1, T1, T1 ) \
                          , _mm_set_epi32( CS6, CS6, CS6, CS6 ) ); \
        VF = _mm_xor_si128( _mm_set_epi32( T1, T1, T1, T1 ), \
                            _mm_set_epi32( CS7, CS7, CS7, CS7 ) ); \
	M[0x0] = mm128_bswap_32( *(buf +  0) ); \
	M[0x1] = mm128_bswap_32( *(buf +  1) ); \
	M[0x2] = mm128_bswap_32( *(buf +  2) ); \
	M[0x3] = mm128_bswap_32( *(buf +  3) ); \
	M[0x4] = mm128_bswap_32( *(buf +  4) ); \
	M[0x5] = mm128_bswap_32( *(buf +  5) ); \
	M[0x6] = mm128_bswap_32( *(buf +  6) ); \
	M[0x7] = mm128_bswap_32( *(buf +  7) ); \
	M[0x8] = mm128_bswap_32( *(buf +  8) ); \
	M[0x9] = mm128_bswap_32( *(buf +  9) ); \
	M[0xA] = mm128_bswap_32( *(buf + 10) ); \
	M[0xB] = mm128_bswap_32( *(buf + 11) ); \
	M[0xC] = mm128_bswap_32( *(buf + 12) ); \
	M[0xD] = mm128_bswap_32( *(buf + 13) ); \
	M[0xE] = mm128_bswap_32( *(buf + 14) ); \
	M[0xF] = mm128_bswap_32( *(buf + 15) ); \
	for (r = 0; r < rounds; r ++) \
		ROUND_S_4WAY(r); \
        H0 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S0, V0 ), V8 ), H0 ); \
        H1 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S1, V1 ), V9 ), H1 ); \
        H2 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S2, V2 ), VA ), H2 ); \
        H3 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S3, V3 ), VB ), H3 ); \
        H4 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S0, V4 ), VC ), H4 ); \
        H5 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S1, V5 ), VD ), H5 ); \
        H6 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S2, V6 ), VE ), H6 ); \
        H7 = _mm_xor_si128( _mm_xor_si128( \
                                   _mm_xor_si128( S3, V7 ), VF ), H7 ); \
	} while (0)

#else

// current impl

#define COMPRESS32_4WAY( rounds ) \
do { \
   __m128i M0, M1, M2, M3, M4, M5, M6, M7; \
   __m128i M8, M9, MA, MB, MC, MD, ME, MF; \
   __m128i V0, V1, V2, V3, V4, V5, V6, V7; \
   __m128i V8, V9, VA, VB, VC, VD, VE, VF; \
   V0 = H0; \
   V1 = H1; \
   V2 = H2; \
   V3 = H3; \
   V4 = H4; \
   V5 = H5; \
   V6 = H6; \
   V7 = H7; \
   V8 = _mm_xor_si128( S0, _mm_set1_epi32( CS0 ) ); \
   V9 = _mm_xor_si128( S1, _mm_set1_epi32( CS1 ) ); \
   VA = _mm_xor_si128( S2, _mm_set1_epi32( CS2 ) ); \
   VB = _mm_xor_si128( S3, _mm_set1_epi32( CS3 ) ); \
   VC = _mm_xor_si128( _mm_set1_epi32( T0 ), _mm_set1_epi32( CS4 ) ); \
   VD = _mm_xor_si128( _mm_set1_epi32( T0 ), _mm_set1_epi32( CS5 ) ); \
   VE = _mm_xor_si128( _mm_set1_epi32( T1 ), _mm_set1_epi32( CS6 ) ); \
   VF = _mm_xor_si128( _mm_set1_epi32( T1 ), _mm_set1_epi32( CS7 ) ); \
   M0 = mm128_bswap_32( buf[ 0] ); \
   M1 = mm128_bswap_32( buf[ 1] ); \
   M2 = mm128_bswap_32( buf[ 2] ); \
   M3 = mm128_bswap_32( buf[ 3] ); \
   M4 = mm128_bswap_32( buf[ 4] ); \
   M5 = mm128_bswap_32( buf[ 5] ); \
   M6 = mm128_bswap_32( buf[ 6] ); \
   M7 = mm128_bswap_32( buf[ 7] ); \
   M8 = mm128_bswap_32( buf[ 8] ); \
   M9 = mm128_bswap_32( buf[ 9] ); \
   MA = mm128_bswap_32( buf[10] ); \
   MB = mm128_bswap_32( buf[11] ); \
   MC = mm128_bswap_32( buf[12] ); \
   MD = mm128_bswap_32( buf[13] ); \
   ME = mm128_bswap_32( buf[14] ); \
   MF = mm128_bswap_32( buf[15] ); \
   ROUND_S_4WAY(0); \
   ROUND_S_4WAY(1); \
   ROUND_S_4WAY(2); \
   ROUND_S_4WAY(3); \
   ROUND_S_4WAY(4); \
   ROUND_S_4WAY(5); \
   ROUND_S_4WAY(6); \
   ROUND_S_4WAY(7); \
   if (rounds == 14) \
   { \
      ROUND_S_4WAY(8); \
      ROUND_S_4WAY(9); \
      ROUND_S_4WAY(0); \
      ROUND_S_4WAY(1); \
      ROUND_S_4WAY(2); \
      ROUND_S_4WAY(3); \
   } \
   H0 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( V8, V0 ), S0 ), H0 ); \
   H1 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( V9, V1 ), S1 ), H1 ); \
   H2 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( VA, V2 ), S2 ), H2 ); \
   H3 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( VB, V3 ), S3 ), H3 ); \
   H4 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( VC, V4 ), S0 ), H4 ); \
   H5 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( VD, V5 ), S1 ), H5 ); \
   H6 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( VE, V6 ), S2 ), H6 ); \
   H7 = _mm_xor_si128( _mm_xor_si128( _mm_xor_si128( VF, V7 ), S3 ), H7 ); \
} while (0)

#endif

#if defined (__AVX2__)

// Blake-256 8 way

#define GS_8WAY( m0, m1, c0, c1, a, b, c, d ) \
do { \
   a = _mm256_add_epi32( _mm256_add_epi32( _mm256_xor_si256( \
                 _mm256_set1_epi32( c1 ), m0 ), b ), a ); \
   d = mm256_ror_32( _mm256_xor_si256( d, a ), 16 ); \
   c = _mm256_add_epi32( c, d ); \
   b = mm256_ror_32( _mm256_xor_si256( b, c ), 12 ); \
   a = _mm256_add_epi32( _mm256_add_epi32( _mm256_xor_si256( \
                 _mm256_set1_epi32( c0 ), m1 ), b ), a ); \
   d = mm256_ror_32( _mm256_xor_si256( d, a ), 8 ); \
   c = _mm256_add_epi32( c, d ); \
   b = mm256_ror_32( _mm256_xor_si256( b, c ), 7 ); \
} while (0)

#define ROUND_S_8WAY(r)   do { \
        GS_8WAY(Mx(r, 0), Mx(r, 1), CSx(r, 0), CSx(r, 1), V0, V4, V8, VC); \
        GS_8WAY(Mx(r, 2), Mx(r, 3), CSx(r, 2), CSx(r, 3), V1, V5, V9, VD); \
        GS_8WAY(Mx(r, 4), Mx(r, 5), CSx(r, 4), CSx(r, 5), V2, V6, VA, VE); \
        GS_8WAY(Mx(r, 6), Mx(r, 7), CSx(r, 6), CSx(r, 7), V3, V7, VB, VF); \
        GS_8WAY(Mx(r, 8), Mx(r, 9), CSx(r, 8), CSx(r, 9), V0, V5, VA, VF); \
        GS_8WAY(Mx(r, A), Mx(r, B), CSx(r, A), CSx(r, B), V1, V6, VB, VC); \
        GS_8WAY(Mx(r, C), Mx(r, D), CSx(r, C), CSx(r, D), V2, V7, V8, VD); \
        GS_8WAY(Mx(r, E), Mx(r, F), CSx(r, E), CSx(r, F), V3, V4, V9, VE); \
} while (0)

#define DECL_STATE32_8WAY \
   __m256i H0, H1, H2, H3, H4, H5, H6, H7; \
   __m256i S0, S1, S2, S3; \
   sph_u32 T0, T1;

#define READ_STATE32_8WAY(state) \
do { \
   H0 = (state)->H[0]; \
   H1 = (state)->H[1]; \
   H2 = (state)->H[2]; \
   H3 = (state)->H[3]; \
   H4 = (state)->H[4]; \
   H5 = (state)->H[5]; \
   H6 = (state)->H[6]; \
   H7 = (state)->H[7]; \
   S0 = (state)->S[0]; \
   S1 = (state)->S[1]; \
   S2 = (state)->S[2]; \
   S3 = (state)->S[3]; \
   T0 = (state)->T0; \
   T1 = (state)->T1; \
} while (0)

#define WRITE_STATE32_8WAY(state) \
do { \
   (state)->H[0] = H0; \
   (state)->H[1] = H1; \
   (state)->H[2] = H2; \
   (state)->H[3] = H3; \
   (state)->H[4] = H4; \
   (state)->H[5] = H5; \
   (state)->H[6] = H6; \
   (state)->H[7] = H7; \
   (state)->S[0] = S0; \
   (state)->S[1] = S1; \
   (state)->S[2] = S2; \
   (state)->S[3] = S3; \
   (state)->T0 = T0; \
   (state)->T1 = T1; \
} while (0)

#define COMPRESS32_8WAY( rounds ) \
do { \
   __m256i M0, M1, M2, M3, M4, M5, M6, M7; \
   __m256i M8, M9, MA, MB, MC, MD, ME, MF; \
   __m256i V0, V1, V2, V3, V4, V5, V6, V7; \
   __m256i V8, V9, VA, VB, VC, VD, VE, VF; \
   V0 = H0; \
   V1 = H1; \
   V2 = H2; \
   V3 = H3; \
   V4 = H4; \
   V5 = H5; \
   V6 = H6; \
   V7 = H7; \
   V8 = _mm256_xor_si256( S0, _mm256_set1_epi32( CS0 ) ); \
   V9 = _mm256_xor_si256( S1, _mm256_set1_epi32( CS1 ) ); \
   VA = _mm256_xor_si256( S2, _mm256_set1_epi32( CS2 ) ); \
   VB = _mm256_xor_si256( S3, _mm256_set1_epi32( CS3 ) ); \
   VC = _mm256_xor_si256( _mm256_set1_epi32( T0 ), _mm256_set1_epi32( CS4 ) ); \
   VD = _mm256_xor_si256( _mm256_set1_epi32( T0 ), _mm256_set1_epi32( CS5 ) ); \
   VE = _mm256_xor_si256( _mm256_set1_epi32( T1 ), _mm256_set1_epi32( CS6 ) ); \
   VF = _mm256_xor_si256( _mm256_set1_epi32( T1 ), _mm256_set1_epi32( CS7 ) ); \
   M0 = mm256_bswap_32( * buf ); \
   M1 = mm256_bswap_32( *(buf+1) ); \
   M2 = mm256_bswap_32( *(buf+2) ); \
   M3 = mm256_bswap_32( *(buf+3) ); \
   M4 = mm256_bswap_32( *(buf+4) ); \
   M5 = mm256_bswap_32( *(buf+5) ); \
   M6 = mm256_bswap_32( *(buf+6) ); \
   M7 = mm256_bswap_32( *(buf+7) ); \
   M8 = mm256_bswap_32( *(buf+8) ); \
   M9 = mm256_bswap_32( *(buf+9) ); \
   MA = mm256_bswap_32( *(buf+10) ); \
   MB = mm256_bswap_32( *(buf+11) ); \
   MC = mm256_bswap_32( *(buf+12) ); \
   MD = mm256_bswap_32( *(buf+13) ); \
   ME = mm256_bswap_32( *(buf+14) ); \
   MF = mm256_bswap_32( *(buf+15) ); \
   ROUND_S_8WAY(0); \
   ROUND_S_8WAY(1); \
   ROUND_S_8WAY(2); \
   ROUND_S_8WAY(3); \
   ROUND_S_8WAY(4); \
   ROUND_S_8WAY(5); \
   ROUND_S_8WAY(6); \
   ROUND_S_8WAY(7); \
   if (rounds == 14) \
   { \
      ROUND_S_8WAY(8); \
      ROUND_S_8WAY(9); \
      ROUND_S_8WAY(0); \
      ROUND_S_8WAY(1); \
      ROUND_S_8WAY(2); \
      ROUND_S_8WAY(3); \
   } \
   H0 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( V8, V0 ), \
                                                              S0 ), H0 ); \
   H1 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( V9, V1 ), \
                                                              S1 ), H1 ); \
   H2 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( VA, V2 ), \
                                                              S2 ), H2 ); \
   H3 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( VB, V3 ), \
                                                              S3 ), H3 ); \
   H4 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( VC, V4 ), \
                                                              S0 ), H4 ); \
   H5 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( VD, V5 ), \
                                                              S1 ), H5 ); \
   H6 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( VE, V6 ), \
                                                              S2 ), H6 ); \
   H7 = _mm256_xor_si256( _mm256_xor_si256( _mm256_xor_si256( VF, V7 ), \
                                                              S3 ), H7 ); \
} while (0)


#endif

// Blake-256 4 way

static const uint32_t salt_zero_4way_small[4] = { 0, 0, 0, 0 };

static void
blake32_4way_init( blake_4way_small_context *ctx, const uint32_t *iv,
                   const uint32_t *salt, int rounds )
{
   casti_m128i( ctx->H, 0 ) = _mm_set1_epi32( iv[0] );
   casti_m128i( ctx->H, 1 ) = _mm_set1_epi32( iv[1] );
   casti_m128i( ctx->H, 2 ) = _mm_set1_epi32( iv[2] );
   casti_m128i( ctx->H, 3 ) = _mm_set1_epi32( iv[3] );
   casti_m128i( ctx->H, 4 ) = _mm_set1_epi32( iv[4] );
   casti_m128i( ctx->H, 5 ) = _mm_set1_epi32( iv[5] );
   casti_m128i( ctx->H, 6 ) = _mm_set1_epi32( iv[6] );
   casti_m128i( ctx->H, 7 ) = _mm_set1_epi32( iv[7] );

   casti_m128i( ctx->S, 0 ) = m128_zero;
   casti_m128i( ctx->S, 1 ) = m128_zero;
   casti_m128i( ctx->S, 2 ) = m128_zero;
   casti_m128i( ctx->S, 3 ) = m128_zero;
/*
   sc->S[0] = _mm_set1_epi32( salt[0] );
   sc->S[1] = _mm_set1_epi32( salt[1] );
   sc->S[2] = _mm_set1_epi32( salt[2] );
   sc->S[3] = _mm_set1_epi32( salt[3] );
*/
   ctx->T0 = ctx->T1 = 0;
   ctx->ptr = 0;
   ctx->rounds = rounds;
}

static void
blake32_4way( blake_4way_small_context *ctx, const void *data, size_t len )
{
   __m128i *buf = (__m128i*)ctx->buf;
   size_t  bptr = ctx->ptr<<2;
   size_t  vptr = ctx->ptr >> 2;
   size_t  blen = len << 2;
   DECL_STATE32_4WAY

   if ( blen < (sizeof ctx->buf) - bptr )
   {
      memcpy( buf + vptr, data, (sizeof ctx->buf) - bptr );
      bptr += blen;
      ctx->ptr = bptr>>2;
      return;
   }

   READ_STATE32_4WAY( ctx );
   while ( blen > 0 )
   {
      size_t clen = ( sizeof ctx->buf ) - bptr;

      if ( clen > blen )
	 clen = blen;
      memcpy( buf + vptr, data, clen );
      bptr += clen;
      data = (const unsigned char *)data + clen;
      blen -= clen;
      if ( bptr == ( sizeof ctx->buf ) )
      {
         if ( ( T0 = T0 + 512 ) < 512 )
            T1 = T1 + 1;
         COMPRESS32_4WAY( ctx->rounds );
	 bptr = 0;
      }
   }
   WRITE_STATE32_4WAY( ctx );
   ctx->ptr = bptr>>2;
}

static void
blake32_4way_close( blake_4way_small_context *ctx, unsigned ub, unsigned n,
               void *dst, size_t out_size_w32 )
{
   __m128i buf[16] __attribute__ ((aligned (64)));
   size_t   ptr     = ctx->ptr;
   size_t   vptr    = ctx->ptr>>2;
   unsigned bit_len = ( (unsigned)ptr << 3 );
   uint32_t tl      = ctx->T0 + bit_len;
   uint32_t th      = ctx->T1;

   if ( ptr == 0 )
   {
      ctx->T0 = 0xFFFFFE00UL;
      ctx->T1 = 0xFFFFFFFFUL;
   }
   else if ( ctx->T0 == 0 )
   {
      ctx->T0 = 0xFFFFFE00UL + bit_len;
      ctx->T1 = ctx->T1 - 1;
   } 
   else
      ctx->T0 -= 512 - bit_len;

   buf[vptr] = _mm_set1_epi32( 0x80 );

   if ( vptr < 12 )
   {
      memset_zero_128( buf + vptr + 1, 13 - vptr  );
      buf[ 13 ] = _mm_or_si128( buf[ 13 ], _mm_set1_epi32( 0x01000000UL ) );
      buf[ 14 ] = mm128_bswap_32( _mm_set1_epi32( th ) );
      buf[ 15 ] = mm128_bswap_32( _mm_set1_epi32( tl ) );
      blake32_4way( ctx, buf + vptr, 64 - ptr );
   }
   else
   {
      memset_zero_128( buf + vptr + 1, (60-ptr) >> 2 );
      blake32_4way( ctx, buf + vptr, 64 - ptr );
      ctx->T0 = 0xFFFFFE00UL;
      ctx->T1 = 0xFFFFFFFFUL;
      memset_zero_128( buf, 56>>2 );
      buf[ 13 ] = _mm_or_si128( buf[ 13 ], _mm_set1_epi32( 0x01000000UL ) );
      buf[ 14 ] = mm128_bswap_32( _mm_set1_epi32( th ) );
      buf[ 15 ] = mm128_bswap_32( _mm_set1_epi32( tl ) );
      blake32_4way( ctx, buf, 64 );
   }

   casti_m128i( dst, 0 ) = mm128_bswap_32( casti_m128i( ctx->H, 0 ) );
   casti_m128i( dst, 1 ) = mm128_bswap_32( casti_m128i( ctx->H, 1 ) );
   casti_m128i( dst, 2 ) = mm128_bswap_32( casti_m128i( ctx->H, 2 ) );
   casti_m128i( dst, 3 ) = mm128_bswap_32( casti_m128i( ctx->H, 3 ) );
   casti_m128i( dst, 4 ) = mm128_bswap_32( casti_m128i( ctx->H, 4 ) );
   casti_m128i( dst, 5 ) = mm128_bswap_32( casti_m128i( ctx->H, 5 ) );
   casti_m128i( dst, 6 ) = mm128_bswap_32( casti_m128i( ctx->H, 6 ) );
   casti_m128i( dst, 7 ) = mm128_bswap_32( casti_m128i( ctx->H, 7 ) );
}

#if defined (__AVX2__)

// Blake-256 8 way

static const sph_u32 salt_zero_8way_small[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static void
blake32_8way_init( blake_8way_small_context *sc, const sph_u32 *iv,
                   const sph_u32 *salt, int rounds )
{
   int i;
   for ( i = 0; i < 8; i++ )
      sc->H[i] = _mm256_set1_epi32( iv[i] );
   for ( i = 0; i < 4; i++ )
      sc->S[i] = _mm256_set1_epi32( salt[i] );
   sc->T0 = sc->T1 = 0;
   sc->ptr = 0;
   sc->rounds = rounds;
}

static void
blake32_8way( blake_8way_small_context *sc, const void *data, size_t len )
{
   __m256i *vdata = (__m256i*)data;
   __m256i *buf;
   size_t ptr;
   const int buf_size = 64;   // number of elements, sizeof/4
   DECL_STATE32_8WAY
   buf = sc->buf;
   ptr = sc->ptr;
   if ( len < buf_size - ptr )
   {
        memcpy_256( buf + (ptr>>2), vdata, len>>2 );
        ptr += len;
        sc->ptr = ptr;
        return;
   }

   READ_STATE32_8WAY(sc);
   while ( len > 0 )
   {
      size_t clen;

      clen = buf_size - ptr;
      if (clen > len)
           clen = len;
      memcpy_256( buf + (ptr>>2), vdata, clen>>2 );
      ptr += clen;
      vdata += (clen>>2);
      len -= clen;
      if ( ptr == buf_size )
      {
          if ( ( T0 = SPH_T32(T0 + 512) ) < 512 )
                T1 = SPH_T32(T1 + 1);
          COMPRESS32_8WAY( sc->rounds );
          ptr = 0;
      }
   }
   WRITE_STATE32_8WAY(sc);
   sc->ptr = ptr;
}

static void
blake32_8way_close( blake_8way_small_context *sc, unsigned ub, unsigned n,
                    void *dst, size_t out_size_w32 )
{
//   union {
        __m256i buf[16];
//        sph_u32 dummy;
//   } u;
   size_t ptr, k;
   unsigned bit_len;
   sph_u32 th, tl;
   __m256i *out;

   ptr = sc->ptr;
   bit_len = ((unsigned)ptr << 3);
   buf[ptr>>2] = _mm256_set1_epi32( 0x80 );
   tl = sc->T0 + bit_len;
   th = sc->T1;

   if ( ptr == 0 )
   {
        sc->T0 = SPH_C32(0xFFFFFE00UL);
        sc->T1 = SPH_C32(0xFFFFFFFFUL);
   }
   else if ( sc->T0 == 0 )
   {
        sc->T0 = SPH_C32(0xFFFFFE00UL) + bit_len;
        sc->T1 = SPH_T32(sc->T1 - 1);
   }
   else
        sc->T0 -= 512 - bit_len;

   if ( ptr <= 52 )
   {
       memset_zero_256( buf + (ptr>>2) + 1, (52 - ptr) >> 2 );
       if ( out_size_w32 == 8 )
           buf[52>>2] = _mm256_or_si256( buf[52>>2],
                                           _mm256_set1_epi32( 0x01000000UL ) );
       *(buf+(56>>2)) = mm256_bswap_32( _mm256_set1_epi32( th ) );
       *(buf+(60>>2)) = mm256_bswap_32( _mm256_set1_epi32( tl ) );
       blake32_8way( sc, buf + (ptr>>2), 64 - ptr );
   }
   else
   {
        memset_zero_256( buf + (ptr>>2) + 1, (60-ptr) >> 2 );
        blake32_8way( sc, buf + (ptr>>2), 64 - ptr );
        sc->T0 = SPH_C32(0xFFFFFE00UL);
        sc->T1 = SPH_C32(0xFFFFFFFFUL);
        memset_zero_256( buf, 56>>2 );
       if ( out_size_w32 == 8 )
           buf[52>>2] = _mm256_set1_epi32( 0x01000000UL );
        *(buf+(56>>2)) = mm256_bswap_32( _mm256_set1_epi32( th ) );
        *(buf+(60>>2)) = mm256_bswap_32( _mm256_set1_epi32( tl ) );
        blake32_8way( sc, buf, 64 );
   }
   out = (__m256i*)dst;
   for ( k = 0; k < out_size_w32; k++ )
        out[k] = mm256_bswap_32( sc->H[k] );
}

#endif

// Blake-256 4 way

// default 14 rounds, backward copatibility
void
blake256_4way_init(void *ctx)
{
   blake32_4way_init( ctx, IV256, salt_zero_4way_small, 14 );
}

void
blake256_4way(void *ctx, const void *data, size_t len)
{
	blake32_4way(ctx, data, len);
}

void
blake256_4way_close(void *ctx, void *dst)
{
        blake32_4way_close(ctx, 0, 0, dst, 8);
}

#if defined(__AVX2__)

// Blake-256 8 way

void
blake256_8way_init(void *cc)
{
   blake32_8way_init( cc, IV256, salt_zero_8way_small, 14 );
}

void
blake256_8way(void *cc, const void *data, size_t len)
{
        blake32_8way(cc, data, len);
}

void
blake256_8way_close(void *cc, void *dst)
{
        blake32_8way_close(cc, 0, 0, dst, 8);
}

#endif

// 14 rounds Blake, Decred
void blake256r14_4way_init(void *cc)
{
   blake32_4way_init( cc, IV256, salt_zero_4way_small, 14 );
}

void
blake256r14_4way(void *cc, const void *data, size_t len)
{
   blake32_4way(cc, data, len);
}

void
blake256r14_4way_close(void *cc, void *dst)
{
   blake32_4way_close(cc, 0, 0, dst, 8);
}

#if defined(__AVX2__)

void blake256r14_8way_init(void *cc)
{
   blake32_8way_init( cc, IV256, salt_zero_8way_small, 14 );
}

void
blake256r14_8way(void *cc, const void *data, size_t len)
{
   blake32_8way(cc, data, len);
}

void
blake256r14_8way_close(void *cc, void *dst)
{
   blake32_8way_close(cc, 0, 0, dst, 8);
}

#endif

// 8 rounds Blakecoin, Vanilla
void blake256r8_4way_init(void *cc)
{
   blake32_4way_init( cc, IV256, salt_zero_4way_small, 8 );
}

void
blake256r8_4way(void *cc, const void *data, size_t len)
{
   blake32_4way(cc, data, len);
}

void
blake256r8_4way_close(void *cc, void *dst)
{
   blake32_4way_close(cc, 0, 0, dst, 8);
}

#if defined (__AVX2__)

void blake256r8_8way_init(void *cc)
{
   blake32_8way_init( cc, IV256, salt_zero_8way_small, 8 );
}

void
blake256r8_8way(void *cc, const void *data, size_t len)
{
   blake32_8way(cc, data, len);
}

void
blake256r8_8way_close(void *cc, void *dst)
{
   blake32_8way_close(cc, 0, 0, dst, 8);
}

#endif

#ifdef __cplusplus
}
#endif

//#endif
