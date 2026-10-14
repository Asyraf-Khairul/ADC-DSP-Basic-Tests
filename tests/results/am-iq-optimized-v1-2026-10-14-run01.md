# Optimized AM IQ timing, 14 October 2026

**PASS for the observed AM-IQ interval, with limited worst-block headroom.** No new streaming faults or near-rail samples occurred. This is not an AM demodulation-quality pass.

## Before and after

| Metric | Original valid baseline | Profiling-only baseline | Optimized v1 |
| --- | ---: | ---: | ---: |
| Host interval, seconds | 62.0724053 | 62.0422013 | 62.0511793 |
| Processed blocks | 12960 | 12922 | 14543 |
| Blocks/s | 208.788429 | 208.263077 | 234.374790 |
| Additional deadline misses | 12960 | 12922 | 0 |
| Additional ADC dropped blocks | 1602 | 1620 | 0 |
| Additional audio underruns | 1602 | 1620 | 0 |
| Other streaming faults / near-rail increases | 0 | 0 | 0 |
| All-block interval maximum, us | Unknown | 7289 | 4086 |
| Largest sampled latest total, us | 4864 | 4900 | 3358 |

Original run02 is the valid unmodified baseline; original run01 had CH1 OFF and is excluded. Original 8188 us was a pre-existing cumulative maximum, not its interval maximum. Both failed baseline logs remain preserved. Profile and optimized runs capture every block, including spectrum updates.

## Identity and procedure

Branch testing/dsp-suite; base 34bde79a126fc4a7737a4e7194b63833e2aace05 plus local changes. Label am-optimized-v1; runtime source fingerprint matched 86d0e4fe2bd42cc00d51896696ec6402e11beee3f1282a7385d0eff4441f9497. UF2 SHA256 0bca44e437d8579749c61407390c5f12f947c87a45c53818ebc00163f6768f92. Generated build/adc_dsp_standalone.uf2 matches firmware/adc_dsp_suite.uf2. Source/build snapshot: firmware/evidence/am-optimized-v1-20261014. No independent flash readback was performed.

Release, -g -O3 -DNDEBUG, -fwrapv, Arm GCC 13.3.1, Pico SDK 2.1.0; BENCH_SELF_TEST=OFF and BENCH_OPERATION_BENCHMARK=OFF. Runtime clock verified at 120000000 Hz. ADC remains 480000 words/s aggregate, 240000 samples/s/channel, 2048 words/block; audio remains 64 samples/block at 15000 samples/s. Spectrum cadence remains once every 64 receiver blocks. Deadline remains 4266 us.

Approved picotool USB BOOTSEL reset then SHA256-checked UF2 mass-storage copy; USB power retained; fresh firmware boot; no physical power cycle. Connected Pico COM8, USB serial E664A836A3704126. BASIC startup and AM transition records are retained separately; neither is included in the explicit measurement window. Preset 1 was confirmed, then at least 5.1 seconds settling elapsed. Only status queries and measurement start/stop were sent; cumulative counters were not reset.

Settings throughout: AM IQ, tune 0 Hz, BW 2, AGC 4/manual, gain exponent 4 (factor 16), volume 25%; squelch -1/off, hang 3, swap 0, IQ correction 0, de-emphasis 0, bass 0, treble 0, blanker 0, notch 0, noise reduction 0, smoothing 10, threshold 1, CW 700 Hz. Settings revision 1 stayed unchanged.

User confirmed mixer disconnected, both outputs ON, 1 kHz sine 300 mVpp High-Z +2.5 V offset, modulation off, 470/1000 ohm dividers, I GP27 Q GP26. Historical ~1.65 V mean / 240 mVpp; no fresh values supplied. Serial samples showed both inputs varying around midscale, but do not substitute for fresh calibrated scope readings.

User confirmed OLED, encoder and both buttons responsive during optimized run; P1/P4 photos not captured. No photographs were inspected.

## Explicit interval and preserved starting values

Host monotonic elapsed between first-line acknowledgement records: 62.0511793 seconds. Device block-boundary elapsed: 62.050189 seconds. The latter defines the exact counter/count window and gives 234.374790 blocks/s against 234.375 nominal. Ordinary status publishes every 32 blocks; start/stop also publish immediately. Host receipt includes USB/command delay, and the stop acknowledgement can include a later published status. Therefore use WINDOW endpoints for exact counts, not the outer status B field.

| Counter | Window start | Window end | Increase |
| --- | ---: | ---: | ---: |
| B | 1616 | 16159 | 14543 |
| deadline | 0 | 0 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |
| clips | 0 | 0 | 0 |

Device endpoint times: 7883100 and 69933289 us since boot. Existing cumulative total maximum was 4121 us at capture start and remained 4121 us at the end. It arose before the interval and is not its maximum. All starting fault/near-rail counters were zero; their later increases were also zero.

The all-block interval maximum was 4086 us, leaving 180 us (4.22%). Ordinary blocks: 14316, maximum 3805 us, zero late. Spectrum blocks: 227, maximum 4086 us, zero late. Largest sampled latest total was 3358 us; snapshots alone would miss the slowest block.

## Stage timings

