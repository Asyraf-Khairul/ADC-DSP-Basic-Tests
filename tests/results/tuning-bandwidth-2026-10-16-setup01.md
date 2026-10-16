# Digital tuning and bandwidth setup - 16 October 2026

Local commit 63f6610e8ab9e894b30543816e5c8033ace43c4a on testing/fm-timing-optimized. No tracked code changes. Prior untracked evidence preserved. New host-only serial collector added; no firmware rebuild, flash or cumulative-counter reset.

Runtime self-reported identity matches fm-optimized-v2 artifact metadata, SHA256 77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e. No flash readback. System clock120 MHz reported. Final settings verified:

```text
B=124864 rev=24 USB IF-I tune=6500 bw=4 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1941/2075/2211/88 Q=2040/2055/2067/4 MAG=123 AUD rms/peak=1402/1968 clips=398614
us=3263 max=4233 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=3075/3925 OUT=188/340 IN=553/584 DSP=2488/3286 STAT=33/105 FFT=0/444
RX latest_us CICMIX=498 FILTER=1458 DEMOD=342 CAPTURE=3 FFTFWD=860 FFTBINS=174 FFTINV=356
MEASURE id=0 active=0 settings_changed=0
```

Setup counters: {"start": {"deadline": 0, "ADCdrop": 0, "FIFO": 0, "AUDunder": 0, "AUDdrop": 0, "IRQlate": 0, "clips": 398614, "B": 121280}, "end": {"deadline": 0, "ADCdrop": 0, "FIFO": 0, "AUDunder": 0, "AUDdrop": 0, "IRQlate": 0, "clips": 398614, "B": 124864}, "increase": {"deadline": 0, "ADCdrop": 0, "FIFO": 0, "AUDunder": 0, "AUDdrop": 0, "IRQlate": 0, "clips": 0, "B": 3584}, "limitation": "Setup interval with deliberate configuration changes, not final all-block timing test. Existing counts preserved."}

Current scope input measurements, output FFT readings, photographs, and final all-block timing test are pending. ADC telemetry alone is not fresh independent scope verification. Existing near-rail counts predate this setup capture; cause is unknown. Cumulative timing max4233 us is not a test-window maximum.

Code-derived plan is in tuning-bandwidth-2026-10-16-plan01.json. Raw timestamped help/status and commands are in the adjacent .txt; SHA256 7717d6333d9f26606740043506bc6508f3af7df18bc836e8f9ad486961709b0f. No publication.

Fresh operator scope readings: GP27 mean1.65V, min1.52V, max1.78V (extrema span260mV, not a separately reported Vpp measurement); GP26 DC1.65V. Exact generator readbacks not supplied. No photograph inspected. Before first FFT measurement, tune6500-01 status confirms USB IF-I,6500Hz,bw4,manualx16,volume25; all streaming fault counters remain0 and cumulative near-rail count398614 unchanged. Electrical frequency verification and final timing remain pending.

First tuning point, operator reported: +6500Hz tuning ->1.5kHz output, -51dBm. SR2.5MS/s and RL250kpoints confirmed against prompted values, giving0.1s and10Hz raw FFT-bin spacing. Actual FFT window/cursor precision remain unconfirmed; no image inspected. Nominal error0Hz at reported precision, not a claim of exact calibrated frequency.

Operator correction: first point tune6500 output1.5kHz level was -57dBm, superseding previously reported -51dBm. Second point tune7000 output1.0kHz level -51dBm, frequency and Hann/Hanning window confirmed by user. Frequencies remain rounded operator readbacks, no image inspection. One deadline miss occurred between post-change and settled tune7000 snapshots (max4408us); other stream counters remained0 and near-rail count398614 unchanged. Event cause/stage not established; preserve separately from future timing window.

Third tuning point: operator reports2.5kHz at -51dBm for tune5500,bw4. Serial tune5500-01 verifies settings; cumulative deadline1,max4408us unchanged and streaming faults0,near-rail398614 unchanged. Original tune6500 repeat is next; bandwidth photos not yet collected.

Return check tune6500: operator confirms1.5kHz and -51dBm. Operator now suspects earlier -57dBm was a readback error; retained as uncertain history, excluded from quantitative amplitude-repeatability assessment. Output frequency returned at reported precision. Final timing still pending.

Bandwidth4 comparison: operator reports2.498 (interpretedkHz) and -52.25dBm, nominal frequency error -2Hz. This fresh reading supersedes no earlier raw readings; it is a separate bandwidth comparison observation. Image bandwidth4_2500Hz requested but saving not confirmed and no image inspected. Keep settings unchanged pending capture.

Wide-band photo reported saved after requested name bandwidth4_2500Hz; actual filename/path not supplied, image not inspected. Bandwidth0 serial status confirms only bw changed, tune5500 retained; deadline1,max4408us,all stream losses0,near-rail398614 unchanged. Operator reports no visible peaks and -81dBm at instructed fixed2.498kHz cursor: unresolved/background. Wide peak -52.25dBm to narrow background -81dBm gives28.75dB separation, not exact rejection. Narrow screenshot save and return-to-wide measurement pending.
