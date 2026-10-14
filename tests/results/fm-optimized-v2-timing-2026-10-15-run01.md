# FM real-I optimization and timing, 15 October 2026

PASS observed FM real-I timing interval. This is a bounded timing/streaming result, not an FM demodulation-quality result.

First candidate fm-optimized-v1 reduced interval misses to one but failed with a 4872 us maximum. Its raw log and report are retained. The second candidate below adds exact within-frame spectrum memoization.

## Changes

- Native-width FFT indices remove repeated 16-bit truncations while retaining signed 16-bit sample intermediates and original rounding/wrapping.
- Quarter-turn index is span/2, avoiding original unused negative shift at stage zero.
- Zoom=1 spectrum copy avoids generic runtime divisions, exact pixels and original cadence.
- Reuse the 256 corrected magnitudes within each spectrum update.
- Memoize exact pixel results for repeated magnitudes below 256 within each update; higher values retain original arithmetic. Cache validity resets every call, so changing scale cannot reuse stale pixels.

No sampling, clock, filter coefficients, demodulation, gain, spectrum cadence or deadline changes. Existing instrumentation remains enabled. Core1 signal work has not been moved outside the inclusive timer. All original failed raw logs remain unchanged; firmware/DSP edits and FM evidence are recorded on the local testing/fm-timing-optimized branch; no push is included.

## Conditions and identity

User confirmed requested FM setup is on: mixer disconnected; existing 470 ohm/1 kohm dividers retained; generator CH1 to GP27, 8 kHz sine carrier, displayed 300 mVpp High-Z +2.5 V offset, internal sine FM 500 Hz with 500 Hz deviation. GP26 midscale bias correction requested using generator CH2 DC through retained divider; user supplied fresh voltage. Exact CH2 offset readback and physical wiring were not independently inspected.

GP26 1.635 V is the user measurement preceding baseline run02, carried forward with setup left unchanged; not freshly measured after flashing.

Settings: {"agc": 4, "bass": 0, "blanker": 0, "bw": 2, "cw": 700, "deemph": 0, "gain_log2": 0, "hang": 3, "input": "IF-I", "iqcorr": 0, "mode": "FM", "notch": 0, "nr": 0, "smooth": 10, "sq": -1, "swap": 0, "threshold": 1, "treble": 0, "tune": 8000, "volume": 25}.

Runtime identity: {"BENCH_BASE_COMMIT": "bfa22b8c150ee87c1808418a08cb066160e3834d", "BENCH_BUILD_LABEL": "fm-optimized-v2", "BENCH_SOURCE_SHA256": "c9424dd398f20e91897c62007188ab5e2d88b0e70e1e3567bc74c89aa570dae6"}. UF2 SHA256 77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e. Runtime identity matched; no independent binary readback. Release -O3, -fwrapv; both self-test and operation benchmark OFF.

1200-baud USB reset, RPI-RP2 UF2 copy, runtime identity verified; power retained. picotool direct forced load unsupported on Windows; BOOTSEL access driver unavailable. No driver changes made.

Reboot starts new cumulative counters; baseline counters are preserved in their own records. After selecting preset 4, the collector allowed at least 5.1 seconds settling. No settings or cumulative counter resets occurred during the interval. Window accumulators only were started/stopped. User replied Responsive after v2 startup and before this timed window; this is not a separately timestamped in-window check.

## Before and after

| Metric | Corrected-bias baseline | Optimized |
| --- | ---: | ---: |
| Host elapsed (s) | 62.055065 | 62.1831426 |
| Device elapsed (s) | 62.058405 | 62.186506 |
| Blocks/s | 234.37598823237562 | 234.37560553731706 |
| Maximum total across every interval block (us) | 4373 | 3969 |
| Processed blocks increase | 14545 | 14575 |
| Deadline misses increase | 80 | 0 |
| ADC drops increase | 0 | 0 |
| FIFO overflows increase | 0 | 0 |
| Audio underruns increase | 0 | 0 |
| Audio drops increase | 0 | 0 |
| Late DMA interrupts increase | 0 | 0 |
| Near-rail samples increase | 0 | 0 |

