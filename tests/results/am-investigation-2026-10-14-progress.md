# AM investigation progress, 14 October 2026

The approved profiling-only hardware run is complete and fails timing. Its detailed evidence is in am-iq-profile-v1-2026-10-14-run01.{txt,json,md}. The original valid baseline run02 and CH1-off run01 remain preserved; run01 is excluded. User confirmed profiling controls responsive and P1/P4 captured; photos were not supplied.

The optimized candidate was approved, flashed and tested. The 62.050189-second device interval passed: 14543 blocks, 234.374790 blocks/s, no new faults or near-rail samples, all-block maximum 4086 us against 4266 us. Headroom is limited to 180 us (4.22%). See am-iq-optimized-v1-2026-10-14-run01.md for the final report.

## Prepared changes

CIC integrators stay in local variables for each group of 16 ADC words. Both axes still receive the original real/zero interleaving, all sixteen integrator updates, and the same modulo-32-bit arithmetic and comb output. The filter response, DC tracking, tuning, gain and demodulation calculations are retained.

ADC statistics accumulate four bounded 32-bit square sums per channel and combine them in 64 bits. Every sample is masked, measured and checked for near-rail conditions. Original mean/RMS calculations are retained.

Spectrum preparation reuses exact log10f results for integer arguments 1..32767, initialized before acquisition. This is input-independent constant precomputation; all signal-dependent spectrum preparation remains inside the existing timing window and runs at its original cadence. Arguments 32768..65535 retain the original log10f fallback. RAM cost is 128 KiB; startup is longer. High-magnitude fallback timing is not yet established. No approximation, global fast-math, rate change, overclock or diagnostics removal.

The all-block measurement instrumentation from the profiling build remains, separating ordinary and spectrum blocks and preserving cumulative counters. Nine receiver timer reads and profiling stores are inside the inclusive window. Diagnostic window bookkeeping is measured separately outside it; no signal processing is moved there.

## Validation

37 receiver scenarios, 512 blocks each, compare fresh-process reference and optimized outputs. PCM maximum/RMS differences, clipping disagreements, magnitude differences, spectrum pixel/scale differences are all zero. Cases include AM depth/amplitude/DC/silence/rails/transients, other receiver modes and processing switches, swapped axes/IQ correction, real-I/real-Q preparation, tuning limits, bandwidths and swap/tune transition. See am-optimized-v1-expanded-numerical-2026-10-14.{json,csv} and waveform CSV. Settled AM depth tests recover approximately 1000 Hz identically on the host; this is not hardware demodulation validation.

20,000 input-statistics blocks match the original masked samples, mean, RMS, minima/maxima and near-rail counts exactly. All 65,535 nonzero uint16 log arguments match the host log10f bit pattern, including the fallback. Timing-window and front-panel tests pass. Rebuilt BASIC comparison reproduces its saved CSV exactly; BASIC kernel is unchanged. Host libraries/shims do not prove Pico floating-point equivalence or IRQ/DMA behavior.

Release build passes, Arm GCC 13.3.1 / Pico SDK 2.1.0, BENCH_SELF_TEST=OFF, BENCH_OPERATION_BENCHMARK=OFF, -O3 and -fwrapv. Linked static allocation ends at 0x20031104; heap capacity is 61,180 bytes. Known queue payload allocations use 51,929 bytes, leaving 9,251 bytes before allocator overhead and other library allocations. This is a static budget, not a runtime free-heap measurement. Core1's 16 KiB stack is included in static allocation; core0 uses its separate 4 KiB scratch bank.

## Prepared identity and next step

Base commit: 34bde79a126fc4a7737a4e7194b63833e2aace05, branch testing/dsp-suite, plus local changes.
Build label: am-optimized-v1.
Source fingerprint: 86d0e4fe2bd42cc00d51896696ec6402e11beee3f1282a7385d0eff4441f9497.
UF2 SHA256: 0bca44e437d8579749c61407390c5f12f947c87a45c53818ebc00163f6768f92.
Generated build/adc_dsp_standalone.uf2 and firmware/adc_dsp_suite.uf2 match.

Approved USB BOOTSEL flash and fresh-boot AM-IQ run completed. Controls were confirmed responsive; optimized-run P1/P4 photos were not captured. Fresh scope measurements were not supplied. No demodulation-quality claim. Work remains local; no commit or push.
