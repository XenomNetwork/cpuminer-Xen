/* $Id: bmw.c 227 2010-06-16 17:28:38Z tp $ */
/*
 * BMW implementation.
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

#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "bmw-hash-4way.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4146)
#endif

#define LPAR   (

static const sph_u64 IV512[] = {
        SPH_C64(0x8081828384858687), SPH_C64(0x88898A8B8C8D8E8F),
        SPH_C64(0x9091929394959697), SPH_C64(0x98999A9B9C9D9E9F),
        SPH_C64(0xA0A1A2A3A4A5A6A7), SPH_C64(0xA8A9AAABACADAEAF),
        SPH_C64(0xB0B1B2B3B4B5B6B7), SPH_C64(0xB8B9BABBBCBDBEBF),
        SPH_C64(0xC0C1C2C3C4C5C6C7), SPH_C64(0xC8C9CACBCCCDCECF),
        SPH_C64(0xD0D1D2D3D4D5D6D7), SPH_C64(0xD8D9DADBDCDDDEDF),
        SPH_C64(0xE0E1E2E3E4E5E6E7), SPH_C64(0xE8E9EAEBECEDEEEF),
        SPH_C64(0xF0F1F2F3F4F5F6F7), SPH_C64(0xF8F9FAFBFCFDFEFF)
};

#if defined(__SSE2__)

// BMW-512 2 way 64


#define s2b0(x) \
   _mm_xor_si128( _mm_xor_si128( _mm_srli_epi64( (x), 1), \
                                 _mm_slli_epi64( (x), 3) ), \
                  _mm_xor_si128( mm128_rol_64( (x),  4), \
                                 mm128_rol_64( (x), 37) ) )

#define s2b1(x) \
   _mm_xor_si128( _mm_xor_si128( _mm_srli_epi64( (x), 1), \
                                 _mm_slli_epi64( (x), 2) ), \
                  _mm_xor_si128( mm128_rol_64( (x), 13), \
                                 mm128_rol_64( (x), 43) ) )

#define s2b2(x) \
   _mm_xor_si128( _mm_xor_si128( _mm_srli_epi64( (x), 2), \
                                 _mm_slli_epi64( (x), 1) ), \
                  _mm_xor_si128( mm128_rol_64( (x), 19), \
                                 mm128_rol_64( (x), 53) ) )

#define s2b3(x) \
   _mm_xor_si128( _mm_xor_si128( _mm_srli_epi64( (x), 2), \
                                 _mm_slli_epi64( (x), 2) ), \
                  _mm_xor_si128( mm128_rol_64( (x), 28), \
                                 mm128_rol_64( (x), 59) ) )

#define s2b4(x) \
   _mm_xor_si128( (x), _mm_srli_epi64( (x), 1 ) )

#define s2b5(x) \
   _mm_xor_si128( (x), _mm_srli_epi64( (x), 2 ) )


#define r2b1(x)    mm128_rol_64( x,  5 )
#define r2b2(x)    mm128_rol_64( x, 11 )
#define r2b3(x)    mm128_rol_64( x, 27 )
#define r2b4(x)    mm128_rol_64( x, 32 )
#define r2b5(x)    mm128_rol_64( x, 37 )
#define r2b6(x)    mm128_rol_64( x, 43 )
#define r2b7(x)    mm128_rol_64( x, 53 )

#define mm128_rol_off_64( M, j, off ) \
   mm128_rol_64( M[ ( (j) + (off) ) & 0xF ] , \
                  ( ( (j) + (off) ) & 0xF ) + 1 )

#define add_elt_2b( M, H, j ) \
   _mm_xor_si128( \
      _mm_add_epi64( \
            _mm_sub_epi64( _mm_add_epi64( mm128_rol_off_64( M, j, 0 ), \
                                          mm128_rol_off_64( M, j, 3 ) ), \
                           mm128_rol_off_64( M, j, 10 ) ), \
            _mm_set1_epi64x( ( (j) + 16 ) * 0x0555555555555555ULL ) ), \
       H[ ( (j)+7 ) & 0xF ] )


#define expand1_2b( qt, M, H, i ) \
   _mm_add_epi64( \
      _mm_add_epi64( \
         _mm_add_epi64( \
             _mm_add_epi64( \
                _mm_add_epi64( s2b1( qt[ (i)-16 ] ), \
                               s2b2( qt[ (i)-15 ] ) ), \
                _mm_add_epi64( s2b3( qt[ (i)-14 ] ), \
                               s2b0( qt[ (i)-13 ] ) ) ), \
             _mm_add_epi64( \
                _mm_add_epi64( s2b1( qt[ (i)-12 ] ), \
                               s2b2( qt[ (i)-11 ] ) ), \
                _mm_add_epi64( s2b3( qt[ (i)-10 ] ), \
                               s2b0( qt[ (i)- 9 ] ) ) ) ), \
         _mm_add_epi64( \
             _mm_add_epi64( \
                _mm_add_epi64( s2b1( qt[ (i)- 8 ] ), \
                               s2b2( qt[ (i)- 7 ] ) ), \
                _mm_add_epi64( s2b3( qt[ (i)- 6 ] ), \
                               s2b0( qt[ (i)- 5 ] ) ) ), \
             _mm_add_epi64( \
                _mm_add_epi64( s2b1( qt[ (i)- 4 ] ), \
                               s2b2( qt[ (i)- 3 ] ) ), \
                _mm_add_epi64( s2b3( qt[ (i)- 2 ] ), \
                               s2b0( qt[ (i)- 1 ] ) ) ) ) ), \
      add_elt_2b( M, H, (i)-16 ) )

#define expand2_2b( qt, M, H, i) \
   _mm_add_epi64( \
      _mm_add_epi64( \
         _mm_add_epi64( \
             _mm_add_epi64( \
                _mm_add_epi64( qt[ (i)-16 ], r2b1( qt[ (i)-15 ] ) ), \
                _mm_add_epi64( qt[ (i)-14 ], r2b2( qt[ (i)-13 ] ) ) ), \
             _mm_add_epi64( \
                _mm_add_epi64( qt[ (i)-12 ], r2b3( qt[ (i)-11 ] ) ), \
                _mm_add_epi64( qt[ (i)-10 ], r2b4( qt[ (i)- 9 ] ) ) ) ), \
         _mm_add_epi64( \
             _mm_add_epi64( \
                _mm_add_epi64( qt[ (i)- 8 ], r2b5( qt[ (i)- 7 ] ) ), \
                _mm_add_epi64( qt[ (i)- 6 ], r2b6( qt[ (i)- 5 ] ) ) ), \
             _mm_add_epi64( \
                _mm_add_epi64( qt[ (i)- 4 ], r2b7( qt[ (i)- 3 ] ) ), \
                _mm_add_epi64( s2b4( qt[ (i)- 2 ] ), \
                               s2b5( qt[ (i)- 1 ] ) ) ) ) ), \
      add_elt_2b( M, H, (i)-16 ) )


#define W2b0 \
   _mm_add_epi64( \
       _mm_add_epi64( \
          _mm_add_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 5], H[ 5] ), \
                            _mm_xor_si128( M[ 7], H[ 7] ) ), \
             _mm_xor_si128( M[10], H[10] ) ), \
          _mm_xor_si128( M[13], H[13] ) ), \
       _mm_xor_si128( M[14], H[14] ) )

#define W2b1 \
   _mm_sub_epi64( \
       _mm_add_epi64( \
          _mm_add_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 6], H[ 6] ), \
                            _mm_xor_si128( M[ 8], H[ 8] ) ), \
             _mm_xor_si128( M[11], H[11] ) ), \
          _mm_xor_si128( M[14], H[14] ) ), \
       _mm_xor_si128( M[15], H[15] ) )

#define W2b2 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_add_epi64( \
             _mm_add_epi64( _mm_xor_si128( M[ 0], H[ 0] ), \
                            _mm_xor_si128( M[ 7], H[ 7] ) ), \
             _mm_xor_si128( M[ 9], H[ 9] ) ), \
          _mm_xor_si128( M[12], H[12] ) ), \
       _mm_xor_si128( M[15], H[15] ) )

#define W2b3 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_add_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 0], H[ 0] ), \
                               _mm_xor_si128( M[ 1], H[ 1] ) ), \
             _mm_xor_si128( M[ 8], H[ 8] ) ), \
          _mm_xor_si128( M[10], H[10] ) ), \
       _mm_xor_si128( M[13], H[13] ) )

#define W2b4 \
   _mm_sub_epi64( \
       _mm_sub_epi64( \
          _mm_add_epi64( \
             _mm_add_epi64( _mm_xor_si128( M[ 1], H[ 1] ), \
                            _mm_xor_si128( M[ 2], H[ 2] ) ), \
             _mm_xor_si128( M[ 9], H[ 9] ) ), \
          _mm_xor_si128( M[11], H[11] ) ), \
       _mm_xor_si128( M[14], H[14] ) )

#define W2b5 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_add_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 3], H[ 3] ), \
                            _mm_xor_si128( M[ 2], H[ 2] ) ), \
             _mm_xor_si128( M[10], H[10] ) ), \
          _mm_xor_si128( M[12], H[12] ) ), \
       _mm_xor_si128( M[15], H[15] ) )

#define W2b6 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_sub_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 4], H[ 4] ), \
                            _mm_xor_si128( M[ 0], H[ 0] ) ), \
             _mm_xor_si128( M[ 3], H[ 3] ) ), \
          _mm_xor_si128( M[11], H[11] ) ), \
       _mm_xor_si128( M[13], H[13] ) )

#define W2b7 \
   _mm_sub_epi64( \
       _mm_sub_epi64( \
          _mm_sub_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 1], H[ 1] ), \
                            _mm_xor_si128( M[ 4], H[ 4] ) ), \
             _mm_xor_si128( M[ 5], H[ 5] ) ), \
          _mm_xor_si128( M[12], H[12] ) ), \
       _mm_xor_si128( M[14], H[14] ) )

#define W2b8 \
   _mm_sub_epi64( \
       _mm_add_epi64( \
          _mm_sub_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 2], H[ 2] ), \
                            _mm_xor_si128( M[ 5], H[ 5] ) ), \
             _mm_xor_si128( M[ 6], H[ 6] ) ), \
          _mm_xor_si128( M[13], H[13] ) ), \
       _mm_xor_si128( M[15], H[15] ) )

#define W2b9 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_add_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 0], H[ 0] ), \
                            _mm_xor_si128( M[ 3], H[ 3] ) ), \
             _mm_xor_si128( M[ 6], H[ 6] ) ), \
          _mm_xor_si128( M[ 7], H[ 7] ) ), \
       _mm_xor_si128( M[14], H[14] ) )

#define W2b10 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_sub_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 8], H[ 8] ), \
                            _mm_xor_si128( M[ 1], H[ 1] ) ), \
             _mm_xor_si128( M[ 4], H[ 4] ) ), \
          _mm_xor_si128( M[ 7], H[ 7] ) ), \
       _mm_xor_si128( M[15], H[15] ) )

#define W2b11 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_sub_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 8], H[ 8] ), \
                            _mm_xor_si128( M[ 0], H[ 0] ) ), \
             _mm_xor_si128( M[ 2], H[ 2] ) ), \
          _mm_xor_si128( M[ 5], H[ 5] ) ), \
       _mm_xor_si128( M[ 9], H[ 9] ) )

#define W2b12 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_sub_epi64( \
             _mm_add_epi64( _mm_xor_si128( M[ 1], H[ 1] ), \
                            _mm_xor_si128( M[ 3], H[ 3] ) ), \
             _mm_xor_si128( M[ 6], H[ 6] ) ), \
          _mm_xor_si128( M[ 9], H[ 9] ) ), \
       _mm_xor_si128( M[10], H[10] ) )

#define W2b13 \
   _mm_add_epi64( \
       _mm_add_epi64( \
          _mm_add_epi64( \
             _mm_add_epi64( _mm_xor_si128( M[ 2], H[ 2] ), \
                            _mm_xor_si128( M[ 4], H[ 4] ) ), \
             _mm_xor_si128( M[ 7], H[ 7] ) ), \
          _mm_xor_si128( M[10], H[10] ) ), \
       _mm_xor_si128( M[11], H[11] ) )

#define W2b14 \
   _mm_sub_epi64( \
       _mm_sub_epi64( \
          _mm_add_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[ 3], H[ 3] ), \
                            _mm_xor_si128( M[ 5], H[ 5] ) ), \
             _mm_xor_si128( M[ 8], H[ 8] ) ), \
          _mm_xor_si128( M[11], H[11] ) ), \
       _mm_xor_si128( M[12], H[12] ) )

#define W2b15 \
   _mm_add_epi64( \
       _mm_sub_epi64( \
          _mm_sub_epi64( \
             _mm_sub_epi64( _mm_xor_si128( M[12], H[12] ), \
                            _mm_xor_si128( M[ 4], H[4] ) ), \
             _mm_xor_si128( M[ 6], H[ 6] ) ), \
          _mm_xor_si128( M[ 9], H[ 9] ) ), \
       _mm_xor_si128( M[13], H[13] ) )


void compress_big_2way( const __m128i *M, const __m128i H[16],
                        __m128i dH[16] )
{
   __m128i qt[32], xl, xh;

   qt[ 0] = _mm_add_epi64( s2b0( W2b0 ), H[ 1] );
   qt[ 1] = _mm_add_epi64( s2b1( W2b1 ), H[ 2] );
   qt[ 2] = _mm_add_epi64( s2b2( W2b2 ), H[ 3] );
   qt[ 3] = _mm_add_epi64( s2b3( W2b3 ), H[ 4] );
   qt[ 4] = _mm_add_epi64( s2b4( W2b4 ), H[ 5] );
   qt[ 5] = _mm_add_epi64( s2b0( W2b5 ), H[ 6] );
   qt[ 6] = _mm_add_epi64( s2b1( W2b6 ), H[ 7] );
   qt[ 7] = _mm_add_epi64( s2b2( W2b7 ), H[ 8] );
   qt[ 8] = _mm_add_epi64( s2b3( W2b8 ), H[ 9] );
   qt[ 9] = _mm_add_epi64( s2b4( W2b9 ), H[10] );
   qt[10] = _mm_add_epi64( s2b0( W2b10), H[11] );
   qt[11] = _mm_add_epi64( s2b1( W2b11), H[12] );
   qt[12] = _mm_add_epi64( s2b2( W2b12), H[13] );
   qt[13] = _mm_add_epi64( s2b3( W2b13), H[14] );
   qt[14] = _mm_add_epi64( s2b4( W2b14), H[15] );
   qt[15] = _mm_add_epi64( s2b0( W2b15), H[ 0] );
   qt[16] = expand1_2b( qt, M, H, 16 );
   qt[17] = expand1_2b( qt, M, H, 17 );
   qt[18] = expand2_2b( qt, M, H, 18 );
   qt[19] = expand2_2b( qt, M, H, 19 );
   qt[20] = expand2_2b( qt, M, H, 20 );
   qt[21] = expand2_2b( qt, M, H, 21 );
   qt[22] = expand2_2b( qt, M, H, 22 );
   qt[23] = expand2_2b( qt, M, H, 23 );
   qt[24] = expand2_2b( qt, M, H, 24 );
   qt[25] = expand2_2b( qt, M, H, 25 );
   qt[26] = expand2_2b( qt, M, H, 26 );
   qt[27] = expand2_2b( qt, M, H, 27 );
   qt[28] = expand2_2b( qt, M, H, 28 );
   qt[29] = expand2_2b( qt, M, H, 29 );
   qt[30] = expand2_2b( qt, M, H, 30 );
   qt[31] = expand2_2b( qt, M, H, 31 );

   xl = _mm_xor_si128(
            _mm_xor_si128( _mm_xor_si128( qt[16], qt[17] ),
                           _mm_xor_si128( qt[18], qt[19] ) ),
            _mm_xor_si128( _mm_xor_si128( qt[20], qt[21] ),
                           _mm_xor_si128( qt[22], qt[23] ) ) );
   xh = _mm_xor_si128( xl,
            _mm_xor_si128(
                 _mm_xor_si128( _mm_xor_si128( qt[24], qt[25] ),
                                _mm_xor_si128( qt[26], qt[27] ) ),
                 _mm_xor_si128( _mm_xor_si128( qt[28], qt[29] ),
                                _mm_xor_si128( qt[30], qt[31] ) ) ) );

   dH[ 0] = _mm_add_epi64(
              _mm_xor_si128( M[0],
                    _mm_xor_si128( _mm_slli_epi64( xh, 5 ),
                                   _mm_srli_epi64( qt[16], 5 ) ) ),
              _mm_xor_si128( _mm_xor_si128( xl, qt[24] ), qt[ 0] ) );
   dH[ 1] = _mm_add_epi64(
              _mm_xor_si128( M[1],
                    _mm_xor_si128( _mm_srli_epi64( xh, 7 ),
                                   _mm_slli_epi64( qt[17], 8 ) ) ),
              _mm_xor_si128( _mm_xor_si128( xl, qt[25] ), qt[ 1] ) );
   dH[ 2] = _mm_add_epi64(
               _mm_xor_si128( M[2],
                    _mm_xor_si128( _mm_srli_epi64( xh, 5 ),
                                _mm_slli_epi64( qt[18], 5 ) ) ),
               _mm_xor_si128( _mm_xor_si128( xl, qt[26] ), qt[ 2] ) );
   dH[ 3] = _mm_add_epi64(
               _mm_xor_si128( M[3],
                    _mm_xor_si128( _mm_srli_epi64( xh, 1 ),
                                   _mm_slli_epi64( qt[19], 5 ) ) ),
               _mm_xor_si128( _mm_xor_si128( xl, qt[27] ), qt[ 3] ) );
   dH[ 4] = _mm_add_epi64(
               _mm_xor_si128( M[4],
                    _mm_xor_si128( _mm_srli_epi64( xh, 3 ),
                                      _mm_slli_epi64( qt[20], 0 ) ) ),
               _mm_xor_si128( _mm_xor_si128( xl, qt[28] ), qt[ 4] ) );
   dH[ 5] = _mm_add_epi64(
               _mm_xor_si128( M[5],
                    _mm_xor_si128( _mm_slli_epi64( xh, 6 ),
                                   _mm_srli_epi64( qt[21], 6 ) ) ),
               _mm_xor_si128( _mm_xor_si128( xl, qt[29] ), qt[ 5] ) );
   dH[ 6] = _mm_add_epi64(
               _mm_xor_si128( M[6],
                    _mm_xor_si128( _mm_srli_epi64( xh, 4 ),
                                   _mm_slli_epi64( qt[22], 6 ) ) ),
                 _mm_xor_si128( _mm_xor_si128( xl, qt[30] ), qt[ 6] ) );
   dH[ 7] = _mm_add_epi64(
               _mm_xor_si128( M[7],
                    _mm_xor_si128( _mm_srli_epi64( xh, 11 ),
                                   _mm_slli_epi64( qt[23], 2 ) ) ),
               _mm_xor_si128( _mm_xor_si128( xl, qt[31] ), qt[ 7] ) );
   dH[ 8] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[4], 9 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[24] ), M[ 8] ) ),
               _mm_xor_si128( _mm_slli_epi64( xl, 8 ),
                              _mm_xor_si128( qt[23], qt[ 8] ) ) );
   dH[ 9] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[5], 10 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[25] ), M[ 9] ) ),
               _mm_xor_si128( _mm_srli_epi64( xl, 6 ),
                              _mm_xor_si128( qt[16], qt[ 9] ) ) );
   dH[10] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[6], 11 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[26] ), M[10] ) ),
               _mm_xor_si128( _mm_slli_epi64( xl, 6 ),
                              _mm_xor_si128( qt[17], qt[10] ) ) );
   dH[11] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[7], 12 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[27] ), M[11] )),
               _mm_xor_si128( _mm_slli_epi64( xl, 4 ),
                              _mm_xor_si128( qt[18], qt[11] ) ) );
   dH[12] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[0], 13 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[28] ), M[12] ) ),
               _mm_xor_si128( _mm_srli_epi64( xl, 3 ),
                              _mm_xor_si128( qt[19], qt[12] ) ) );
   dH[13] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[1], 14 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[29] ), M[13] ) ),
               _mm_xor_si128( _mm_srli_epi64( xl, 4 ),
                              _mm_xor_si128( qt[20], qt[13] ) ) );
   dH[14] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[2], 15 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[30] ), M[14] ) ),
               _mm_xor_si128( _mm_srli_epi64( xl, 7 ),
                              _mm_xor_si128( qt[21], qt[14] ) ) );
   dH[15] = _mm_add_epi64( _mm_add_epi64(
               mm128_rol_64( dH[3], 16 ),
               _mm_xor_si128( _mm_xor_si128( xh, qt[31] ), M[15] ) ),
               _mm_xor_si128( _mm_srli_epi64( xl, 2 ),
                              _mm_xor_si128( qt[22], qt[15] ) ) );
}

static const __m128i final_b2[16] =
{
   { 0xaaaaaaaaaaaaaaa0, 0xaaaaaaaaaaaaaaa0 },
   { 0xaaaaaaaaaaaaaaa0, 0xaaaaaaaaaaaaaaa0 },
   { 0xaaaaaaaaaaaaaaa1, 0xaaaaaaaaaaaaaaa1 },
   { 0xaaaaaaaaaaaaaaa1, 0xaaaaaaaaaaaaaaa1 },
   { 0xaaaaaaaaaaaaaaa2, 0xaaaaaaaaaaaaaaa2 },
   { 0xaaaaaaaaaaaaaaa2, 0xaaaaaaaaaaaaaaa2 },
   { 0xaaaaaaaaaaaaaaa3, 0xaaaaaaaaaaaaaaa3 },
   { 0xaaaaaaaaaaaaaaa3, 0xaaaaaaaaaaaaaaa3 },
   { 0xaaaaaaaaaaaaaaa4, 0xaaaaaaaaaaaaaaa4 },
   { 0xaaaaaaaaaaaaaaa4, 0xaaaaaaaaaaaaaaa4 },
   { 0xaaaaaaaaaaaaaaa5, 0xaaaaaaaaaaaaaaa5 },
   { 0xaaaaaaaaaaaaaaa5, 0xaaaaaaaaaaaaaaa5 },
   { 0xaaaaaaaaaaaaaaa6, 0xaaaaaaaaaaaaaaa6 },
   { 0xaaaaaaaaaaaaaaa6, 0xaaaaaaaaaaaaaaa6 },
   { 0xaaaaaaaaaaaaaaa7, 0xaaaaaaaaaaaaaaa7 },
   { 0xaaaaaaaaaaaaaaaf, 0xaaaaaaaaaaaaaaaf }
};

void bmw512_2way_init( bmw_2way_big_context *ctx )
{
   ctx->H[ 0] = _mm_set1_epi64x( IV512[ 0] );
   ctx->H[ 1] = _mm_set1_epi64x( IV512[ 1] );
   ctx->H[ 2] = _mm_set1_epi64x( IV512[ 2] );
   ctx->H[ 3] = _mm_set1_epi64x( IV512[ 3] );
   ctx->H[ 4] = _mm_set1_epi64x( IV512[ 4] );
   ctx->H[ 5] = _mm_set1_epi64x( IV512[ 5] );
   ctx->H[ 6] = _mm_set1_epi64x( IV512[ 6] );
   ctx->H[ 7] = _mm_set1_epi64x( IV512[ 7] );
   ctx->H[ 8] = _mm_set1_epi64x( IV512[ 8] );
   ctx->H[ 9] = _mm_set1_epi64x( IV512[ 9] );
   ctx->H[10] = _mm_set1_epi64x( IV512[10] );
   ctx->H[11] = _mm_set1_epi64x( IV512[11] );
   ctx->H[12] = _mm_set1_epi64x( IV512[12] );
   ctx->H[13] = _mm_set1_epi64x( IV512[13] );
   ctx->H[14] = _mm_set1_epi64x( IV512[14] );
   ctx->H[15] = _mm_set1_epi64x( IV512[15] );
   ctx->ptr = 0;
   ctx->bit_count = 0;
}

void bmw512_2way( bmw_2way_big_context *ctx, const void *data, size_t len )
{
   __m128i *buf = (__m128i*)ctx->buf;
   __m128i htmp[16];
   __m128i *h1 = ctx->H;
   __m128i *h2 = htmp;
   size_t blen = len << 1;
   size_t ptr = ctx->ptr;
   size_t bptr = ctx->ptr << 1;
   size_t vptr = ctx->ptr >> 3;
//   const int buf_size = 128;  // bytes of one lane, compatible with len

   ctx->bit_count += len << 3;
   while ( blen > 0 )
   {
      size_t clen = (sizeof ctx->buf ) - bptr;
      if ( clen > blen )
         clen = blen;
      memcpy( buf + vptr, data, clen );
      bptr += clen;
      vptr = bptr >> 4;
      data = (const unsigned char *)data + clen;
      blen -= clen;
      if ( ptr == (sizeof ctx->buf ) )
      {
         __m128i *ht;
         compress_big_2way( buf, h1, h2 );
         ht = h1;
         h1 = h2;
         h2 = ht;
         ptr = 0;
      }
   }
   ctx->ptr = ptr;
   if ( h1 != ctx->H )
        memcpy_128( ctx->H, h1, 16 );
}

void bmw512_2way_close( bmw_2way_big_context *ctx, void *dst )
{
   __m128i h1[16], h2[16], *h;
   __m128i *buf = (__m128i*)ctx->buf;
   size_t   vptr    = ctx->ptr >> 3;
//   unsigned bit_len = ( (unsigned)(ctx->ptr) << 1 );

   buf[ vptr++ ] = _mm_set1_epi64x( 0x80 );
   h = ctx->H;

   if ( vptr == 16 )
   {
      compress_big_2way( buf, h, h1 );
      vptr = 0;
      h = h1;
   }
   memset_zero_128( buf + vptr, 16 - vptr - 1 );
   buf[ 15 ] = _mm_set1_epi64x( ctx->bit_count );
   compress_big_2way( buf, h, h2 );
   memcpy_128( buf, h2, 16 );
   compress_big_2way( buf, final_b2, h1 );
   memcpy( (__m128i*)dst, h1+16, 8 );
}

#endif  // __SSE2__



#if defined(__AVX2__)

// BMW-512 4 way 64


#define sb0(x) \
   _mm256_xor_si256( _mm256_xor_si256( _mm256_srli_epi64( (x), 1), \
                                       _mm256_slli_epi64( (x), 3) ), \
                     _mm256_xor_si256( mm256_rol_64( (x),  4), \
                                       mm256_rol_64( (x), 37) ) )

#define sb1(x) \
   _mm256_xor_si256( _mm256_xor_si256( _mm256_srli_epi64( (x), 1), \
                                       _mm256_slli_epi64( (x), 2) ), \
                     _mm256_xor_si256( mm256_rol_64( (x), 13), \
                                       mm256_rol_64( (x), 43) ) )

#define sb2(x) \
   _mm256_xor_si256( _mm256_xor_si256( _mm256_srli_epi64( (x), 2), \
                                       _mm256_slli_epi64( (x), 1) ), \
                     _mm256_xor_si256( mm256_rol_64( (x), 19), \
                                       mm256_rol_64( (x), 53) ) )

#define sb3(x) \
   _mm256_xor_si256( _mm256_xor_si256( _mm256_srli_epi64( (x), 2), \
                                       _mm256_slli_epi64( (x), 2) ), \
                     _mm256_xor_si256( mm256_rol_64( (x), 28), \
                                       mm256_rol_64( (x), 59) ) )

#define sb4(x) \
  _mm256_xor_si256( (x), _mm256_srli_epi64( (x), 1 ) )

#define sb5(x) \
  _mm256_xor_si256( (x), _mm256_srli_epi64( (x), 2 ) )

#define rb1(x)    mm256_rol_64( x,  5 ) 
#define rb2(x)    mm256_rol_64( x, 11 ) 
#define rb3(x)    mm256_rol_64( x, 27 ) 
#define rb4(x)    mm256_rol_64( x, 32 ) 
#define rb5(x)    mm256_rol_64( x, 37 ) 
#define rb6(x)    mm256_rol_64( x, 43 ) 
#define rb7(x)    mm256_rol_64( x, 53 ) 

#define rol_off_64( M, j, off ) \
   mm256_rol_64( M[ ( (j) + (off) ) & 0xF ] , \
                  ( ( (j) + (off) ) & 0xF ) + 1 )

#define add_elt_b( M, H, j ) \
   _mm256_xor_si256( \
      _mm256_add_epi64( \
            _mm256_sub_epi64( _mm256_add_epi64( rol_off_64( M, j, 0 ), \
                                                rol_off_64( M, j, 3 ) ), \
                             rol_off_64( M, j, 10 ) ), \
            _mm256_set1_epi64x( ( (j) + 16 ) * 0x0555555555555555ULL ) ), \
       H[ ( (j)+7 ) & 0xF ] )
          
#define expand1b( qt, M, H, i ) \
   _mm256_add_epi64( \
      _mm256_add_epi64( \
         _mm256_add_epi64( \
             _mm256_add_epi64( \
                _mm256_add_epi64( sb1( qt[ (i)-16 ] ), \
                                  sb2( qt[ (i)-15 ] ) ), \
                _mm256_add_epi64( sb3( qt[ (i)-14 ] ), \
                                  sb0( qt[ (i)-13 ] ) ) ), \
             _mm256_add_epi64( \
                _mm256_add_epi64( sb1( qt[ (i)-12 ] ), \
                                  sb2( qt[ (i)-11 ] ) ), \
                _mm256_add_epi64( sb3( qt[ (i)-10 ] ), \
                                  sb0( qt[ (i)- 9 ] ) ) ) ), \
         _mm256_add_epi64( \
             _mm256_add_epi64( \
                _mm256_add_epi64( sb1( qt[ (i)- 8 ] ), \
                                  sb2( qt[ (i)- 7 ] ) ), \
                _mm256_add_epi64( sb3( qt[ (i)- 6 ] ), \
                                  sb0( qt[ (i)- 5 ] ) ) ), \
             _mm256_add_epi64( \
                _mm256_add_epi64( sb1( qt[ (i)- 4 ] ), \
                                  sb2( qt[ (i)- 3 ] ) ), \
                _mm256_add_epi64( sb3( qt[ (i)- 2 ] ), \
                                  sb0( qt[ (i)- 1 ] ) ) ) ) ), \
      add_elt_b( M, H, (i)-16 ) )

#define expand2b( qt, M, H, i) \
   _mm256_add_epi64( \
      _mm256_add_epi64( \
         _mm256_add_epi64( \
             _mm256_add_epi64( \
                _mm256_add_epi64( qt[ (i)-16 ], rb1( qt[ (i)-15 ] ) ), \
                _mm256_add_epi64( qt[ (i)-14 ], rb2( qt[ (i)-13 ] ) ) ), \
             _mm256_add_epi64( \
                _mm256_add_epi64( qt[ (i)-12 ], rb3( qt[ (i)-11 ] ) ), \
                _mm256_add_epi64( qt[ (i)-10 ], rb4( qt[ (i)- 9 ] ) ) ) ), \
         _mm256_add_epi64( \
             _mm256_add_epi64( \
                _mm256_add_epi64( qt[ (i)- 8 ], rb5( qt[ (i)- 7 ] ) ), \
                _mm256_add_epi64( qt[ (i)- 6 ], rb6( qt[ (i)- 5 ] ) ) ), \
             _mm256_add_epi64( \
                _mm256_add_epi64( qt[ (i)- 4 ], rb7( qt[ (i)- 3 ] ) ), \
                _mm256_add_epi64( sb4( qt[ (i)- 2 ] ), \
                                  sb5( qt[ (i)- 1 ] ) ) ) ) ), \
      add_elt_b( M, H, (i)-16 ) )


#define Wb0 \
   _mm256_add_epi64( \
       _mm256_add_epi64( \
          _mm256_add_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 5], H[ 5] ), \
                               _mm256_xor_si256( M[ 7], H[ 7] ) ), \
             _mm256_xor_si256( M[10], H[10] ) ), \
          _mm256_xor_si256( M[13], H[13] ) ), \
       _mm256_xor_si256( M[14], H[14] ) )

#define Wb1 \
   _mm256_sub_epi64( \
       _mm256_add_epi64( \
          _mm256_add_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 6], H[ 6] ), \
                               _mm256_xor_si256( M[ 8], H[ 8] ) ), \
             _mm256_xor_si256( M[11], H[11] ) ), \
          _mm256_xor_si256( M[14], H[14] ) ), \
       _mm256_xor_si256( M[15], H[15] ) )

#define Wb2 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_add_epi64( \
             _mm256_add_epi64( _mm256_xor_si256( M[ 0], H[ 0] ), \
                               _mm256_xor_si256( M[ 7], H[ 7] ) ), \
             _mm256_xor_si256( M[ 9], H[ 9] ) ), \
          _mm256_xor_si256( M[12], H[12] ) ), \
       _mm256_xor_si256( M[15], H[15] ) )

#define Wb3 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_add_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 0], H[ 0] ), \
                               _mm256_xor_si256( M[ 1], H[ 1] ) ), \
             _mm256_xor_si256( M[ 8], H[ 8] ) ), \
          _mm256_xor_si256( M[10], H[10] ) ), \
       _mm256_xor_si256( M[13], H[13] ) )

#define Wb4 \
   _mm256_sub_epi64( \
       _mm256_sub_epi64( \
          _mm256_add_epi64( \
             _mm256_add_epi64( _mm256_xor_si256( M[ 1], H[ 1] ), \
                               _mm256_xor_si256( M[ 2], H[ 2] ) ), \
             _mm256_xor_si256( M[ 9], H[ 9] ) ), \
          _mm256_xor_si256( M[11], H[11] ) ), \
       _mm256_xor_si256( M[14], H[14] ) )

#define Wb5 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_add_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 3], H[ 3] ), \
                               _mm256_xor_si256( M[ 2], H[ 2] ) ), \
             _mm256_xor_si256( M[10], H[10] ) ), \
          _mm256_xor_si256( M[12], H[12] ) ), \
       _mm256_xor_si256( M[15], H[15] ) )

#define Wb6 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_sub_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 4], H[ 4] ), \
                               _mm256_xor_si256( M[ 0], H[ 0] ) ), \
             _mm256_xor_si256( M[ 3], H[ 3] ) ), \
          _mm256_xor_si256( M[11], H[11] ) ), \
       _mm256_xor_si256( M[13], H[13] ) )

#define Wb7 \
   _mm256_sub_epi64( \
       _mm256_sub_epi64( \
          _mm256_sub_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 1], H[ 1] ), \
                               _mm256_xor_si256( M[ 4], H[ 4] ) ), \
             _mm256_xor_si256( M[ 5], H[ 5] ) ), \
          _mm256_xor_si256( M[12], H[12] ) ), \
       _mm256_xor_si256( M[14], H[14] ) )

#define Wb8 \
   _mm256_sub_epi64( \
       _mm256_add_epi64( \
          _mm256_sub_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 2], H[ 2] ), \
                               _mm256_xor_si256( M[ 5], H[ 5] ) ), \
             _mm256_xor_si256( M[ 6], H[ 6] ) ), \
          _mm256_xor_si256( M[13], H[13] ) ), \
       _mm256_xor_si256( M[15], H[15] ) )

#define Wb9 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_add_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 0], H[ 0] ), \
                               _mm256_xor_si256( M[ 3], H[ 3] ) ), \
             _mm256_xor_si256( M[ 6], H[ 6] ) ), \
          _mm256_xor_si256( M[ 7], H[ 7] ) ), \
       _mm256_xor_si256( M[14], H[14] ) )

#define Wb10 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_sub_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 8], H[ 8] ), \
                               _mm256_xor_si256( M[ 1], H[ 1] ) ), \
             _mm256_xor_si256( M[ 4], H[ 4] ) ), \
          _mm256_xor_si256( M[ 7], H[ 7] ) ), \
       _mm256_xor_si256( M[15], H[15] ) )

#define Wb11 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_sub_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 8], H[ 8] ), \
                               _mm256_xor_si256( M[ 0], H[ 0] ) ), \
             _mm256_xor_si256( M[ 2], H[ 2] ) ), \
          _mm256_xor_si256( M[ 5], H[ 5] ) ), \
       _mm256_xor_si256( M[ 9], H[ 9] ) )

#define Wb12 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_sub_epi64( \
             _mm256_add_epi64( _mm256_xor_si256( M[ 1], H[ 1] ), \
                               _mm256_xor_si256( M[ 3], H[ 3] ) ), \
             _mm256_xor_si256( M[ 6], H[ 6] ) ), \
          _mm256_xor_si256( M[ 9], H[ 9] ) ), \
       _mm256_xor_si256( M[10], H[10] ) )

#define Wb13 \
   _mm256_add_epi64( \
       _mm256_add_epi64( \
          _mm256_add_epi64( \
             _mm256_add_epi64( _mm256_xor_si256( M[ 2], H[ 2] ), \
                               _mm256_xor_si256( M[ 4], H[ 4] ) ), \
             _mm256_xor_si256( M[ 7], H[ 7] ) ), \
          _mm256_xor_si256( M[10], H[10] ) ), \
       _mm256_xor_si256( M[11], H[11] ) )

#define Wb14 \
   _mm256_sub_epi64( \
       _mm256_sub_epi64( \
          _mm256_add_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[ 3], H[ 3] ), \
                               _mm256_xor_si256( M[ 5], H[ 5] ) ), \
             _mm256_xor_si256( M[ 8], H[ 8] ) ), \
          _mm256_xor_si256( M[11], H[11] ) ), \
       _mm256_xor_si256( M[12], H[12] ) )

#define Wb15 \
   _mm256_add_epi64( \
       _mm256_sub_epi64( \
          _mm256_sub_epi64( \
             _mm256_sub_epi64( _mm256_xor_si256( M[12], H[12] ), \
                               _mm256_xor_si256( M[ 4], H[4] ) ), \
             _mm256_xor_si256( M[ 6], H[ 6] ) ), \
          _mm256_xor_si256( M[ 9], H[ 9] ) ), \
       _mm256_xor_si256( M[13], H[13] ) )

void compress_big( const __m256i *M, const __m256i H[16], __m256i dH[16] )
{
   __m256i qt[32], xl, xh;

   qt[ 0] = _mm256_add_epi64( sb0( Wb0 ), H[ 1] ); 
   qt[ 1] = _mm256_add_epi64( sb1( Wb1 ), H[ 2] ); 
   qt[ 2] = _mm256_add_epi64( sb2( Wb2 ), H[ 3] ); 
   qt[ 3] = _mm256_add_epi64( sb3( Wb3 ), H[ 4] ); 
   qt[ 4] = _mm256_add_epi64( sb4( Wb4 ), H[ 5] ); 
   qt[ 5] = _mm256_add_epi64( sb0( Wb5 ), H[ 6] ); 
   qt[ 6] = _mm256_add_epi64( sb1( Wb6 ), H[ 7] ); 
   qt[ 7] = _mm256_add_epi64( sb2( Wb7 ), H[ 8] ); 
   qt[ 8] = _mm256_add_epi64( sb3( Wb8 ), H[ 9] ); 
   qt[ 9] = _mm256_add_epi64( sb4( Wb9 ), H[10] ); 
   qt[10] = _mm256_add_epi64( sb0( Wb10), H[11] ); 
   qt[11] = _mm256_add_epi64( sb1( Wb11), H[12] ); 
   qt[12] = _mm256_add_epi64( sb2( Wb12), H[13] ); 
   qt[13] = _mm256_add_epi64( sb3( Wb13), H[14] );
   qt[14] = _mm256_add_epi64( sb4( Wb14), H[15] ); 
   qt[15] = _mm256_add_epi64( sb0( Wb15), H[ 0] ); 
   qt[16] = expand1b( qt, M, H, 16 ); 
   qt[17] = expand1b( qt, M, H, 17 ); 
   qt[18] = expand2b( qt, M, H, 18 ); 
   qt[19] = expand2b( qt, M, H, 19 ); 
   qt[20] = expand2b( qt, M, H, 20 ); 
   qt[21] = expand2b( qt, M, H, 21 ); 
   qt[22] = expand2b( qt, M, H, 22 ); 
   qt[23] = expand2b( qt, M, H, 23 ); 
   qt[24] = expand2b( qt, M, H, 24 ); 
   qt[25] = expand2b( qt, M, H, 25 ); 
   qt[26] = expand2b( qt, M, H, 26 ); 
   qt[27] = expand2b( qt, M, H, 27 ); 
   qt[28] = expand2b( qt, M, H, 28 ); 
   qt[29] = expand2b( qt, M, H, 29 ); 
   qt[30] = expand2b( qt, M, H, 30 ); 
   qt[31] = expand2b( qt, M, H, 31 ); 

   xl = _mm256_xor_si256( 
              _mm256_xor_si256( _mm256_xor_si256( qt[16], qt[17] ), 
                                _mm256_xor_si256( qt[18], qt[19] ) ), 
              _mm256_xor_si256( _mm256_xor_si256( qt[20], qt[21] ), 
                                _mm256_xor_si256( qt[22], qt[23] ) ) ); 
   xh = _mm256_xor_si256( xl, 
             _mm256_xor_si256( 
                 _mm256_xor_si256( _mm256_xor_si256( qt[24], qt[25] ),
                                   _mm256_xor_si256( qt[26], qt[27] ) ),
                 _mm256_xor_si256( _mm256_xor_si256( qt[28], qt[29] ),
                                   _mm256_xor_si256( qt[30], qt[31] ) )));

   dH[ 0] = _mm256_add_epi64(
                 _mm256_xor_si256( M[0],
                      _mm256_xor_si256( _mm256_slli_epi64( xh, 5 ),
                                        _mm256_srli_epi64( qt[16], 5 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[24] ), qt[ 0] ));
   dH[ 1] = _mm256_add_epi64(
                 _mm256_xor_si256( M[1],
                      _mm256_xor_si256( _mm256_srli_epi64( xh, 7 ),
                                        _mm256_slli_epi64( qt[17], 8 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[25] ), qt[ 1] ));
   dH[ 2] = _mm256_add_epi64(
                 _mm256_xor_si256( M[2],
                      _mm256_xor_si256( _mm256_srli_epi64( xh, 5 ),
                                        _mm256_slli_epi64( qt[18], 5 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[26] ), qt[ 2] ));
   dH[ 3] = _mm256_add_epi64(
                 _mm256_xor_si256( M[3],
                      _mm256_xor_si256( _mm256_srli_epi64( xh, 1 ),
                                        _mm256_slli_epi64( qt[19], 5 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[27] ), qt[ 3] ));
   dH[ 4] = _mm256_add_epi64(
                 _mm256_xor_si256( M[4],
                      _mm256_xor_si256( _mm256_srli_epi64( xh, 3 ),
                                        _mm256_slli_epi64( qt[20], 0 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[28] ), qt[ 4] ));
   dH[ 5] = _mm256_add_epi64(
                 _mm256_xor_si256( M[5],
                      _mm256_xor_si256( _mm256_slli_epi64( xh, 6 ),
                                        _mm256_srli_epi64( qt[21], 6 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[29] ), qt[ 5] ));
   dH[ 6] = _mm256_add_epi64(
                 _mm256_xor_si256( M[6],
                      _mm256_xor_si256( _mm256_srli_epi64( xh, 4 ),
                                        _mm256_slli_epi64( qt[22], 6 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[30] ), qt[ 6] ));
   dH[ 7] = _mm256_add_epi64(
                 _mm256_xor_si256( M[7],
                      _mm256_xor_si256( _mm256_srli_epi64( xh, 11 ),
                                        _mm256_slli_epi64( qt[23], 2 ) ) ),
                 _mm256_xor_si256( _mm256_xor_si256( xl, qt[31] ), qt[ 7] ));
   dH[ 8] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[4], 9 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[24] ), M[ 8] )),
                 _mm256_xor_si256( _mm256_slli_epi64( xl, 8 ),
                                   _mm256_xor_si256( qt[23], qt[ 8] ) ) );
   dH[ 9] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[5], 10 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[25] ), M[ 9] )),
                 _mm256_xor_si256( _mm256_srli_epi64( xl, 6 ),
                                   _mm256_xor_si256( qt[16], qt[ 9] ) ) );
   dH[10] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[6], 11 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[26] ), M[10] )),
                 _mm256_xor_si256( _mm256_slli_epi64( xl, 6 ),
                                   _mm256_xor_si256( qt[17], qt[10] ) ) );
   dH[11] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[7], 12 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[27] ), M[11] )),
                 _mm256_xor_si256( _mm256_slli_epi64( xl, 4 ),
                                   _mm256_xor_si256( qt[18], qt[11] ) ) );
   dH[12] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[0], 13 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[28] ), M[12] )),
                 _mm256_xor_si256( _mm256_srli_epi64( xl, 3 ),
                                   _mm256_xor_si256( qt[19], qt[12] ) ) );
   dH[13] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[1], 14 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[29] ), M[13] )),
                 _mm256_xor_si256( _mm256_srli_epi64( xl, 4 ),
                                   _mm256_xor_si256( qt[20], qt[13] ) ) );
   dH[14] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[2], 15 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[30] ), M[14] )),
                 _mm256_xor_si256( _mm256_srli_epi64( xl, 7 ),
                                   _mm256_xor_si256( qt[21], qt[14] ) ) );
   dH[15] = _mm256_add_epi64( _mm256_add_epi64(
                 mm256_rol_64( dH[3], 16 ),
                 _mm256_xor_si256( _mm256_xor_si256( xh, qt[31] ), M[15] )),
                 _mm256_xor_si256( _mm256_srli_epi64( xl, 2 ),
                                   _mm256_xor_si256( qt[22], qt[15] ) ) );
} 

static const __m256i final_b[16] =
{
   { 0xaaaaaaaaaaaaaaa0, 0xaaaaaaaaaaaaaaa0,
     0xaaaaaaaaaaaaaaa0, 0xaaaaaaaaaaaaaaa0 },
   { 0xaaaaaaaaaaaaaaa1, 0xaaaaaaaaaaaaaaa1,
     0xaaaaaaaaaaaaaaa1, 0xaaaaaaaaaaaaaaa1 },
   { 0xaaaaaaaaaaaaaaa2, 0xaaaaaaaaaaaaaaa2,
     0xaaaaaaaaaaaaaaa2, 0xaaaaaaaaaaaaaaa2 },
   { 0xaaaaaaaaaaaaaaa3, 0xaaaaaaaaaaaaaaa3,
     0xaaaaaaaaaaaaaaa3, 0xaaaaaaaaaaaaaaa3 },
   { 0xaaaaaaaaaaaaaaa4, 0xaaaaaaaaaaaaaaa4,
     0xaaaaaaaaaaaaaaa4, 0xaaaaaaaaaaaaaaa4 },
   { 0xaaaaaaaaaaaaaaa5, 0xaaaaaaaaaaaaaaa5,
     0xaaaaaaaaaaaaaaa5, 0xaaaaaaaaaaaaaaa5 },
   { 0xaaaaaaaaaaaaaaa6, 0xaaaaaaaaaaaaaaa6,
     0xaaaaaaaaaaaaaaa6, 0xaaaaaaaaaaaaaaa6 },
   { 0xaaaaaaaaaaaaaaa7, 0xaaaaaaaaaaaaaaa7,
     0xaaaaaaaaaaaaaaa7, 0xaaaaaaaaaaaaaaa7 },
   { 0xaaaaaaaaaaaaaaa8, 0xaaaaaaaaaaaaaaa8,
     0xaaaaaaaaaaaaaaa8, 0xaaaaaaaaaaaaaaa8 },
   { 0xaaaaaaaaaaaaaaa9, 0xaaaaaaaaaaaaaaa9,
     0xaaaaaaaaaaaaaaa9, 0xaaaaaaaaaaaaaaa9 },
   { 0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa,
     0xaaaaaaaaaaaaaaaa, 0xaaaaaaaaaaaaaaaa },
   { 0xaaaaaaaaaaaaaaab, 0xaaaaaaaaaaaaaaab,
     0xaaaaaaaaaaaaaaab, 0xaaaaaaaaaaaaaaab },
   { 0xaaaaaaaaaaaaaaac, 0xaaaaaaaaaaaaaaac,
     0xaaaaaaaaaaaaaaac, 0xaaaaaaaaaaaaaaac },
   { 0xaaaaaaaaaaaaaaad, 0xaaaaaaaaaaaaaaad,
     0xaaaaaaaaaaaaaaad, 0xaaaaaaaaaaaaaaad },
   { 0xaaaaaaaaaaaaaaae, 0xaaaaaaaaaaaaaaae,
     0xaaaaaaaaaaaaaaae, 0xaaaaaaaaaaaaaaae },
   { 0xaaaaaaaaaaaaaaaf, 0xaaaaaaaaaaaaaaaf,
     0xaaaaaaaaaaaaaaaf, 0xaaaaaaaaaaaaaaaf }
};

static void
bmw64_4way_init( bmw_4way_big_context *sc, const sph_u64 *iv )
{
   for ( int i = 0; i < 16; i++ )
      sc->H[i] = _mm256_set1_epi64x( iv[i] );
   sc->ptr = 0;
   sc->bit_count = 0;
}

static void
bmw64_4way( bmw_4way_big_context *sc, const void *data, size_t len )
{
   __m256i *vdata = (__m256i*)data;
   __m256i *buf;
   __m256i htmp[16];
   __m256i *h1, *h2;
   size_t ptr;
   const int buf_size = 128;  // bytes of one lane, compatible with len

   sc->bit_count += (sph_u64)len << 3;
   buf = sc->buf;
   ptr = sc->ptr;
   h1 = sc->H;
   h2 = htmp;
   while ( len > 0 )
   {
      size_t clen;
      clen = buf_size - ptr;
      if ( clen > len )
         clen = len;
      memcpy_256( buf + (ptr>>3), vdata, clen >> 3 );
      vdata = vdata + (clen>>3);
      len -= clen;
      ptr += clen;
      if ( ptr == buf_size )
      {
         __m256i *ht;
         compress_big( buf, h1, h2 );
         ht = h1;
         h1 = h2;
         h2 = ht;
         ptr = 0;
      }
   }
   sc->ptr = ptr;
   if ( h1 != sc->H )
        memcpy_256( sc->H, h1, 16 );
}

static void
bmw64_4way_close(bmw_4way_big_context *sc, unsigned ub, unsigned n,
	void *dst, size_t out_size_w64)
{
   __m256i *buf;
   __m256i h1[16], h2[16], *h;
   size_t ptr, u, v;
   unsigned z;
   const int buf_size = 128;  // bytes of one lane, compatible with len

   buf = sc->buf;
   ptr = sc->ptr;
   z = 0x80 >> n;
   buf[ ptr>>3 ] = _mm256_set1_epi64x( z );
   ptr += 8;
   h = sc->H;

   if (  ptr > (buf_size - 8) )
   {
      memset_zero_256( buf + (ptr>>3), (buf_size - ptr) >> 3 );
      compress_big( buf, h, h1 );
      ptr = 0;
      h = h1;
   }
   memset_zero_256( buf + (ptr>>3), (buf_size - 8 - ptr) >> 3 );
   buf[ (buf_size - 8) >> 3 ] = _mm256_set1_epi64x( sc->bit_count + n );
   compress_big( buf, h, h2 );
   for ( u = 0; u < 16; u ++ )
      buf[u] = h2[u];
   compress_big( buf, final_b, h1 );
   for (u = 0, v = 16 - out_size_w64; u < out_size_w64; u ++, v ++)
      casti_m256i(dst,u) = h1[v];
}

void
bmw512_4way_init(void *cc)
{
	bmw64_4way_init(cc, IV512);
}

void
bmw512_4way(void *cc, const void *data, size_t len)
{
	bmw64_4way(cc, data, len);
}

void
bmw512_4way_close(void *cc, void *dst)
{
	bmw512_4way_addbits_and_close(cc, 0, 0, dst);
}

void
bmw512_4way_addbits_and_close(void *cc, unsigned ub, unsigned n, void *dst)
{
	bmw64_4way_close(cc, ub, n, dst, 8);
}

#endif  // __AVX2__

#ifdef __cplusplus
}
#endif