Observed headroom: 297 us (6.96%). A short successful run does not guarantee every input, enabled effect or longer run will meet timing.

## Preserved starting counters

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| B | 4319 | 18894 | 14575 |
| deadline | 0 | 0 | 0 |
| clips | 0 | 0 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |

Device endpoints: 19416251 to 81602757 us. Status revision remains 1. Exact window endpoints determine throughput; USB receipt and 32-block status publication affect outer serial snapshots.

Cumulative maximum at selected start: 4003 us; cumulative maximum at end: 4003 us. These include earlier operation after this boot. Largest sampled latest total: 3470 us. Serial samples do not capture every block; all-block window maxima do.

| Block class | Blocks | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14347 | 0 | 3362.906 | 3969 |
| SPECTRUM | 228 | 0 | 3767.605 | 3858 |

## Stage timings

| Stage | Ordinary mean / max us | Spectrum mean / max us |
| --- | ---: | ---: |
| TOTAL | 3362.906 / 3969 | 3767.605 / 3858 |
| IN | 553.758 / 575 | 554.399 / 562 |
| DSP | 2578.202 / 3029 | 2577.535 / 2635 |
| STAT | 34.594 / 100 | 34.522 / 42 |
| FFT | 0.000 / 0 | 402.781 / 458 |
| OUT | 195.623 / 341 | 196.430 / 212 |
| CICMIX | 498.779 / 504 | 498.759 / 503 |
| FILTER | 1541.326 / 1757 | 1540.811 / 1571 |
| DEMOD | 359.135 / 467 | 359.075 / 379 |
| CAPTURE | 2.470 / 3 | 2.491 / 3 |
| FFTFWD | 861.743 / 911 | 861.504 / 869 |
| FFTBINS | 254.534 / 387 | 254.272 / 275 |
| FFTINV | 356.522 / 417 | 356.478 / 361 |

DSP contains receiver stages; FILTER contains FFTFWD/FFTBINS/FFTINV. FFT is separate display preparation. Do not sum nested times or independent maxima. Elapsed times include interrupts. This combined change is not an isolated per-edit benchmark.

Window bookkeeping remains separately measured: {"max_us": 66, "sum_us": 59135}. No DSP work is placed there.

Final serial snapshot (may follow the frozen endpoint):

```text
B=18894 rev=1 FM IF-I tune=8000 bw=2 agc=4 gain=2^0 vol=25
I min/mean/max/rms=1933/2073/2209/88 Q=2024/2057/2069/2 MAG=123 AUD rms/peak=1568/3156 clips=0
us=3470 max=4003 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=3277/3799 OUT=193/367 IN=557/586 DSP=2681/3029 STAT=38/100 FFT=0/611
RX latest_us CICMIX=498 FILTER=1645 DEMOD=356 CAPTURE=2 FFTFWD=886 FFTBINS=297 FFTINV=393
```

## Validation and limits

43 fresh-process receiver scenarios, 512 blocks each: PCM, magnitude, spectrum pixels and scale match the frozen reference exactly. Includes FM real-I gain x1 at deviations 250/500 Hz and modulation off, all original receiver regression cases, and spectrum zoom 2/3/4. 9,000 FFT/IFFT cases, sizes 1..256 with silence, full-scale rails and seeded random inputs, also match exactly. These are host checks, not target numerical or demodulation-quality proof.

Static end 0x20031134, heap capacity 61,132 bytes; known queue payload 51,929 bytes leaves 9,203 before allocator/library overhead. This is a static budget, not a free-heap measurement. No new persistent tables or heap allocations. Local magnitude/pixel/validity arrays add 800 bytes of spectrum stack storage within the existing 16 KiB core1 stack; not a runtime stack high-water measurement.

The 500 Hz recovered tone, modulation-off suppression, deviation response, distortion and speaker audibility are not measured here. Input settings are user reported; fresh complete GP27 scope limits are unavailable. Photos were not requested or archived. Prior AM hardware timing has not been revalidated on this new binary.

Mean FFT filter time decreased from 1632.1 to 1541.3 us; mean spectrum preparation decreased from 730.0 to 402.8 us. These compare whole builds under observed conditions, not isolated per-edit benchmarks.
