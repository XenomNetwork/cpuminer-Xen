#if !defined(SIMD_MMX_H__)
#define SIMD_MMX_H__ 1

#if defined(__MMX__)

////////////////////////////////////////////////////////////////
//
//               64 bit MMX vectors.
//
// There are rumours MMX wil be removed. Although casting with int64
// works there is likely some overhead to move the data to An MMX register
// and back.


// Pseudo constants
#define m64_zero   _mm_setzero_si64()
#define m64_one_64 _mm_set_pi32(  0UL, 1UL )
#define m64_one_32 _mm_set1_pi32( 1UL )
#define m64_one_16 _mm_set1_pi16( 1U )
#define m64_one_8  _mm_set1_pi8(  1U );
#define m64_neg1   _mm_set1_pi32( 0xFFFFFFFFUL )
/* cast also works, which is better?
#define m64_zero   ( (__m64)0ULL )
#define m64_one_64 ( (__m64)1ULL )
#define m64_one_32 ( (__m64)0x0000000100000001ULL )
#define m64_one_16 ( (__m64)0x0001000100010001ULL )
#define m64_one_8  ( (__m64)0x0101010101010101ULL )
#define m64_neg1   ( (__m64)0xFFFFFFFFFFFFFFFFULL )
*/


#define casti_m64(p,i) (((__m64*)(p))[(i)])

// cast all arguments as the're likely to be uint64_t

// Bitwise not: ~(a)
#define mm64_not( a ) _mm_xor_si64( (__m64)a, m64_neg1 )

// Unary negate elements
#define mm64_negate_32( v ) _mm_sub_pi32( m64_zero, (__m64)v )
#define mm64_negate_16( v ) _mm_sub_pi16( m64_zero, (__m64)v )
#define mm64_negate_8(  v ) _mm_sub_pi8(  m64_zero, (__m64)v )

// Rotate bits in packed elements of 64 bit vector
#define mm64_rol_32( a, n ) \
   _mm_or_si64( _mm_slli_pi32( (__m64)(a), n ), \
                _mm_srli_pi32( (__m64)(a), 32-(n) ) )

#define mm64_ror_32( a, n ) \
   _mm_or_si64( _mm_srli_pi32( (__m64)(a), n ), \
                _mm_slli_pi32( (__m64)(a), 32-(n) ) )

#define mm64_rol_16( a, n ) \
   _mm_or_si64( _mm_slli_pi16( (__m64)(a), n ), \
                _mm_srli_pi16( (__m64)(a), 16-(n) ) )

#define mm64_ror_16( a, n ) \
   _mm_or_si64( _mm_srli_pi16( (__m64)(a), n ), \
                _mm_slli_pi16( (__m64)(a), 16-(n) ) )

// Rotate packed elements accross lanes. Useful for byte swap and byte
// rotation.

// _mm_shuffle_pi8 requires SSSE3 while _mm_shuffle_pi16 requires SSE
// even though these are MMX instructions.

// Swap hi & lo 32 bits.
#define mm64_swap32( a )      _mm_shuffle_pi16( (__m64)(a), 0x4e )

#define mm64_ror1x16_64( a )  _mm_shuffle_pi16( (__m64)(a), 0x39 ) 
#define mm64_rol1x16_64( a )  _mm_shuffle_pi16( (__m64)(a), 0x93 ) 

// Swap hi & lo 16 bits of each 32 bit element
#define mm64_swap16_32( a )  _mm_shuffle_pi16( (__m64)(a), 0xb1 )

#if defined(__SSSE3__)

// Endian byte swap packed elements
// A vectorized version of the u64 bswap, use when data already in MMX reg.
#define mm64_bswap_64( v ) \
    _mm_shuffle_pi8( (__m64)v, _mm_set_pi8( 0,1,2,3,4,5,6,7 ) )

#define mm64_bswap_32( v ) \
    _mm_shuffle_pi8( (__m64)v, _mm_set_pi8( 4,5,6,7,  0,1,2,3 ) )

/*
#define mm64_bswap_16( v ) \
    _mm_shuffle_pi8( (__m64)v, _mm_set_pi8( 6,7,  4,5,  2,3,  0,1 ) );
*/

#else

#define mm64_bswap_64( v ) \
       (__m64)__builtin_bswap64( (uint64_t)v )

// This exists only for compatibility with CPUs without SSSE3. MMX doesn't
// have extract 32 instruction so pointers are needed to access elements.
// It' more efficient for the caller to use scalar variables and call
// bswap_32 directly.
#define mm64_bswap_32( v ) \
   _mm_set_pi32( __builtin_bswap32( ((uint32_t*)&v)[1] ), \
                 __builtin_bswap32( ((uint32_t*)&v)[0] )  )

#endif

// Invert vector: {3,2,1,0} -> {0,1,2,3}
// Invert_64 is the same as bswap64
// Invert_32 is the same as swap32

#define mm64_invert_16( v ) _mm_shuffle_pi16( (__m64)v, 0x1b )

#if defined(__SSSE3__)

// An SSE2 or MMX version of this would be monstrous, shifting, masking and
// oring each byte individually.
#define mm64_invert_8(  v ) \
    _mm_shuffle_pi8( (__m64)v, _mm_set_pi8( 0,1,2,3,4,5,6,7 ) );

#endif

// 64 bit mem functions use integral sizes instead of bytes, data must
// be aligned to 64 bits.
static inline void memcpy_m64( __m64 *dst, const __m64 *src, int n )
{   for ( int i = 0; i < n; i++ ) dst[i] = src[i]; }

static inline void memset_zero_m64( __m64 *src, int n )
{   for ( int i = 0; i < n; i++ ) src[i] = (__m64)0ULL; }

static inline void memset_m64( __m64 *dst, const __m64 a,  int n )
{   for ( int i = 0; i < n; i++ ) dst[i] = a; }

#endif // MMX

#endif // SIMD_MMX_H__

