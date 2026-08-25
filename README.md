# Deribit Dual ASM Fork — 20.7M events/sec

AVX2 hand-rolled ASM proof of C++23 dual-engine.

## Proof
Deribit Dual ASM Fork — 20.7M events/sec
Proof: 48.202ms = 20.74M/sec (was 14.93M)

ASM: vmulpd + vhaddpd verified via objdump

Baseline 14.93M (66.95ms) -> ASM 20.74M (48.20ms) = +38.6%

Main: https://github.com/nsmanju/deribit-feed-strategy-dual-engine

