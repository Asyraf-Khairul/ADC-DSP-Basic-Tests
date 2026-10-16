# FM physical demodulation preflight, 15 October 2026

Read-only help/status; no rebuild, flash, DSP-setting changes or cumulative-counter resets.

Local branch testing/fm-timing-optimized, commit 63f6610e8ab9e894b30543816e5c8033ace43c4a.

Runtime identity match: True. Settings match: False. Runtime identity is self-reported, not an independent flash readback.

Physical setup confirmation and fresh scope readings pending. The earlier 62.186506-second timing pass is historical evidence, not a current demodulation result.

```text
B=29088 rev=0 BASIC IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=0/13/24/1 Q=0/13/24/1 MAG=1 AUD rms/peak=6/12 clips=58700
us=2980 max=4184 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=2775/3909 OUT=205/297 IN=551/582 DSP=2186/3275 STAT=36/74 FFT=0/0
RX latest_us CICMIX=0 FILTER=0 DEMOD=0 CAPTURE=0 FFTFWD=0 FFTBINS=0 FFTINV=0
MEASURE id=0 active=0 settings_changed=0
```

These are cumulative starting values; no new physical-test interval has been measured.

Operator clarification: nothing has been connected yet. The initial near-ground ADC readings are a pre-connection observation, not a failed connected-input test. Generator settings, wiring, output probe and fresh voltages remain unverified.
