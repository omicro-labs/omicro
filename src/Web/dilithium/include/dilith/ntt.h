#ifndef DL_NTT_H
#define DL_NTT_H

#include <stdint.h>
#include <dilith/params.h>

#define ntt DILITHIUM_NAMESPACE(ntt)
void ntt(int32_t a[N]);

#define invntt_tomont DILITHIUM_NAMESPACE(invntt_tomont)
void invntt_tomont(int32_t a[N]);

#endif
