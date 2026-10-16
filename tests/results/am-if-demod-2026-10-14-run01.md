# Section 4.5.5: AM real-input timing and pending demodulation measurements

14 October 2026. Timing: **PASS for this observed interval, with only 63 us (1.48%) margin**. Demodulation: **UNVERIFIED**; no recovered-audio measurements have been supplied. This is a timing result under a broadly user-confirmed setup, not independently verified physical signal conditions.

## Configuration and provenance

Local branch testing/am-iq-optimized, publication commit bfa22b8c150ee87c1808418a08cb066160e3834d. Runtime am-optimized-v1 fingerprint 86d0e4fe2bd42cc00d51896696ec6402e11beee3f1282a7385d0eff4441f9497 matches the published artifact. Embedded base commit remains 34bde79a126fc4a7737a4e7194b63833e2aace05. Artifact SHA256 0bca44e437d8579749c61407390c5f12f947c87a45c53818ebc00163f6768f92. No independent binary readback. Clock reports 120 MHz.

Existing settings matched Preset 2: AM, IF-I, +8000 Hz, BW 2, AGC 4/manual, gain exponent 4 = factor 16, volume 25%; squelch -1/off, hang 3, swap 0, IQ correction 0, de-emphasis 0, bass 0, treble 0, blanker 0, notch 0, noise reduction 0, smoothing 10, threshold 1, CW 700 Hz. Revision 1 remained unchanged. Firmware exposes these settings rather than a preset number.

No rebuild, flash, reboot, DSP calculation change, preset application or settings transition was performed. The host disabled automatic serial streaming for coherent requested status records, queried help/status and used measure start/stop for a new window. These commands never reset cumulative counters. After verification, 5.1 seconds settling preceded the measurement. OLED and controls stayed available, but responsiveness was not reconfirmed for this run.

User broadly confirmed intended setup and requested timing now: mixer disconnected; dividers retained; CH1 GP27, 8 kHz sine carrier, 300 mVpp High-Z +2.5 V offset, internal 1 kHz AM at 30%; GP26 intended DC bias. Bias method and exact readbacks not supplied; preflight Q variation conflicts with quiet DC assumption. The user requested proceeding without the detailed measurements. Fresh GP27 mean/Vpp/min/max with AM off/on and GP26 DC were not supplied. The 0?3.3 V physical-input requirement remains unverified by scope; no new near-rail detections alone does not verify every physical peak.

Raw Q remained varying: sampled count range 1922..2209, AC RMS 87..90 counts. This conflicts with intended quiet DC. IF-I replaces Q digitally before receiver processing; it does not make the physical pin quiet or bypass its diagnostic checks.

## Recorded interval

Host monotonic first-line start-to-stop acknowledgements: 62.0448071 s. Exact device window: 62.046272 s, 1601226642 to 1663272914 us since boot. Throughput 234.373469 blocks/s versus nominal 234.375. Window endpoints determine exact counts; 32-block status publication and serial delivery can shift outer snapshot timestamps/counts.

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| B | 375056 | 389598 | 14542 |
| deadline | 6 | 6 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |
| clips | 189643 | 189643 | 0 |

The six starting deadline misses and 189643 near-rail detections predate the measurement and stayed unchanged during it. Preflight had two deadline misses, so four arose between preflight and this window; their causes were not captured. The interval pass must not be described as fault-free continuous operation since boot.

Cumulative boot maximum was 4417 us at interval start and 4417 us at the end, pre-existing and separate from the interval maximum of 4203 us. Largest sampled latest total was 3459 us; snapshots alone miss peaks.

| Block class | Count | Late | Interval maximum total, us |
| --- | ---: | ---: | ---: |
| ORDINARY | 14315 | 0 | 3951 |
| SPECTRUM | 227 | 0 | 4203 |

| Stage | Ordinary mean / max, us | Spectrum-block mean / max, us |
| --- | ---: | ---: |
| TOTAL | 3428.142 / 3951 | 4074.930 / 4203 |
| IN | 553.711 / 581 | 554.423 / 560 |
| DSP | 2642.768 / 2998 | 2642.216 / 2706 |
| STAT | 34.882 / 91 | 34.819 / 51 |
| FFT | 0.000 / 0 | 645.463 / 717 |
| OUT | 196.035 / 360 | 196.819 / 277 |
| CICMIX | 498.818 / 504 | 498.709 / 503 |
| FILTER | 1599.757 / 1805 | 1599.718 / 1654 |
| DEMOD | 365.214 / 468 | 365.115 / 387 |
| CAPTURE | 2.469 / 3 | 2.449 / 3 |
| FFTFWD | 922.473 / 988 | 922.300 / 929 |
| FFTBINS | 222.242 / 363 | 222.419 / 274 |
| FFTINV | 386.518 / 440 | 386.471 / 390 |

