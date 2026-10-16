# USB/LSB preflight ? 15 October 2026

Read-only help/status; no firmware or settings writes. Physical quadrature input verification pending. No hardware pass yet.

Branch testing/fm-timing-optimized, commit 63f6610e8ab9e894b30543816e5c8033ace43c4a.

```text
AssertionError('Expected exactly one Pico serial port')
```

## Source inspection

ADC_mapping: stream.cpp ADC1 GP27 I first, then ADC0 GP26 Q; alternating at 480 kword/s aggregate.

presets: Preset6 USB mode3, preset7 LSB mode2; both default IQ and tune0. Presets default gain_log2=4, so explicitly choose gain0 for initial x1 test.

planned_settings: IQ, tune0, BW2, manual AGC4, gain_log2=0, volume25, squelch off, swap0, iqcorr0 and all optional effects off. Final settings must be verified over serial.

sideband_convention: set_mode retains positive FFT bins for USB and negative bins for LSB; forward FFT uses negative-exponent twiddles. For physical I=A*cos(wt), Q=A*sin(wt), Q lags I by quarter-cycle: positive complex frequency, USB stronger. Physical Q leading I by quarter-cycle gives negative complex frequency, LSB stronger. Displayed phase numbers alone do not establish physical orientation; scope lead/lag is required.

demodulation: Both USB/LSB return filtered real component i. Selection occurs in complex FFT filter, not by changing final demodulation expression.

tuning: Must use0 Hz for this 1 kHz IQ test, not prior FM+8000.

PWM: stream_audio clamps250 + trunc(sample*249*volume/(32768*100)); at25% volume first nonzero target-duty change requires abs(sample)>=527. AUD RMS is pre-volume/pre-PWM broadband data; weak-side tone can be quantized away and must not be inferred solely from AUD.

window: Worker timer includes process_samples and stream_audio, original4266us deadline; every-block ordinary/spectrum accumulators and64-bit device endpoints preserve cumulative health/history; bookkeeping separately measured.

inspection_files: ['TEST_GUIDE.md', 'settings.cpp', 'bench.cpp', 'stream.cpp', 'receiver_dsp/rx_dsp.cpp', 'receiver_dsp/fft.cpp', 'receiver_dsp/fft_filter.cpp', 'timing_capture.h', 'main.cpp']

Runtime not yet verified: only COM3 Intel AMT is visible, no Pico serial port. First step is USB reconnection with generator outputs off; physical input verification must precede timing and functional collection.
