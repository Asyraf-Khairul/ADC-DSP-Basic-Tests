# LSB IQ timing ? 15 October 2026

PASS observed LSB IQ timing and serial input-condition interval

Physical scope voltage/phase verification was waived by operator. This timing result is not a quadrature or sideband-rejection pass. Generator phase label: Q displayed90deg; physical phase not verified.

Runtime identity matched artifact: {"BENCH_BUILD_LABEL": "fm-optimized-v2", "BENCH_SOURCE_SHA256": "c9424dd398f20e91897c62007188ab5e2d88b0e70e1e3567bc74c89aa570dae6", "BENCH_BASE_COMMIT": "bfa22b8c150ee87c1808418a08cb066160e3834d"}. UF2 SHA256 77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e. Self-reported identity; no flash readback. Local commit 63f6610e8ab9e894b30543816e5c8033ace43c4a.

Settings: {"agc": 4, "bass": 0, "blanker": 0, "bw": 2, "cw": 700, "deemph": 0, "gain_log2": 4, "hang": 3, "input": "IQ", "iqcorr": 0, "mode": "LSB", "notch": 0, "nr": 0, "smooth": 10, "sq": -1, "swap": 0, "threshold": 1, "treble": 0, "tune": 0, "volume": 25}.

USB to LSB before settling; all other settings fixed. At least5.1 seconds settling before capture; no settings changes or cumulative-counter resets during interval. No firmware rebuild or flash.

Device duration 62.049315s; host duration 62.0475246s; throughput 234.378091 blocks/s (nominal234.375). Settings revisions 4 to 4.

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| B | 835805 | 850348 | 14543 |
| deadline | 0 | 0 | 0 |
| clips | 1059439 | 1059439 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |

Interval all-block maximum 4129us; headroom 137us against4266us. Cumulative boot maxima at start/end 4229/4229us. Largest serial-sampled latest total 3212us. Serial snapshots do not capture every block; window endpoints and accumulated maxima do.

| Class | Blocks | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14316 | 0 | 3190.303 | 3643 |
| SPECTRUM | 227 | 0 | 3507.141 | 4129 |

| Stage | Ordinary mean / max us | Spectrum mean / max us |
| --- | ---: | ---: |
| TOTAL | 3190.303 / 3643 | 3507.141 / 4129 |
| IN | 553.087 / 576 | 553.700 / 562 |
| DSP | 2389.411 / 2718 | 2389.960 / 2646 |
| STAT | 34.948 / 95 | 35.062 / 74 |
| FFT | 0.000 / 0 | 328.828 / 542 |
| OUT | 212.162 / 400 | 197.727 / 306 |
| CICMIX | 498.804 / 504 | 498.749 / 503 |
| FILTER | 1456.806 / 1606 | 1456.916 / 1574 |
| DEMOD | 345.147 / 479 | 345.427 / 418 |
| CAPTURE | 2.466 / 22 | 2.436 / 3 |
| FFTFWD | 861.732 / 911 | 861.652 / 896 |
| FFTBINS | 170.091 / 280 | 170.167 / 226 |
| FFTINV | 356.472 / 396 | 356.577 / 383 |

DSP/FILTER timings include nested stages; do not sum nested values or independent maxima. Elapsed processing plus audio-preparation window includes interrupt service. Diagnostic bookkeeping separately measured: {"max_us": 35, "sum_us": 62515}.

Digital audio snapshot range: {"rms_min": 318, "rms_max": 343, "peak_min": 480, "peak_max": 560, "limitation": "Broadband digital samples before volume/PWM, not selective electrical-output tone level"}.

Final serial snapshot:

```text
B=850348 rev=4 LSB IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1937/2078/2205/89 Q=1930/2071/2214/87 MAG=29 AUD rms/peak=330/512 clips=1059439
us=3172 max=4229 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=2967/3919 OUT=205/536 IN=553/582 DSP=2380/3288 STAT=33/106 FFT=0/542
RX latest_us CICMIX=498 FILTER=1453 DEMOD=342 CAPTURE=2 FFTFWD=861 FFTBINS=167 FFTINV=356
MEASURE id=2 active=0 settings_changed=0
```

No recovered-tone frequency, electrical wanted/unwanted level, physical phase reversal or audibility measured by this collector. No photograph filenames supplied. UI responsiveness unverified for this run. Raw/JSON evidence beside this report. No commit or push.

Raw log SHA256: 7643e95ce1f90b00dc4f4bd8052b1f974541b67df6c1c46b868aa81a13620be3