Receiver DSP contains CIC/mixing, filter and demodulation stages. FFTFWD/FFTBINS/FFTINV subdivide FILTER; FFT is separate spectrum-display preparation. Nested timings and independent maxima must not be summed. Latest FFT=0 is not evidence that spectrum work is absent. Existing firmware instrumentation remained unchanged. Separate window bookkeeping averaged 4.185 us, maximum 66 us; it is diagnostics outside the retained inclusive processing/audio window.

Final stopped status (latest timing may represent a block outside the exact window):

    B=389598 rev=1 AM IF-I tune=8000 bw=2 agc=4 gain=2^4 vol=25
    us=3414 max=4417 deadline=6 ADCdrop=0 FIFO=0 AUDunder=0 AUDdrop=0 IRQlate=0
    TIMING us latest/max PROC=3223/4194 OUT=191/360 IN=553/583 DSP=2636/3013 STAT=33/111 FFT=0/976
    RX latest_us CICMIX=499 FILTER=1600 DEMOD=360 CAPTURE=2 FFTFWD=921 FFTBINS=225 FFTINV=386

## Compact Section 4.5.5 results

| Criterion | Measured result | Assessment |
| --- | --- | --- |
| Continuous processing in measured window | 14542 blocks / 62.046272 s; 234.373469 blocks/s | Observed pass |
| Deadline and streaming faults | No additional faults; interval max 4203 / 4266 us | Pass with narrow 63 us margin |
| Input near-rail detections | Increase 0 | No new detections; fresh scope limits unverified |
| Recovered tone within 980?1020 Hz | Not measured | Unverified |
| AM-off 1 kHz component at least 10 dB lower | Not measured | Unverified |
| Audio amplitude versus 20/40/60% depth | Not measured | Unverified |
| Recovered-audio clipping | Not inspected | Unverified |

Next step: identify the actual filter/amplifier and audio probe point. GP16 is raw PWM, not reconstructed audio. If its wiring is unclear, obtain a schematic/photo before advising scope-ground attachment. Then guide one measurement at a time: 30% AM-on recovered tone/frequency/amplitude/clipping; AM-off residual 1 kHz with carrier/gain/volume/scope unchanged; depth comparison only after A/B are satisfactory. Frequency-selective measurement is needed for the 10 dB criterion; broadband RMS does not establish it. Save a representative AM-input capture and matched AM-on/off recovered-audio captures once those points/settings are confirmed. No such captures have been supplied.

Raw timestamped evidence and structured summary are beside this report. Prior unsuccessful and successful AM-IQ records remain unchanged. No commit or push for this AM real-input test.

## Subsequent silent-speaker check

User reports GP16 through a filter to a speaker, with no audible sound. Read-only status shows volume 25%, squelch off, digital audio RMS 234 counts and peaks 544/480 counts. Nonzero samples do not establish a recovered 1 kHz tone or a functioning analog output. Filter topology, amplifier presence, power and speaker load remain unverified; a photo/schematic is required before advising a scope-ground connection. No volume/gain or physical connection changes were requested.

Cumulative deadline misses rose from 6 at the measurement end to 7 by this later check. This one additional miss is outside the saved successful interval, with cause/stage not captured. It further limits the conclusion to the observed window. Raw evidence: am-if-demod-2026-10-14-audio-check01.txt and .json.

## User-supplied audio schematic

The schematic shows GP16 through C1 (100 uF) and R1 (100 ohm), with C2 (470 nF) from the filtered node to ground. No audio amplifier is shown. A low-impedance speaker can load this passive network heavily; this plausibly explains low volume but is not a measured diagnosis. Connector switching and physical implementation have not been verified. The identified scope point is the R1/C2 junction, referenced to C2 ground/Pico GND, using a high-impedance input. Proposed next measurement temporarily removes the speaker/headphone load and retains AM 30% and firmware gain/volume. The user has not yet confirmed that action or supplied audio measurements. The schematic was viewed in chat; no local image artifact is claimed.

## Loaded-output observation

