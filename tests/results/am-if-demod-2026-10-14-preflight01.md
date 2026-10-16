# Section 4.5.5 AM real-I demodulation: preflight, 14 October 2026

Status: awaiting physical setup confirmation and fresh instrument readings. No timing or demodulation verdict for the intended setup yet.

Local branch testing/am-iq-optimized, publication commit bfa22b8c150ee87c1808418a08cb066160e3834d. Read-only help/status on COM8 verified runtime label am-optimized-v1, source fingerprint 86d0e4fe2bd42cc00d51896696ec6402e11beee3f1282a7385d0eff4441f9497, embedded source base 34bde79a126fc4a7737a4e7194b63833e2aace05. Clock reported 120000000 Hz. This verifies self-reported identity against the tested artifact, not an independent running-binary readback.

Current settings already match Preset 2: AM, IF-I, tune +8000 Hz, BW 2, AGC 4/manual, gain exponent 4 (factor 16), volume 25%, squelch -1/off, hang 3, swap 0, IQ correction 0, de-emphasis 0, bass 0, treble 0, blanker 0, notch 0, noise reduction 0, smoothing 10, threshold 1, CW 700 Hz. Revision 1; no explicit capture active (id 0). The status does not expose a preset number; correspondence is verified from all settings against settings.cpp.

Preflight snapshot B=75776: cumulative deadline misses 2, near-rail samples 189643, maximum total since boot 4417 us. ADC drops, FIFO overflows, audio underruns, audio drops and late DMA interrupts were zero. These are preserved historical starting observations, not changes attributable to a new test. Their origin is unknown. Latest total was 3458 us.

ADC raw counts: I min/mean/max/RMS 1987/2077/2161/46; Q 1940/2073/2203/87. Q varies substantially in this snapshot; quiet DC bias is not established. Counts are automated diagnostics, not fresh scope voltages. Previous approximately 1.65 V/240 mVpp scope measurements are historical and are not reused as new readings.

Requested user confirmation: mixer/dividers/wiring; actual generator carrier/amplitude/offset/load/modulation settings and enabled states; GP26 bias source and fresh DC voltage; GP27 mean/Vpp/min/max after the resistor with AM off/on at 30%; filtered audio probe point and available scope/FFT measurement. Audio ground connection is not advised until its circuit is known. GP16 raw PWM is not reconstructed audio.

No rebuild, flashing, reboot, DSP calculation change, settings change or cumulative-counter reset was performed. Raw timestamped help/status output and structured preflight records are saved alongside this file. Waiting for readings before the new >=60-second measurement or any physical-signal conclusion. Nothing committed or pushed for this test.
