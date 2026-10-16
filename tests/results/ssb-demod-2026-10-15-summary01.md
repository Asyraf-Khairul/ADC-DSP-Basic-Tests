# USB/LSB electrical selection and timing ? 15 October 2026

Direct ADC-injection evidence for Section4.5.7. Existing fm-optimized-v2 firmware was used throughout; no rebuild, flash, DSP/rate/deadline change or cumulative-counter reset. New host collector and test evidence remain local and uncommitted.

## Assessment

Observed USB/LSB selection reverses when the generator displayed Q phase changes from90 to270degrees. Both initial-condition timing windows pass. Physical quadrature and exact input voltage matching were not measured: the operator requested proceeding without those scope checks. These limits prevent an unqualified controlled-quadrature/full-receiver pass.

## Electrical observations

| Generator displayed Q phase | Mode | Output near1kHz | FFT level / background | Interpretation |
| --- | --- | --- | --- | --- |
| 90deg | USB | Visible peak | -43dBm peak | Stronger condition |
| 90deg | LSB | Unresolved | approximately -75dBm background | Not an exact residual-tone measurement |
| 270deg | USB | Unresolved | approximately -72dBm background | Not an exact residual-tone measurement |
| 270deg | LSB | Visible peak | -43dBm peak | Stronger condition |

At displayedQ90,USBpeak-43dBm is32dB above the LSBbackground near-75dBm. At displayedQ270,LSBpeak-43dBm is29dB above the USBbackground near-72dBm. Both support the proposed15dB bench target if scope reference, bandwidth, load and all other settings remained identical as instructed. The unwanted tones were unresolved; report peak-to-background separations, not exact rejection or zero output. FFTdBm reference/impedance are unverified, so delivered speaker power cannot be calculated.

Reference repeat: after returning receiverUSB and displayedQ90, operator answered yes to return of approximately1kHz/-43dBm. This is a prompted approximate confirmation, not an independently supplied numerical cursor readback. The frequency is consistent with the proposed980-1020Hz bench target, but a precise error or accuracy pass is not established for all four cases. No validated capture filenames supplied yet.

## Input and source conventions

Generator CH1 toGP27/I andCH2 toGP26/Q through retained470ohm/1kohm dividers, mixer disconnected, shared grounds. Both instructed continuous1kHz sine,300mVpp displayed,High-Z,+2.5Voffset,modulationoff,CH1phase0,CH2initialphase90. CH2 changed from prior DC bias to sine; later only displayedCH2phase changed90to270 and back. User confirmed setup set, then waived fresh means/minima/maxima/Vpp and physical quarter-period checks. Serial ADC statistics indicate two AC signals near midscale but do not establish matched calibrated amplitudes or a250us lead/lag.

Code inspection: GP27I is sampled first, GP26Q next; sampling is interleaved. The negative-exponent forward FFT retains positive bins forUSB and negative bins forLSB. With I=Acos(wt),Q=Asin(wt), QlagsI by a quarter-cycle andUSB is the expected wanted mode. QleadingI by a quarter-cycle givesLSBwanted. Displayed generator phase convention cannot establish which physical signal leads; this was not scoped. Both final demodulation paths return the real filtered component; selection occurs in the complex FFT filter.

Settings throughout final comparisons: IQ,tune0Hz,BW2,manualAGC4,gain_log2=4(x16),volume25%,sq=-1,hang3,swap0,iqcorr0,deemph0,bass0,treble0,blanker0,notch0,nr0,smooth10,threshold1,cw700. Whole configuration verified over serial. Presets6/7 selectUSB/LSB, but subsequent switches used mode-only commands to preserve settings. Initial accidentalx2 gain was changed tox16 before all timing/functional results: sampledPCMpeak500 atvolume25 was below the first nonzero PWM target-step threshold527. No DSP implementation changed.

Output probe instructed at existing100ohm R1/470nF C2 junction after100uF coupling capacitor fromGP16; ground to circuit ground, speaker connected. CH1AC coupling,10ms/div acquisition andFFTspan0-2kHz/sourceCH1 instructed. Actual full scope readbacks,FFTwindow/reference,probe point and load impedance were not independently inspected.

## Timing results

Both windows allowed at least5.1seconds settling, then captured every processed block for at least60seconds. Cumulative counters were preserved; only diagnostic measurement accumulators started/stopped. Both windows used initial displayedQ90; no separate all-blockQ270 window was collected.

| Mode | Device seconds | Host seconds | Blocks | Blocks/s | Ordinary / spectrum blocks | Ordinary / spectrum max us | Overall max us | Headroom us | Revision |
| --- | ---: | ---: | ---: | ---: | --- | --- | ---: | ---: | --- |
| USB | 62.046212 | 62.0441702 | 14542 | 234.373696 | 14314 / 228 | 3656 / 3867 | 3867 | 399 | 3 to 3 |
| LSB | 62.049315 | 62.0475246 | 14543 | 234.378091 | 14316 / 227 | 3643 / 4129 | 4129 | 137 | 4 to 4 |