The user cannot disconnect the soldered speaker and reports it was audible using Dawson firmware previously. Therefore no disconnection is required, and speaker loading is not established as the cause of silence. With the speaker connected, the user reports an approximately triangular waveform at 240 kHz and 60 mVpp. No screenshot, scope model or independent probe-location confirmation was supplied. This frequency matches the firmware PWM carrier, suggesting residual carrier ripple rather than recovered audio. The reading is not a frequency-selective 1 kHz amplitude measurement and neither proves nor disproves demodulation. Next: inspect a roughly 5 ms time span and obtain a screenshot/model before choosing a suitable audio-component measurement.

## Output scope photograph reviewed

A user-supplied Tektronix 2 Series MSO photograph shows CH1 at 20 mV/div, AC coupling, x10, 20 MHz bandwidth limit; horizontal 400 us/div, 50 MS/s, 200k points. Displayed CH1 peak-to-peak statistic is 68.00 mV. The waveform is a dense fast-ripple band with no clear repeatable 1 ms audio waveform visible. At this timebase a 1 kHz period would span 2.5 divisions. This is consistent with residual PWM ripple; it does not exclude a smaller hidden audio component or independently verify the carrier frequency. No selective 1 kHz amplitude measurement is established. CH2 displays 260 mVpp and 1.647 V mean, but its probe location is unconfirmed, so those are not assigned as GP27 verification. The photo is supporting ripple evidence, not proof of recovered AM audio; viewed in chat, not saved as a local image artifact.

## User-reported modulation-depth diagnostic

User transcribed P0 AUD=70 at AM depth 0% and AUD=230 at 30%. This is a broadband digital RMS ratio of 3.286, equivalent to 10.33 dB. It supports a response to applied AM depth but does not establish the recovered tone frequency or the proposed 10 dB frequency-selective 1 kHz suppression target. No paired selective spectrum measurements, distortion measurements or analog amplitude readings were supplied. The requested comparison used unchanged carrier, gain exponent 4 and volume 25%; per-reading settings snapshots were not captured. A later read-only runtime check is retained in JSON. The AM-demodulation acceptance criteria remain unverified.

## Follow-up, 15 October 2026

The user reports no visible 1 kHz peak in the audio-output FFT. No cursor amplitude, noise-floor bound or final FFT/acquisition settings were supplied. Recovered-tone frequency and selective AM-off suppression remain unverified; this observation does not by itself identify a DSP fault. Next proposed check uses the known input carrier to validate the scope FFT: source CH2 at GP27, 0?12 kHz view and 10 ms/div acquisition, without changing the generator or receiver.

### Output scaling finding and revised next step

Inspection of stream_audio shows output = 250 + trunc(sample * 249 * volume / (32768 * 100)). At volume 25%, a sample magnitude must reach 527 counts to move the PWM duty by even one count. AUD is measured before this conversion. A previously sampled audio peak of 384 counts would therefore produce a constant duty code for that whole block (after any interpolation transient from the preceding block), despite nonzero digital audio RMS. Other sampled blocks have exceeded the threshold; this is not evidence that every block is suppressed. Quantization is a concrete candidate explanation for weak/missing analog audio. The revised next step is a separate volume-only trial at 100%, retaining gain exponent 4 and AM depth 30%, then checking the output FFT. The previous input-FFT validation suggestion has not been performed and is superseded as the immediate next step. No firmware or runtime setting was changed by the agent.

## Volume-100% trial: reported 1 kHz peak, 15 October 2026

Following the requested volume-only change to 100% (gain exponent 4), the user reports a peak near 1 kHz at approximately -60 dBm on the filtered-output CH1 FFT, but still no audible sound from the speaker. This is consistent with recovered audio becoming visible when PWM quantization is reduced. It is not an isolated proof of the mechanism or a full demodulation pass. Exact cursor frequency, FFT amplitude reference/window/resolution and noise-floor margin are not verified, and no dBm-to-voltage conversion is made. The passive speaker circuit can yield a weak audible output; actual speaker performance is still unverified.

This trial differs from the original volume-25% timing interval and broadband AUD comparisons. The next required selective comparison is AM depth 0%, carrier still on, keeping volume 100%, gain exponent 4, scope settings and loads unchanged. Read the level at the same 1 kHz frequency after five seconds, not the largest peak elsewhere. A second saved capture is pending. Full timing at this new volume and precise recovered-frequency acceptance are not yet established.

## Volume-100% trial: AM depth 0% comparison

