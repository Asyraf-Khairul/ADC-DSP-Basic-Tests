# FM real-I timing ? 15 October 2026

**FAIL against the 4266 us deadline under the observed conditions.** Throughput stayed nominal and no block-loss counters increased, but 54 spectrum-update blocks were late. An additional input-condition issue is 2360 near-rail detections. FM demodulation performance is not established.

## Identity, setup and procedure

Branch testing/am-iq-optimized, publication commit bfa22b8c150ee87c1808418a08cb066160e3834d. Runtime label am-optimized-v1; source fingerprint 86d0e4fe2bd42cc00d51896696ec6402e11beee3f1282a7385d0eff4441f9497, embedded base 34bde79a126fc4a7737a4e7194b63833e2aace05. Runtime identity matched the published tested artifact SHA256 0bca44e437d8579749c61407390c5f12f947c87a45c53818ebc00163f6768f92; no binary readback. Runtime clock reported 120 MHz. ADC rate/block size and audio rates remain unchanged.

User confirmed requested FM setup is on: mixer disconnected; existing 470 ohm/1 kohm dividers retained; generator CH1 to GP27, 8 kHz sine carrier, displayed 300 mVpp High-Z +2.5 V offset, internal sine FM 500 Hz with 500 Hz deviation. GP26 bias arrangement remains unverified; it varied in prior AM diagnostics.

No fresh FM scope mean/Vpp/min/max or independent generator readbacks were supplied. Earlier AM measurements are not treated as fresh FM evidence. The user confirmed the requested setup was on. GP26 wiring/output state needs clarification because it reads close to ground.

Preset 4 verified by all runtime settings: FM, IF-I, tune +8000 Hz, BW 2, AGC 4/manual, gain exponent 0 (factor 1), volume 25%; squelch -1/off, hang 3, swap 0, IQ correction 0, de-emphasis 0, bass 0, treble 0, blanker 0, notch 0, noise reduction 0, smoothing 10, threshold 1, CW 700 Hz. Revision 5 remained unchanged. User selected the preset before connection; no settings transition was commanded by the collector.

No rebuild, flash, reboot, DSP/sample-rate change or cumulative-counter reset. The collector sent stream 0, help, status and measure start/stop, after verifying no existing capture was active. Settling was at least 5.1 seconds. Existing OLED and controls remained available, but their responsiveness was not independently confirmed for FM.

## Exact window and preserved cumulative values

Host first-line acknowledgement duration: 62.1139908 s. Device capture duration: 62.114179 s, between device timestamps 3798103065 and 3860217244 us. 14558 blocks at 234.374828/s versus nominal 234.375. Window endpoints define exact counts; normal 32-block publication and serial/command latency affect outer status receipt times.

| Counter | Window start | Window end | Increase |
| --- | ---: | ---: | ---: |
| B | 889949 | 904507 | 14558 |
| deadline | 99 | 153 | 54 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |
| clips | 213125 | 215485 | 2360 |

Pre-settling snapshot had B=888704, deadline=95, near-rail count=212920. Values increased before capture; these earlier increments are excluded from the measured-window differences above. Cumulative total maximum was already 4958 us at window start and remained 4958 us. It is not the interval maximum.

All-block interval maximum: 4465 us, 199 us (4.66%) over the deadline. Largest sampled latest total: 3494 us, which alone would miss the failure.

| Block class | Count | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14331 | 0 | 3453.412 | 4060 |
| SPECTRUM | 227 | 54 | 4184.273 | 4465 |

## Stage timings

| Stage | Ordinary mean / max us | Spectrum-block mean / max us |
| --- | ---: | ---: |
| TOTAL | 3453.412 / 4060 | 4184.273 / 4465 |
| IN | 552.530 / 580 | 553.335 / 574 |
| DSP | 2669.717 / 3119 | 2670.154 / 2905 |
| STAT | 34.732 / 95 | 34.617 / 44 |
| FFT | 0.000 / 0 | 715.824 / 815 |
| OUT | 195.682 / 349 | 209.048 / 260 |
| CICMIX | 498.808 / 504 | 498.855 / 504 |
| FILTER | 1632.392 / 1898 | 1632.837 / 1794 |
| DEMOD | 359.620 / 484 | 359.339 / 377 |
| CAPTURE | 2.472 / 3 | 2.498 / 3 |
| FFTFWD | 922.580 / 1001 | 922.604 / 968 |
| FFTBINS | 254.733 / 481 | 255.022 / 343 |
| FFTINV | 386.529 / 437 | 386.626 / 410 |

DSP is the receiver processing aggregate. FILTER contains FFTFWD/FFTBINS/FFTINV; FFT is separate spectrum-display preparation. Do not sum nested stages or independent maxima. Timings include interrupt service. All 54 misses were in the spectrum-update class; no ordinary blocks were late. The filter is the largest receiver substage, and spectrum preparation adds about 715.8 us on update blocks. This identifies insufficient headroom on spectrum updates, not a causally isolated defect in a specific operation.

Separate window bookkeeping averaged 4.160 us and peaked at 65 us, outside the retained processing-plus-audio window. Existing receiver instrumentation remained enabled. Fault counters capture any resulting streaming losses; none occurred in this interval.

Final sampled status (may follow the exact window endpoint):

    B=904507 rev=5 FM IF-I tune=8000 bw=2 agc=4 gain=2^0 vol=25
    I min/mean/max/rms=1937/2071/2205/88 Q=5/13/25/1 MAG=124 AUD rms/peak=1648/3910 clips=215485
    us=3494 max=4958 deadline=153 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
    TIMING us latest/max PROC=3306/4752 OUT=188/349 IN=552/582 DSP=2716/3264 STAT=38/95 FFT=0/1500
    RX latest_us CICMIX=499 FILTER=1628 DEMOD=409 CAPTURE=3 FFTFWD=922 FFTBINS=252 FFTINV=386

## Input issue and next action

Sampled GP26/Q min/max counts: 2..25, mean range 13..13. GP27/I sampled min/max: 1934..2212. These are ADC counts, not calibrated scope voltages. The near-ground Q readings are consistent with additional low-rail detections, but the counter is combined across both channels; per-channel increments were not measured. IF-I ignores physical Q in its demodulation input by substituting midscale digitally; physical Q statistics and rail checks remain active.

First clarify what drives GP26 and establish its intended quiet midscale bias. Preserve this failed run. Repeat timing after input conditions are corrected; any later source optimization is a separate task and must preserve rates, diagnostic meanings and spectrum function. Do not claim a 500 Hz FM demodulation or continuous deadline pass from these counters. The spectrum-update headroom issue remains independent of the unused-input condition unless a retest establishes otherwise.

No 500 Hz frequency/amplitude, modulation-off or deviation-scaling result has been obtained. The speaker/filter issue from AM is not assumed solved. Raw timestamped log and structured JSON are saved beside this report. Nothing committed or pushed.

## GP26 connection clarified

After the run, the user answered ?nothing? when asked what drives GP26: it was unconnected/floating. This invalidates the intended quiet midscale input condition and is consistent with the low-rail readings. It does not by itself explain the 54 spectrum-block deadline misses. Establish and verify a quiet approximately 1.65 V bias on GP26 before a new timing run. No corrective connection has yet been confirmed; preserve this run as failed timing with an uncontrolled unused input.

Clarification: ?nothing? establishes no external source, not necessarily a truly floating pin. If the existing divider's 1 kohm shunt remains connected, GP26 is pulled toward ground. Its resistor continuity was not verified. In either case, the intended midscale bias is absent; the earlier floating description was an assumption.
