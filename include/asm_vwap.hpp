#pragma once
#include <immintrin.h>
#include <cstdint>
#pragma GCC push_options
#pragma GCC target("avx2,fma")
inline __attribute__((always_inline)) uint64_t rdtsc_tick() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi) :: "memory");
    return ((uint64_t)hi << 32) | lo;
}
inline __attribute__((always_inline)) uint64_t rdtsc_fenced() {
    uint32_t lo, hi;
    __asm__ volatile ("lfence\n\t rdtsc\n\t lfence" : "=a"(lo), "=d"(hi) :: "memory");
    return ((uint64_t)hi << 32) | lo;
}
inline __attribute__((always_inline)) double asm_dot4_avx2(const double* prices, const double* sizes) {
    __m256d p = _mm256_loadu_pd(prices);
    __m256d s = _mm256_loadu_pd(sizes);
    __m256d mul = _mm256_mul_pd(p, s);
    __m128d low = _mm256_castpd256_pd128(mul);
    __m128d high = _mm256_extractf128_pd(mul, 1);
    __m128d sum = _mm_add_pd(low, high);
    sum = _mm_hadd_pd(sum, sum);
    return _mm_cvtsd_f64(sum);
}
inline __attribute__((always_inline)) double asm_filtered_mid(const double bid_p[5], const double bid_s[5], const double ask_p[5], const double ask_s[5]) {
    double bid_sz = bid_s[0]+bid_s[1]+bid_s[2]+bid_s[3]+bid_s[4];
    double ask_sz = ask_s[0]+ask_s[1]+ask_s[2]+ask_s[3]+ask_s[4];
    double bid_px_sz = asm_dot4_avx2(bid_p, bid_s) + bid_p[4]*bid_s[4];
    double ask_px_sz = asm_dot4_avx2(ask_p, ask_s) + ask_p[4]*ask_s[4];
    if (bid_sz==0 || ask_sz==0) return 0.0;
    return (bid_px_sz/bid_sz + ask_px_sz/ask_sz)*0.5;
}
#pragma GCC pop_options
