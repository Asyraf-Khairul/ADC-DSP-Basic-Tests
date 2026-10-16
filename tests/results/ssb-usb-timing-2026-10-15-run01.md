# USB IQ timing ? 15 October 2026

PASS observed USB IQ timing and serial input-condition interval

Physical scope voltage/phase verification was waived by operator. This timing result is not a quadrature or sideband-rejection pass. Generator phase label: Q displayed90deg; physical phase not verified.

Runtime identity matched artifact: {"BENCH_BUILD_LABEL": "fm-optimized-v2", "BENCH_SOURCE_SHA256": "c9424dd398f20e91897c62007188ab5e2d88b0e70e1e3567bc74c89aa570dae6", "BENCH_BASE_COMMIT": "bfa22b8c150ee87c1808418a08cb066160e3834d"}. UF2 SHA256 77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e. Self-reported identity; no flash readback. Local commit 63f6610e8ab9e894b30543816e5c8033ace43c4a.

Settings: {"agc": 4, "bass": 0, "blanker": 0, "bw": 2, "cw": 700, "deemph": 0, "gain_log2": 4, "hang": 3, "input": "IQ", "iqcorr": 0, "mode": "USB", "notch": 0, "nr": 0, "smooth": 10, "sq": -1, "swap": 0, "threshold": 1, "treble": 0, "tune": 0, "volume": 25}.

None performed. At least5.1 seconds settling before capture; no settings changes or cumulative-counter resets during interval. No firmware rebuild or flash.

Device duration 62.046212s; host duration 62.0441702s; throughput 234.373696 blocks/s (nominal234.375). Settings revisions 3 to 3.

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| B | 818870 | 833412 | 14542 |
| deadline | 0 | 0 | 0 |
| clips | 1059439 | 1059439 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |

Interval all-block maximum 3867us; headroom 399us against4266us. Cumulative boot maxima at start/end 4229/4229us. Largest serial-sampled latest total 3216us. Serial snapshots do not capture every block; window endpoints and accumulated maxima do.

| Class | Blocks | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14314 | 0 | 3188.570 | 3656 |
| SPECTRUM | 228 | 0 | 3501.860 | 3867 |

| Stage | Ordinary mean / max us | Spectrum mean / max us |
| --- | ---: | ---: |
| TOTAL | 3188.570 / 3656 | 3501.860 / 3867 |
| IN | 553.096 / 582 | 553.684 / 561 |
| DSP | 2388.040 / 2712 | 2388.145 / 2512 |
| STAT | 34.692 / 90 | 34.636 / 59 |
| FFT | 0.000 / 0 | 326.219 / 434 |
| OUT | 212.044 / 353 | 197.355 / 306 |
| CICMIX | 498.792 / 504 | 498.750 / 503 |
| FILTER | 1455.489 / 1620 | 1455.206 / 1482 |
| DEMOD | 345.115 / 469 | 345.224 / 413 |
| CAPTURE | 2.471 / 3 | 2.526 / 3 |
| FFTFWD | 861.745 / 933 | 861.570 / 870 |
| FFTBINS | 168.716 / 277 | 168.618 / 187 |
| FFTINV | 356.491 / 397 | 356.461 / 360 |

DSP/FILTER timings include nested stages; do not sum nested values or independent maxima. Elapsed processing plus audio-preparation window includes interrupt service. Diagnostic bookkeeping separately measured: {"max_us": 65, "sum_us": 62684}.

Digital audio snapshot range: {"rms_min": 2756, "rms_max": 2874, "peak_min": 3968, "peak_max": 4080, "limitation": "Broadband digital samples before volume/PWM, not selective electrical-output tone level"}.

Final serial snapshot:

```text
B=833412 rev=3 USB IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1932/2075/2204/87 Q=1936/2067/2213/88 MAG=248 AUD rms/peak=2872/4064 clips=1059439
us=3170 max=4229 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=2965/3919 OUT=205/536 IN=553/582 DSP=2379/3288 STAT=33/106 FFT=0/434
RX latest_us CICMIX=498 FILTER=1450 DEMOD=343 CAPTURE=3 FFTFWD=861 FFTBINS=165 FFTINV=356
MEASURE id=1 active=0 settings_changed=0
```

No recovered-tone frequency, electrical wanted/unwanted level, physical phase reversal or audibility measured by this collector. No photograph filenames supplied. UI responsiveness unverified for this run. Raw/JSON evidence beside this report. No commit or push.

Raw log SHA256: 4def9e4b4813904f8ea734bf1307072e64347895cfd78686f61d83279ec285bb
