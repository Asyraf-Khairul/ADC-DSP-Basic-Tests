# AM IQ profiling baseline, 14 October 2026

This profiling-only firmware preserves the receiver calculations and fails real-time timing. It is the measured baseline for targeted optimization; run02 remains the original unmodified baseline. CH1-off run01 is excluded.

Firmware label am-profile-v1; serial source fingerprint verified: c456818c3dec8a673e5a7d7f681e46a9440ae089647939a63eaad2264ce45252.
UF2 SHA256: 7eca95ae6b806447dccce2b577756574a7ecd1440433af008521b93b738c3b1d.
Restart: Approved picotool USB BOOTSEL reset then UF2 mass-storage copy; USB power retained; fresh firmware boot; no physical power cycle.

After AM preset confirmation and at least five seconds settling, the explicit all-block capture lasted 62.046524 device seconds and 62.0422013 host seconds between acknowledgement records. Device boundaries are authoritative for the exact block/counter window; host receive timestamps include command/USB latency. Settings stayed at preset 1, revision 1.

Blocks 12922, rate 208.263077/s; deadline increases 12922; ADC drops and audio underruns +1620 each; other streaming faults and near-rail increases zero.
Ordinary blocks: 12720, all late, maximum 5396 us. Spectrum blocks: 202, all late, maximum 7289 us. Deadline remains 4266 us. Largest sampled latest total was only 4900 us, demonstrating why the all-block capture is needed. The cumulative boot maximum ended at 7289 us; the explicit interval also observed that value.

| Block class | Stage | Mean us | Maximum us |
| --- | --- | ---: | ---: |
| ORDINARY | TOTAL | 4736.159 | 5396 |
| ORDINARY | IN | 701.506 | 1199 |
| ORDINARY | DSP | 3801.389 | 4143 |
| ORDINARY | CICMIX | 1744.914 | 1791 |
| ORDINARY | FILTER | 1617.707 | 1821 |
| ORDINARY | DEMOD | 349.549 | 466 |
| ORDINARY | FFT | 0.000 | 0 |
| ORDINARY | FFTFWD | 932.596 | 1010 |
| ORDINARY | FFTBINS | 225.047 | 402 |
| ORDINARY | FFTINV | 390.804 | 435 |
| ORDINARY | OUT | 196.476 | 441 |
| SPECTRUM | TOTAL | 6523.668 | 7289 |
| SPECTRUM | IN | 700.124 | 806 |
| SPECTRUM | DSP | 3803.292 | 3846 |
| SPECTRUM | CICMIX | 1748.832 | 1776 |
| SPECTRUM | FILTER | 1617.490 | 1654 |
| SPECTRUM | DEMOD | 348.144 | 380 |
| SPECTRUM | FFT | 1785.569 | 2498 |
| SPECTRUM | FFTFWD | 934.292 | 961 |
| SPECTRUM | FFTBINS | 224.827 | 260 |
| SPECTRUM | FFTINV | 388.698 | 417 |
| SPECTRUM | OUT | 197.233 | 229 |

Nested filter timings are subdivisions, not additional costs. Independent maxima must not be summed. CIC/DC/mixing and FFT filtering dominate ordinary receiver work. Spectrum preparation adds an average 1785.569 us on update blocks, peaking at 2498 us. No spectrum work or cadence was removed.

The startup probe reported 120000000 Hz and 87 us for 1024 timer reads plus loop/store overhead, about 0.085 us per iteration. Nine extra receiver timer reads add about 0.765 us of that estimated primitive cost per block; this is not a full instrumentation A/B measurement. Timer calls and profiling stores within process_samples remain included in total processing time. Separate window bookkeeping averaged 4.264 us and peaked at 66 us including any interrupts. It occurs after the unchanged inclusive processing/audio window; it does no signal-processing work, and resulting streaming losses remain visible.

Both inputs were confirmed on; no fresh scope values supplied. User confirmed controls responsive and P1/P4 captured, but photographs were not supplied. Timing with unmodulated sines establishes no demodulation-quality pass.

Raw log and full JSON are saved beside this report. Starting/ending cumulative counters, startup, transition and every status are retained there. No commit or push.
