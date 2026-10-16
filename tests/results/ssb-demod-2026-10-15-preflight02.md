# USB/LSB connected preflight ? 15 October 2026

Read-only help/status; no firmware or settings writes. Physical quadrature input verification pending. No hardware pass yet.

Branch testing/fm-timing-optimized, commit 63f6610e8ab9e894b30543816e5c8033ace43c4a.

```text
B=9152 rev=0 BASIC IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1/15/31/4 Q=6/16/32/3 MAG=5 AUD rms/peak=10/23 clips=77973
us=2982 max=4229 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=2777/3919 OUT=205/310 IN=551/582 DSP=2188/3288 STAT=36/74 FFT=0/0
RX latest_us CICMIX=0 FILTER=0 DEMOD=0 CAPTURE=0 FFTFWD=0 FFTBINS=0 FFTINV=0
MEASURE id=0 active=0 settings_changed=0
```

User replied set after configuring both generator channels as continuous 1 kHz sine, displayed 300 mVpp, +2.5 V offset, High-Z, modulation off; CH1 phase0 and CH2 phase90 instructed. Exact readbacks not independently inspected. Outputs remain off per instructions pending next step. Fresh ADC-pin voltages and physical phase relationship are not yet verified.
