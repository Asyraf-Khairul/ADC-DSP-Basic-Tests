# FM real-I optimization and timing, 15 October 2026

FAIL observed FM real-I timing interval. This is a bounded timing/streaming result, not an FM demodulation-quality result.

## Changes

- Native-width FFT indices remove repeated 16-bit truncations while retaining signed 16-bit sample intermediates and original rounding/wrapping.
- Quarter-turn index is span/2, avoiding original unused negative shift at stage zero.
- Zoom=1 spectrum copy avoids generic runtime divisions, exact pixels and original cadence.

No sampling, clock, filter coefficients, demodulation, gain, spectrum cadence or deadline changes. Existing instrumentation remains enabled. Core1 signal work has not been moved outside the inclusive timer. All original failed raw logs remain unchanged; firmware/DSP edits and new evidence remain local, uncommitted and unpushed.

## Conditions and identity

User confirmed requested FM setup is on: mixer disconnected; existing 470 ohm/1 kohm dividers retained; generator CH1 to GP27, 8 kHz sine carrier, displayed 300 mVpp High-Z +2.5 V offset, internal sine FM 500 Hz with 500 Hz deviation. GP26 midscale bias correction requested using generator CH2 DC through retained divider; user supplied fresh voltage. Exact CH2 offset readback and physical wiring were not independently inspected.

GP26 1.635 V is the user measurement preceding baseline run02, carried forward with setup left unchanged; not freshly measured after flashing.

Settings: {"agc": 4, "bass": 0, "blanker": 0, "bw": 2, "cw": 700, "deemph": 0, "gain_log2": 0, "hang": 3, "input": "IF-I", "iqcorr": 0, "mode": "FM", "notch": 0, "nr": 0, "smooth": 10, "sq": -1, "swap": 0, "threshold": 1, "treble": 0, "tune": 8000, "volume": 25}.

Runtime identity: {"BENCH_BASE_COMMIT": "bfa22b8c150ee87c1808418a08cb066160e3834d", "BENCH_BUILD_LABEL": "fm-optimized-v1", "BENCH_SOURCE_SHA256": "c61de66ea6531b1d8d0609d46bcfdbdec12eb90ef4ca1d65a30cb42f5cc7767a"}. UF2 SHA256 5a31caf35c9ca3759959e3316184a959fb73646c6d5d3e6bb590d27fe47d67ff. Runtime identity matched; no independent binary readback. Release -O3, -fwrapv; both self-test and operation benchmark OFF.

1200-baud USB reset, RPI-RP2 UF2 copy, runtime identity verified; power retained. picotool direct forced load unsupported on Windows; BOOTSEL access driver unavailable. No driver changes made.

Reboot starts new cumulative counters; baseline counters are preserved in their own records. After selecting preset 4, the collector allowed at least 5.1 seconds settling. No settings or cumulative counter resets occurred during the interval. Window accumulators only were started/stopped. UI responsiveness remains unverified until the user confirms.

## Before and after

| Metric | Corrected-bias baseline | Optimized |
| --- | ---: | ---: |
| Host elapsed (s) | 62.055065 | 62.0654846 |
| Device elapsed (s) | 62.058405 | 62.067215 |
| Blocks/s | 234.37598823237562 | 234.37494335777754 |
| Maximum total across every interval block (us) | 4373 | 4872 |
| Processed blocks increase | 14545 | 14547 |
| Deadline misses increase | 80 | 1 |
| ADC drops increase | 0 | 0 |
| FIFO overflows increase | 0 | 0 |
| Audio underruns increase | 0 | 0 |
| Audio drops increase | 0 | 0 |
| Late DMA interrupts increase | 0 | 0 |
| Near-rail samples increase | 0 | 0 |

Observed headroom: -606 us (-14.21%). A short successful run does not guarantee every input, enabled effect or longer run will meet timing.

## Preserved starting counters

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| B | 6774 | 21321 | 14547 |
| deadline | 0 | 1 | 1 |
| clips | 0 | 0 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |

Device endpoints: 29890524 to 91957739 us. Status revision remains 1. Exact window endpoints determine throughput; USB receipt and 32-block status publication affect outer serial snapshots.

Cumulative maximum at selected start: 4140 us; cumulative maximum at end: 4872 us. These include earlier operation after this boot. Largest sampled latest total: 3395 us. Serial samples do not capture every block; all-block window maxima do.

| Block class | Blocks | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14319 | 0 | 3363.595 | 3895 |
| SPECTRUM | 228 | 1 | 3986.596 | 4872 |

## Stage timings

| Stage | Ordinary mean / max us | Spectrum mean / max us |
| --- | ---: | ---: |
| TOTAL | 3363.595 / 3895 | 3986.596 / 4872 |
| IN | 554.269 / 587 | 554.987 / 569 |
| DSP | 2578.270 / 2977 | 2579.504 / 2959 |
| STAT | 34.643 / 95 | 34.811 / 68 |
| FFT | 0.000 / 0 | 619.583 / 932 |
| OUT | 195.646 / 367 | 197.083 / 352 |
| CICMIX | 498.784 / 504 | 498.776 / 503 |
| FILTER | 1541.234 / 1797 | 1541.539 / 1692 |
| DEMOD | 359.258 / 443 | 359.404 / 400 |
| CAPTURE | 2.466 / 3 | 2.452 / 3 |
| FFTFWD | 861.639 / 911 | 861.680 / 887 |
| FFTBINS | 254.521 / 478 | 254.711 / 340 |
| FFTINV | 356.541 / 414 | 356.680 / 396 |

DSP contains receiver stages; FILTER contains FFTFWD/FFTBINS/FFTINV. FFT is separate display preparation. Do not sum nested times or independent maxima. Elapsed times include interrupts. This combined change is not an isolated per-edit benchmark.

Window bookkeeping remains separately measured: {"max_us": 66, "sum_us": 59767}. No DSP work is placed there.

Final serial snapshot (may follow the frozen endpoint):

```text
B=21321 rev=1 FM IF-I tune=8000 bw=2 agc=4 gain=2^0 vol=25
I min/mean/max/rms=1933/2071/2213/89 Q=2046/2056/2068/2 MAG=122 AUD rms/peak=1657/2894 clips=0
us=3363 max=4872 deadline=1 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=3174/4520 OUT=189/367 IN=553/587 DSP=2576/3055 STAT=45/95 FFT=0/932
RX latest_us CICMIX=500 FILTER=1546 DEMOD=355 CAPTURE=3 FFTFWD=860 FFTBINS=262 FFTINV=356
```

## Validation and limits

43 fresh-process receiver scenarios, 512 blocks each: PCM, magnitude, spectrum pixels and scale match the frozen reference exactly. Includes FM real-I gain x1 at deviations 250/500 Hz and modulation off, all original receiver regression cases, and spectrum zoom 2/3/4. 9,000 FFT/IFFT cases, sizes 1..256 with silence, full-scale rails and seeded random inputs, also match exactly. These are host checks, not target numerical or demodulation-quality proof.

Static end 0x20031134, heap capacity 61,132 bytes; known queue payload 51,929 bytes leaves 9,203 before allocator/library overhead. This is a static budget, not a free-heap measurement. No new lookup tables or heap allocations.

The 500 Hz recovered tone, modulation-off suppression, deviation response, distortion and speaker audibility are not measured here. Input settings are user reported; fresh complete GP27 scope limits are unavailable. Photos were not requested or archived. Prior AM hardware timing has not been revalidated on this new binary.
