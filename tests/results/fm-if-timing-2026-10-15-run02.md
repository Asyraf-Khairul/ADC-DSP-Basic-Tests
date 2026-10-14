# FM real-I repeat after GP26 bias correction ? 15 October 2026

**FAIL against the 4266 us timing deadline.** The input-condition correction was effective in this interval: no additional near-rail samples occurred. Deadline misses persisted exclusively on spectrum-update blocks. No blocks were dropped and no audio underruns were recorded. FM demodulation performance remains untested.

## Procedure and identity

Running label am-optimized-v1, source fingerprint 86d0e4fe2bd42cc00d51896696ec6402e11beee3f1282a7385d0eff4441f9497, embedded base 34bde79a126fc4a7737a4e7194b63833e2aace05. The serial identity matches the published tested artifact SHA256 0bca44e437d8579749c61407390c5f12f947c87a45c53818ebc00163f6768f92. Publication commit bfa22b8c150ee87c1808418a08cb066160e3834d, branch testing/am-iq-optimized. No independent firmware binary readback.

No rebuild, flashing, reboot, DSP/rate change or cumulative-counter reset. The only physical correction requested since run01 was to provide midscale bias to GP26 using generator CH2 DC through the retained divider. User reported a fresh GP26 scope reading of **1.635 V**. Exact generator CH2 offset and wiring were not independently inspected.

Generator CH1 remains the user-confirmed 8 kHz sine carrier, displayed 300 mVpp, High-Z, +2.5 V offset, internal sine FM 500 Hz with 500 Hz deviation, through the existing divider to GP27. Mixer disconnected. Fresh GP27 mean/Vpp/min/max were not supplied, so complete scope verification of the FM waveform limits is not claimed.

All runtime settings matched Preset 4 and remained revision 5: FM, IF-I, tune +8000 Hz, BW 2, AGC 4/manual, gain exponent 0 (factor 1), volume 25%, squelch -1/off, hang 3, swap 0, IQ correction 0, de-emphasis 0, bass 0, treble 0, blanker 0, notch 0, noise reduction 0, smoothing 10, threshold 1, CW 700 Hz. No preset or setting write was issued. System clock remains 120 MHz; block deadline, rates and spectrum cadence unchanged.

After confirming settings and no active capture, the collector allowed at least 5.1 seconds settling and collected an explicit all-block window. Only serial reporting/help/status and measure start/stop commands were used. OLED and controls remained available; responsiveness was not user-confirmed for this run.

## Before/after comparison

| Metric | Run01: no GP26 source | Run02: GP26 1.635 V reported |
| --- | ---: | ---: |
| Host elapsed, s | 62.1139908 | 62.0550650 |
| Device window, s | 62.114179 | 62.058405 |
| Blocks | 14558 | 14545 |
| Blocks/s | 234.374828 | 234.375988 |
| Additional deadline misses | 54 | 80 |
| Additional near-rail samples | 2360 | 0 |
| Additional ADC drops / audio underruns | 0 / 0 | 0 / 0 |
| Additional FIFO / audio drops / late IRQ | 0 / 0 / 0 | 0 / 0 / 0 |
| Interval maximum total, us | 4465 | 4373 |

Both failures are retained. Different miss counts and maxima across two intervals are not evidence that the bias correction caused a timing improvement or regression; both demonstrate insufficient deadline headroom.

## Preserved counters and maxima

| Counter | Window start | Window end | Increase |
| --- | ---: | ---: | ---: |
| B | 1295640 | 1310185 | 14545 |
| deadline | 1573 | 1653 | 80 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |
| clips | 221663 | 221663 | 0 |

Device endpoints: 5529051660 and 5591110065 us since boot. Exact window counts and device timestamps determine throughput, nominally 234.375 blocks/s. Outer serial snapshots are subject to 32-block publication and USB/command latency; stop acknowledgements can represent later status than the frozen endpoint.

Cumulative maximum at interval start and end: 4958 / 4958 us, pre-existing. Interval maximum: 4373 us, exceeding 4266 us by 107 us (2.51%). Largest sampled latest total: 3511 us. Prior cumulative counts and maximum are not attributed to this interval.

| Block class | Count | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14318 | 0 | 3454.039 | 4027 |
| SPECTRUM | 227 | 80 | 4199.740 | 4373 |

## Stage timings

| Stage | Ordinary mean / max us | Spectrum-block mean / max us |
| --- | ---: | ---: |
| TOTAL | 3454.039 / 4027 | 4199.740 / 4373 |
| IN | 554.225 / 581 | 555.057 / 584 |
| DSP | 2668.841 / 3106 | 2669.401 / 2781 |
| STAT | 34.666 / 89 | 34.767 / 49 |
| FFT | 0.000 / 0 | 729.996 / 851 |
| OUT | 195.557 / 374 | 209.295 / 245 |
| CICMIX | 498.794 / 504 | 498.789 / 503 |
| FILTER | 1632.067 / 1862 | 1632.229 / 1685 |
| DEMOD | 359.220 / 496 | 359.194 / 373 |
| CAPTURE | 2.464 / 3 | 2.480 / 3 |
| FFTFWD | 922.466 / 983 | 922.753 / 969 |
| FFTBINS | 254.541 / 395 | 254.348 / 274 |
| FFTINV | 386.528 / 439 | 386.577 / 402 |

DSP includes receiver work. FILTER contains FFTFWD/FFTBINS/FFTINV; FFT is separate spectrum-display preparation. Do not sum nested stages or independent maxima. Total retains processing and audio preparation together, including interrupts. All 80 misses were among 227 spectrum-update blocks; ordinary blocks all met the deadline. Receiver FFT filtering remains the largest receiver substage, while spectrum preparation adds about 730.0 us on update blocks. These measurements identify the remaining workload/headroom problem, without isolating a particular instruction or proving a per-overrun cause.

Existing instrumentation remains enabled. Window bookkeeping, separate from the retained signal-processing/audio window, averaged 4.160 us and peaked at 23 us. No DSP work was moved outside the inclusive timer.

Final sampled status (may follow the frozen window endpoint):

    B=1310185 rev=5 FM IF-I tune=8000 bw=2 agc=4 gain=2^0 vol=25
    I min/mean/max/rms=1931/2071/2210/88 Q=2049/2056/2067/2 MAG=123 AUD rms/peak=1568/2620 clips=221663
    us=3511 max=4958 deadline=1653 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
    TIMING us latest/max PROC=3317/4752 OUT=194/374 IN=554/584 DSP=2727/3264 STAT=35/104 FFT=0/1500
    RX latest_us CICMIX=498 FILTER=1698 DEMOD=356 CAPTURE=2 FFTFWD=924 FFTBINS=291 FFTINV=415

## Input condition and conclusion

Sampled Q means ranged 2056..2058 counts, with min/max 2044..2074 and AC RMS 2..2 counts. This is consistent with the restored midscale bias and no new rail detections; it is not an independently calibrated voltage measurement. I sampled min/max were 1930..2219 counts.

The corrected input condition did not resolve the timing failure. Before presenting final FM demodulation results as deadline-compliant, address the receiver/spectrum processing headroom and repeat the all-block capture under the same documented settings. No firmware optimization is included in this test. The observed absence of streaming losses does not relax the deadline criterion. No 500 Hz spectral, modulation-off, deviation-response, distortion or audibility measurement is claimed.

Raw log and JSON are beside this report. Previous AM and failed FM runs remain unchanged. New evidence and host collector changes remain local; no commit or push.
