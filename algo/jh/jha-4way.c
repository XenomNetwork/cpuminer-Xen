#include "jha-gate.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
//#include "avxdefs.h"

#if defined(JHA_4WAY)

#include "algo/blake/blake-hash-4way.h"
#include "algo/skein/skein-hash-4way.h"
#include "algo/jh/jh-hash-4way.h"
#include "algo/keccak/keccak-hash-4way.h"
#include "algo/groestl/aes_ni/hash-groestl.h"

//static __thread keccak512_4way_context jha_kec_mid
//                                   __attribute__ ((aligned (64)));

void jha_hash_4way( void *out, const void *input )
{
    uint64_t hash0[8] __attribute__ ((aligned (64)));
    uint64_t hash1[8] __attribute__ ((aligned (64)));
    uint64_t hash2[8] __attribute__ ((aligned (64)));
    uint64_t hash3[8] __attribute__ ((aligned (64)));
    uint64_t vhash[8*4] __attribute__ ((aligned (64)));
    uint64_t vhashA[8*4] __attribute__ ((aligned (64)));
    uint64_t vhashB[8*4] __attribute__ ((aligned (64)));
    __m256i* vh  = (__m256i*)vhash;
    __m256i* vhA = (__m256i*)vhashA;
    __m256i* vhB = (__m256i*)vhashB;
    __m256i vh_mask;

    blake512_4way_context  ctx_blake;
    hashState_groestl      ctx_groestl;
    jh512_4way_context     ctx_jh;
    skein512_4way_context  ctx_skein;
    keccak512_4way_context ctx_keccak;

    keccak512_4way_init( &ctx_keccak );
    keccak512_4way( &ctx_keccak, input, 80 );
    keccak512_4way_close( &ctx_keccak, vhash );

    // Heavy & Light Pair Loop
    for ( int round = 0; round < 3; round++ )
    {
       vh_mask = _mm256_cmpeq_epi64( _mm256_and_si256(
               vh[0], _mm256_set1_epi64x( 1 ) ), m256_zero );

       mm256_dintrlv_4x64( hash0, hash1, hash2, hash3, vhash, 512 );
       init_groestl( &ctx_groestl, 64 );
       update_and_final_groestl( &ctx_groestl, (char*)hash0,
                                               (char*)hash0, 512 );
       init_groestl( &ctx_groestl, 64 );
       update_and_final_groestl( &ctx_groestl, (char*)hash1,
                                               (char*)hash1, 512 );
       init_groestl( &ctx_groestl, 64 );
       update_and_final_groestl( &ctx_groestl, (char*)hash2,
                                               (char*)hash2, 512 );
       init_groestl( &ctx_groestl, 64 );
       update_and_final_groestl( &ctx_groestl, (char*)hash3,
                                               (char*)hash3, 512 );
       mm256_intrlv_4x64( vhashA, hash0, hash1, hash2, hash3, 512 );

       skein512_4way_init( &ctx_skein );
       skein512_4way( &ctx_skein, vhash, 64 );
       skein512_4way_close( &ctx_skein, vhashB );

       for ( int i = 0; i < 8; i++ )
          vh[i] = _mm256_blendv_epi8( vhA[i], vhB[i], vh_mask );

       blake512_4way_init( &ctx_blake );
       blake512_4way( &ctx_blake, vhash, 64 );
       blake512_4way_close( &ctx_blake, vhashA );

       jh512_4way_init( &ctx_jh );
       jh512_4way( &ctx_jh, vhash, 64 );
       jh512_4way_close( &ctx_jh, vhashB );

       for ( int i = 0; i < 8; i++ )
          casti_m256i( out, i ) = _mm256_blendv_epi8( vhA[i], vhB[i], vh_mask );
    }

//    mm256_dintrlv_4x64( out, out+32, out+64, out+96, vhash, 256 );
}

int scanhash_jha_4way( struct work *work, uint32_t max_nonce,
                       uint64_t *hashes_done, struct thr_info *mythr )
{
   uint32_t hash[8*4] __attribute__ ((aligned (64)));
   uint32_t vdata[20*4] __attribute__ ((aligned (64)));
    uint32_t edata[20] __attribute__ ((aligned (64)));
   uint32_t *hash7 = &(hash[25]);
   uint32_t lane_hash[8] __attribute__ ((aligned (32)));
   uint32_t *pdata = work->data;
   uint32_t *ptarget = work->target;
   const uint32_t first_nonce = pdata[19];
   const uint32_t Htarg = ptarget[7];
   uint32_t n = pdata[19];
    __m256i  *noncev = (__m256i*)vdata + 9;   // aligned
   int thr_id = mythr->id;  // thr_id arg is deprecated

   uint64_t htmax[] = {
		0,
		0xF,
		0xFF,
		0xFFF,
		0xFFFF,
		0x10000000
	};
   uint32_t masks[] = {
		0xFFFFFFFF,
		0xFFFFFFF0,
		0xFFFFFF00,
		0xFFFFF000,
		0xFFFF0000,
		0
	};

   for ( int i=0; i < 19; i++ )
      be32enc( &edata[i], pdata[i] );

   mm256_intrlv_4x64( vdata, edata, edata, edata, edata, 640 );
//   mm256_bswap_intrlv80_4x64( vdata, pdata );

   for ( int m = 0; m < 6; m++ )
   {
      if ( Htarg <= htmax[m] )
      {
         uint32_t mask = masks[m];
         do {
              *noncev = mm256_intrlv_blend_32( mm256_bswap_32(
                _mm256_set_epi32( n+3, 0, n+2, 0, n+1, 0, n, 0 ) ), *noncev );
//              be32enc( noncep,   n   );
//              be32enc( noncep+2, n+1 );
//              be32enc( noncep+4, n+2 );
//              be32enc( noncep+6, n+3 );

              jha_hash_4way( hash, vdata );
              pdata[19] = n;

//              for ( int i = 0; i < 4; i++ )
//              if ( ( !( (hash+(i<<3))[7] & mask ) == 0 )
//                  && fulltest( hash+(i<<3), ptarget ) )
              for ( int i = 0; i < 4; i++ ) if ( !( (hash7[i] & mask ) == 0 ) )
              {
                 mm256_extr_lane_4x64( lane_hash, hash, i, 256 );
                 if ( fulltest( hash+(i<<3), ptarget ) && !opt_benchmark )
                 {
                    pdata[19] = n+i;
                    submit_lane_solution( work, lane_hash, mythr, i );
//                 nonces[ num_found++ ] = n+i;
//                 work_set_target_ratio( work, hash+(i<<3) );
                 }
              }
              n += 4;
         } while ( ( n < max_nonce ) && !work_restart[thr_id].restart );
         break;
      }
   }
   *hashes_done = n - first_nonce + 1;
   return 0;
}
#endif