In response to changing only AM depth to 0% with carrier output instructed to remain on, the user reports no visible 1 kHz peak. The preceding 30% trial showed a peak near 1 kHz at approximately -60 dBm. This on/off behavior supports qualitative recovery of an AM-dependent audio component. No off-state level or noise-floor bound has been supplied, so the 10 dB selective suppression target cannot yet be marked passed. A disappearing peak is not zero residual. Exact tone-frequency tolerance, distortion/clipping, depth linearity and audible speaker output remain unverified. Requested next evidence is the trace/noise-floor level near 1 kHz on unchanged FFT settings and an AM-off capture.

## Updated selective AM-on/off readings, 15 October 2026

The user now reports approximately -56 dBm for the peak near 1 kHz at 30% AM, and approximately -80 dBm for the background near 1 kHz at 0% AM, with no visible off-state peak. The latest paired report supersedes the earlier rough -60 dBm on-state estimate. The on-peak/background difference is (-56)-(-80) = 24 dB. Under the instructed unchanged FFT, gain exponent 4, volume 100% and load, this supports the proposed 10 dB reduction bench target. The -80 dBm value is a background/noise-floor reading, not a resolved residual tone; exact residual-tone suppression and zero residual are not claimed. Matched captures/data exports have not been supplied.

| Criterion | Current evidence | Assessment |
| --- | --- | --- |
| AM-dependent recovered component | Peak near 1 kHz at 30%, no distinct peak at 0% | Supported by user observations |
| Modulation-off reduction target | On peak -56 dBm; off background -80 dBm; approximately 24 dB separation | 10 dB bench target supported, conditional on unchanged settings; approximate noise-floor comparison |
| Tone frequency within 2% | Approximately 1 kHz; exact cursor reading absent | Unverified quantitatively |
| Audio amplitude versus depth | No 20/40/60% comparison yet | Unverified |
| Clipping/distortion | No adequate inspection/measurement | Unverified |
| Audible output | User still reports no sound from speaker | Not demonstrated |

Next planned step is a separate 20% AM-depth reading at the same 1 kHz FFT peak, followed by 40% and 60% if no clipping is found. Carrier amplitude/frequency, gain, volume, scope settings and connected load must stay unchanged for this comparison. Original timing evidence remains the volume-25% interval; no new timing pass at 100% volume is claimed.

## Depth series: corrected 20% reading

The user corrected the 20% AM peak reading from -67 dBm to approximately -62 dBm; only -62 dBm is used for analysis. The requested settings remain gain exponent 4, volume 100%, unchanged carrier and scope/load. Relative to the 30% reference (-56 dBm), the reported reduction is 6 dB; ideal amplitude proportionality would predict approximately 3.52 dB. This preliminary, approximate comparison establishes neither linearity nor a fault. Collect 40%/60% and check repeatability before interpreting the difference.

## Depth series: 40% reading

User reports approximately -53 dBm at the same nominal 1 kHz FFT peak after setting AM depth to 40%, with all other settings requested unchanged. The partial series is 20%: -62 dBm, 30% reference: -56 dBm, 40%: -53 dBm. Level increases with depth; 60%, repeatability and clipping checks remain pending.

## Initial depth sweep complete

User reports approximately -48 dBm at 60% AM. All entries are manual approximate FFT readings at nominal 1 kHz, with gain exponent 4 and volume 100% requested unchanged.

| AM depth | Reported 1 kHz peak |
| --- | ---: |
| 20% | -62 dBm (corrected) |
| 30% | -56 dBm (earlier reference) |
| 40% | -53 dBm |
| 60% | -48 dBm |

The peak increases monotonically with modulation depth. From 20% to 60%, the observed increase is 14 dB, versus 20 log10(60/20) = 9.54 dB for ideal proportional voltage amplitude. Consequently this is not established linear response. Approximate readings, input modulation accuracy, FFT settings and output PWM quantization could contribute; none is isolated by this sweep. Clipping/distortion has not been checked, and absolute output voltage/power is not inferred from dBm without the reference convention. Next: repeat the 30% reading with unchanged settings to assess drift/repeatability before further conclusions.

## Closing observation and reporting scope, 15 October 2026

User confirms that returning AM depth to 30% restores approximately -56 dBm. This supports repeatability of that approximate reference after the sweep; it does not establish a calibrated uncertainty or full proportional response. The evidence is sufficient for a bounded report section, not a claim that every AM-demodulation acceptance criterion passed. No further hardware action, firmware change or counter reset was performed at closure. The newly collected real-IF evidence remains local and has not been committed or pushed.