Deadline4266us, nominal234.375blocks/s. USBheadroom399us(9.35%); LSBheadroom137us(3.21%). These are bounded observations, not guarantees for every signal, processing option or longer run.

| Counter | USB increase | LSB increase |
| --- | ---: | ---: |
| deadline | 0 | 0 |
| ADCdrop | 0 | 0 |
| FIFO | 0 | 0 |
| AUDunder | 0 | 0 |
| AUDdrop | 0 | 0 |
| IRQlate | 0 | 0 |
| clips | 0 | 0 |

Starting/ending boot maxima were4229/4229us in both windows, pre-existing rather than measured interval maxima. Largest sampled latest totals were3216us(USB) and3212us(LSB); serial snapshots do not capture every block. Exact device-window endpoints supply throughput and per-block accumulators supply maxima. USB latency and32-block status publication affect host observations. Nested stage timings and independent maxima must not be added; see individual reports for full stage tables and original start/end counters.

Near-rail history: first connected preflight77973, later before fixed tests1059439. The increase occurred during setup and was not time-resolved; no precise cause can be assigned. Both fixed windows preserved1059439 unchanged. Do not generalize zero in-window detections to the entire setup session.

After the LSBwindow through the final reference status: {"deadline_increase": 0, "near_rail_increase": 0, "streaming_increases": {"ADCdrop": 0, "FIFO": 0, "AUDunder": 0, "AUDdrop": 0, "IRQlate": 0}, "interpretation": "Differences between frozenLSB endpoint and final serial publication; mode transitions and phase reversal included. No new all-block timing-window maximum for those transitions."}. These cumulative observations do not replace a new all-block phase270 timing test.

Broadband digital audio snapshot comparison (supporting only): {"USB_median_AUD_RMS": 2818, "LSB_median_AUD_RMS": 334, "ratio": 8.437125748502995, "difference_dB": 18.523890439235466, "limitation": "Broadband pre-volume/pre-PWM snapshots under unverified physical phase; not selective electrical 1kHz rejection and not an accepted15dB bench result."}. It is not a selective output-frequency measurement.

## Remaining observations and limitations

Speaker audibility: user reports no audible tone in strongUSB at gainx16,volume25. Current-session UI responsiveness: user confirms OLED,encoder and both buttons responsive; retrospective confirmation rather than separately timestamped in-window check. Clipping/distortion: no verified output-waveform or quantified measurement. Photographs: not saved yet; user says they can save later, no filenames supplied. Useful remaining captures are physical input lead/lag and matched strong/weak FFTs; any later captures must carry actual date/settings and must not be presented as photographs of the logged intervals.

No full RF/mixer/antenna chain, calibrated sensitivity, speaker power, distortion performance or numerical hardware equivalence is established. Proposed frequency/rejection targets are bench targets, not certified specifications.

## Identity and evidence

Branch testing/fm-timing-optimized, commit 63f6610e8ab9e894b30543816e5c8033ace43c4a. Self-reported runtime identity matched {"BENCH_BUILD_LABEL": "fm-optimized-v2", "BENCH_SOURCE_SHA256": "c9424dd398f20e91897c62007188ab5e2d88b0e70e1e3567bc74c89aa570dae6", "BENCH_BASE_COMMIT": "bfa22b8c150ee87c1808418a08cb066160e3834d"}. No independent flash readback. Exact artifact SHA256 77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e. Binary embeds parentbfa22b8 plus source fingerprint because it predates the local publication commit.

- ssb-usb-timing-2026-10-15-run01.txt
- ssb-usb-timing-2026-10-15-run01.json
- ssb-usb-timing-2026-10-15-run01.md
- ssb-lsb-timing-2026-10-15-run01.txt
- ssb-lsb-timing-2026-10-15-run01.json
- ssb-lsb-timing-2026-10-15-run01.md
- ssb-mode-switch-2026-10-15-run01.json
- ssb-mode-switch-2026-10-15-run02.json
- ssb-mode-switch-2026-10-15-run03.json
- ssb-final-status-2026-10-15-run01.json
- ssb-demod-2026-10-15-preflight01.json andpreflight02.json
- ssb-demod-2026-10-15-settings01.json andsettings02.json

Full manual observation sequence and source/provenance are retained in the JSON beside this report. No commit or push.


Operator follow-up: UI responsive; no audible speaker tone in strongUSB. Captures not yet saved. Common inaudibility across AM/FM/USB suggests investigating the shared output path and level, but does not prove its cause. Provided circuit has passive coupling/100ohm filtering and speaker connection with no dedicated amplifier shown. Speaker impedance and delivered signal voltage/power are unverified; no inference of speaker power from FFTdBm. No gain/volume or firmware changes made.
