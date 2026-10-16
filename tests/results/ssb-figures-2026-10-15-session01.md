# Subsequent USB/LSB figure session ? 15 October2026

Separate figure verification session; not a recreation of historical timing conditions. Initial read-only help/status; no rebuild, flash, settings write or counter reset. Input/probe/phase verification and new capture filenames pending.

Branch testing/fm-timing-optimized, commit 63f6610e8ab9e894b30543816e5c8033ace43c4a.

```text
B=1656608 rev=8 USB IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1934/2076/2210/88 Q=1936/2079/2211/89 MAG=247 AUD rms/peak=2776/4016 clips=1059439
us=3170 max=4229 deadline=0 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
sq=-1 hang=3 swap=0 iqcorr=0 deemph=0 bass=0 treble=0 blanker=0 notch=0 nr=0 smooth=10 threshold=1 cw=700
TIMING us latest/max PROC=2965/3919 OUT=205/536 IN=553/582 DSP=2378/3288 STAT=34/120 FFT=0/929
RX latest_us CICMIX=498 FILTER=1451 DEMOD=342 CAPTURE=2 FFTFWD=861 FFTBINS=165 FFTINV=356
MEASURE id=2 active=0 settings_changed=0
```

Both input sine waves visible per user. At instructed displayedQ90, scopeCH1/GP27 user reports approximately mean1.65V,min1.52V,max1.78V,Vpp240mV. Extrema imply260mV span; discrepancy preserved without assuming readout timing/statistic selection. Reported range lies within0-3.3V. Probe/coupling/scales were instructed, not independently inspected. FreshGP26 values, cursor phase and capture filenames remain pending.

GP26/CH2: operator replied same to requested mean/min/max/Vpp, recorded as approximately1.65/1.52/1.78V and240mVpp by comparison withGP27, rather than independently typed precise values. Inputs appear approximately matched by operator comparison; exact equality is not established. The260mV extrema-span versus240mVpp discrepancy remains. Next physical time-cursor phase/lead-lag measurement; no accepted input capture yet.

Scope issue: user reports unclean traces and too-small/clipping indications at200mV/div. Exact channel/readout and live state not supplied. This is not evidence of ADC clipping;1.65V-biased waveforms may exceed an unsuitable scope range/offset. No phase capture accepted. Proposed both-channelDC,Position0,Offset+1.65V,Scale100mV/div; horizontal200us/div,CH1rising trigger1.65V. Actual settings and success remain unconfirmed.

Phase issue: operator reports approximately45degrees with an unclean measurement; no exact source/reference supplied. Do not accept as quadrature. At1kHz45degrees would correspond to125us, not250us. Corrected generator instructions now explicitly use AFG1022InterCH/FreqLockON/PhaseDev90deg (manualpage44;PhaseDev valid only withFreqLockON). Earlier instructions omitted this interchannel locking step. This is a proposed correction, not a verified setting or established cause. No historical phase observation is silently replaced.
