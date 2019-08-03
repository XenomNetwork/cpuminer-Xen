#ifndef MYRGR_GATE_H__
#define MYRGR_GATE_H__

#include "algo-gate-api.h"
#include <stdint.h>

#if defined(__AVX2__) && defined(__AES__)
  #define MYRGR_4WAY
#endif

#if defined(MYRGR_4WAY)

void myriad_4way_hash( void *state, const void *input );

int scanhash_myriad_4way( struct work *work, uint32_t max_nonce,
                         uint64_t *hashes_done, struct thr_info *mythr );

void init_myrgr_4way_ctx();

#endif

void myriad_hash( void *state, const void *input );

int scanhash_myriad( struct work *work, uint32_t max_nonce,
                    uint64_t *hashes_done, struct thr_info *mythr );

void init_myrgr_ctx();

#endif