| Stage | Ordinary mean / max us | Spectrum-block mean / max us |
| --- | ---: | ---: |
| TOTAL | 3336.779 / 3805 | 3969.427 / 4086 |
| IN | 553.611 / 582 | 554.251 / 560 |
| DSP | 2552.344 / 2883 | 2551.872 / 2611 |
| STAT | 34.694 / 84 | 34.564 / 42 |
| FFT | 0.000 / 0 | 631.850 / 709 |
| OUT | 195.396 / 378 | 195.700 / 221 |
| CICMIX | 498.787 / 504 | 498.802 / 503 |
| FILTER | 1600.149 / 1838 | 1599.793 / 1638 |
| DEMOD | 364.579 / 463 | 364.577 / 396 |
| CAPTURE | 2.460 / 3 | 2.471 / 3 |
| FFTFWD | 922.497 / 998 | 922.445 / 939 |
| FFTBINS | 222.567 / 429 | 222.317 / 241 |
| FFTINV | 386.576 / 433 | 386.533 / 389 |

DSP contains CICMIX, FILTER, DEMOD and CAPTURE. FILTER contains FFTFWD, FFTBINS and FFTINV; FFT is separate spectrum-display preparation. Nested stages are not additional work. Independent maxima are from potentially different blocks and must not be summed. Timings include interrupts.

Final stopped acknowledgement latest timings (this snapshot may follow the explicit endpoint):

    us=3333 max=4121 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
    TIMING us latest/max PROC=3145/3921 OUT=188/378 IN=560/584 DSP=2550/2883 STAT=35/85 FFT=0/788
    RX latest_us CICMIX=499 FILTER=1594 DEMOD=366 CAPTURE=3 FFTFWD=922 FFTBINS=219 FFTINV=385

## What changed and measured effect

| Stage / block class | Profiling mean us | Optimized mean us | Reduction |
| --- | ---: | ---: | ---: |
| IN / ordinary | 701.506 | 553.611 | 21.08% |
| CICMIX / ordinary | 1744.914 | 498.787 | 71.41% |
| DSP / ordinary | 3801.389 | 2552.344 | 32.86% |
| FILTER / ordinary | 1617.707 | 1600.149 | 1.09% |
| FFT / spectrum | 1785.569 | 631.850 | 64.61% |
| TOTAL / ordinary | 4736.159 | 3336.779 | 29.55% |
| TOTAL / spectrum | 6523.668 | 3969.427 | 39.15% |

CIC processing keeps integrator state local across groups of sixteen ADC words, preserving all samples, original zero interleaving, integrator/comb updates and modulo-32-bit arithmetic. ADC statistics use four bounded uint32 square sums combined in uint64; original masking, mean/RMS and per-sample rail checks remain. Spectrum display uses precomputed exact target log10f values for integers 1..32767; larger values retain log10f. Constant initialization occurs before acquisition; every input-dependent spectrum calculation remains in the original inclusive timing window. No signal processing, filtering or diagnostics were disabled or moved outside it. The stage reductions support these mechanisms; this was a combined-build comparison, not isolated hardware A/B trials of each change.

The log cache costs 128 KiB and adds startup work. The linked heap capacity is 61180 bytes; known queue payload allocations total 51929, leaving 9251 before allocator overhead and any other library allocations. This is a static budget, not measured free heap. Spectrum fallback timing at high magnitudes remains uncharacterized.

Instrumentation stayed enabled: nine extra receiver timer reads/block, all included in the inclusive processing/audio window. Startup measured 87 us for 1024 reads plus loop/store overhead (~0.085 us/read, ~0.765 us for nine); this estimates primitive cost, not a full instrumentation A/B. Separate all-block bookkeeping averaged 4.135 us and peaked at 65 us, including interrupts. It occurs outside the retained processing/audio timing window and performs diagnostic work only. Any resulting streaming losses would still be counted; none occurred.

## Numerical checks and limits

37 scenarios x 512 blocks against frozen receiver reference files: bit-identical PCM, magnitude, spectrum pixels and scale. PCM max/RMS errors and clipping disagreements were zero; settled amplitude differences were zero where defined. AM depths 10/30/60/90% recovered approximately 1000 Hz identically in host tests, with waveform samples saved. Coverage includes amplitudes/DC/silence/rails/transients, all receiver modes with processing switches, swapped I/Q, real-I/Q preparation, tuning limits, bandwidths and a swap/tune transition. Host math/shims do not prove target numerical equivalence or physical demodulation quality.

20,000 ADC-statistics blocks matched the original exactly; all 65,535 nonzero uint16 logarithm inputs matched host log10f. BASIC numerical output reproduced its earlier CSV; timing-window and front-panel tests passed. BASIC was observed briefly during startup but did not receive a new 60-second hardware run. Other receiver modes received numerical regressions only. Full numerical JSON/CSV/waveforms: am-optimized-v1-expanded-numerical-2026-10-14.*. Software checks: am-optimized-v1-software-checks-2026-10-14.json.

The observed timing pass has modest 4.22% worst-block headroom, so it is not a universal real-time guarantee. FFT filtering is now the dominant remaining ordinary DSP stage (~1.60 ms); it is the next focused optimization target if more margin is required. Longer runs, different levels, log fallback and other modes still need separate measurements.

Next demodulation test: a known amplitude-modulated carrier with a 1 kHz modulation tone and controlled depths (e.g. 10%, 30%, 60%, 90%), within the established biased ADC range. If the generator cannot synthesize a matched complex-IQ AM pair, use an explicitly documented real-IF AM test (e.g. 8 kHz carrier, input I, tune 8 kHz) as a separate configuration. Confirm the envelope at the ADC pin and measure recovered audio frequency, amplitude/distortion and clipping while recording timing. This test has not been performed.

Raw evidence: am-iq-optimized-v1-2026-10-14-run01.txt. Machine-readable summary: am-iq-optimized-v1-2026-10-14-run01.json. All previous logs/build snapshots preserved. Work remains local; no commit or push.
