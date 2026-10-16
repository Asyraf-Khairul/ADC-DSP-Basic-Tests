# USB measurable-output settings ? 15 October 2026

At gain x2, latest PCM peak500 is below first nonzero PWM target-duty threshold527 at volume25. Gain x16 selected before timing and functional comparisons; input/phase verification still pending. All cumulative counters preserved; no rebuild or flash.

```text
B=597600 rev=3 USB IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1936/2077/2213/88 Q=1925/2078/2214/89 MAG=248 AUD rms/peak=2766/3984 clips=1059439
us=3208 max=4229 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=2987/3919 OUT=221/536 IN=554/582 DSP=2397/3288 STAT=36/98 FFT=0/417
RX latest_us CICMIX=499 FILTER=1461 DEMOD=348 CAPTURE=2 FFTFWD=863 FFTBINS=172 FFTINV=357
MEASURE id=0 active=0 settings_changed=0
```
