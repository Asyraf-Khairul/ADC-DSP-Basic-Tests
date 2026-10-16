# Section4.5.8 - Digital tuning and bandwidth verification

16 October2026, Pacific/Auckland. Electrical observations support functional digital tuning and bandwidth selection under the reported bench conditions. The final timing window FAILED the strict no-deadline-miss requirement.

## Identity and setup

Local branch `testing/fm-timing-optimized`, commit `63f6610e8ab9e894b30543816e5c8033ace43c4a`. Runtime reports `fm-optimized-v2` with source fingerprint `c9424dd398f20e91897c62007188ab5e2d88b0e70e1e3567bc74c89aa570dae6` and embedded base `bfa22b8c150ee87c1808418a08cb066160e3834d`. This artifact predates the local commit. Self-reported identity matched local metadata; no independent flash readback. UF2 SHA256 `77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e`.

No rebuild, flash, firmware changes, clocks/rates changes or cumulative diagnostic resets. No commit or push. Existing untracked work preserved; new host-only collectors and evidence remain local.

Instructed generator CH1 continuous8kHz sine,300mVpp High-Z,+2.5V offset,modulationoff via divider toGP27;GP26 externally biased; mixer disconnected; speaker retained. User confirmed connections generally.

Fresh operator scope readings: GP27 mean1.65V, minimum1.52V, maximum1.78V; GP26 DC1.65V. The GP27 extrema span260mV, not a separately supplied Vpp readout. These values are within ADC input range. Exact current generator display readbacks and physical wiring were not independently inspected.

Scope CH1 instructed at the filtered R1(100ohm)/C2(470nF) junction, circuit-ground reference, AC coupling,20mV/div,10ms/div and FFT0-4kHz. SR2.5MS/s and RL250kpoints operator-confirmed:100ms acquisition,10Hz raw FFT-bin spacing. Hann/Hanning window confirmed. Keep measurement settings and speaker load fixed. Reference impedance for FFT dBm was not verified; readings are not speaker-power measurements.

## Code basis

IF-I selects GP27 and substitutes midscale for internal Q, so this test avoids generator quadrature-sign ambiguity. The receiver mixes with exp(-j*2*pi*tune*t) and USB retains positive translated frequencies. For a fixed8000Hz input, expected audio frequency is8000 minus actual tuning. NCO rate30kHz,32-bit phase increment, tuning commands in1Hz steps; numerical increment quantization below0.000007Hz. Phase lookup uses11MSBs, not FFT-bin quantization. Actual clocks are not independently calibrated.

USB filters keep bins3 through16/19/22/25/28 for bandwidth indices0/1/2/3/4 respectively. At30kHz/256, bin spacing117.1875Hz. The256-point Hann window and overlap-add shape transitions. Retained bin centres are not measured -3dB cutoffs. At2.5kHz (near bin21.33), the tone is inside bw4 and outside bw0, sufficiently away from their retained-bin boundaries for a qualitative selection comparison.

## Digital tuning results

USB IF-I, bandwidth4, manual gain x16, volume25%, effects and squelch off. Generator held fixed at intended8kHz. At least5.2s settling before each serial condition snapshot.

| Requested tuning Hz | Expected nominal audio Hz | Operator reported audio Hz | Nominal error at reported precision Hz |
| ---: | ---: | ---: | ---: |
| 6500 | 1500 | 1500 (rounded) | 0 (rounded) |
| 7000 | 1000 | 1000 (rounded confirmation) | 0 (rounded) |
| 5500 | 2500 | 2500 (rounded) | 0 (rounded) |
| 6500 repeated | 1500 | 1500 (rounded confirmation) | 0 (rounded) |

A later bandwidth4 reading at tune5500 gave2498Hz, nominal error-2Hz. Functional translation and return are supported at reported precision. Do not claim exact zero error. The provisional +/-20Hz tolerance was conditional on resolution:10Hz FFT-bin spacing is confirmed, but first rows were reported in rounded kHz and cursor-step/calibration were not independently verified, so a formal all-row precision pass is not established.

Initial6500 level was reported -51dBm, corrected to -57dBm, then flagged by the operator as likely a readback error after repeat -51dBm. Preserve that history; do not use it to claim a precise amplitude-repeatability difference.7000 and5500 tuning observations each initially reported approximately -51dBm.

## Bandwidth results

Fixed tune5500Hz; only bandwidth changed4 ->0 ->4, all other firmware settings verified unchanged. Scope settings instructed unchanged.

| Bandwidth | Cursor frequency | Displayed level | Observation |
| --- | --- | --- | --- |
| 4 | 2.498kHz | -52.25dBm | Resolved output peak |
| 0 | Same2.498kHz cursor instructed | -81dBm | No visible peak; unresolved background reading |
| 4 repeat | Approximately same, operator confirmation | Approximately same, no new numeric reading | Peak returned |

Wide peak to narrow background separation:28.75dB. This is not an exact measurement of rejected-tone amplitude, zero output, receiver dynamic range or a -3dB cutoff. A common-passband reference was not measured; conclusion is limited to the selected2.5kHz tone.

## Photographs

| Requested filename | Reported save | Inspection |
| --- | --- | --- |
| bandwidth4_2500Hz | Operator confirmed | Not supplied/inspected |
| bandwidth0_2500Hz | Operator confirmed | Not supplied/inspected |

Exact filenames/extensions/paths were not separately supplied. Suggested caption: Filtered-output FFT comparison with an intended8kHz input and DSP tuning+5500Hz: (a) bandwidth4, resolved peak at2.498kHz and -52.25dBm; (b) bandwidth0, no resolved peak and approximately -81dBm background at the same cursor position. Matching measurement settings were instructed and retained according to the test procedure. Figures must be checked against actual saved files before publication.

## Final all-block timing

Final settings: `{"agc": 4, "bass": 0, "blanker": 0, "bw": 4, "cw": 700, "deemph": 0, "gain_log2": 4, "hang": 3, "input": "IF-I", "iqcorr": 0, "mode": "USB", "notch": 0, "nr": 0, "smooth": 10, "sq": -1, "swap": 0, "threshold": 1, "treble": 0, "tune": 5500, "volume": 25}`. Settling at least5.1s before measure start. Settings revision30 throughout. Dedicated all-block window accumulators reset at window start; cumulative health and timing counters preserved. Every ordinary/spectrum block contributes to captured counts/maxima; serial latest snapshots alone do not capture every block.

Device window **62.045896s**; host first-start-record to stopped-record **62.0456658s**. Processed **14542** blocks, **234.374889blocks/s**, versus nominal234.375.

| Counter | Start | End | Change |
| --- | ---: | ---: | ---: |
| B | 703359 | 717901 | 14542 |
| deadline | 1 | 2 | 1 |
| clips | 398614 | 398614 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |

| Block class | Count | Late | Mean inclusive us | Maximum inclusive us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14314 | 0 | 3287.540 | 3841 |
| SPECTRUM | 228 | 1 | 3589.382 | 4327 |

Interval maximum **4327us**, deadline4266us, headroom **-61us** (61us over). Largest serial-sampled latest total3317us. Cumulative maximum since boot **4408 -> 4408us**, inherited from earlier setup, not this window.

One new late block was classified SPECTRUM. No ADC drops, FIFO overflows, audio underruns, audio drops, late DMA interrupts or near-rail increments occurred. Earlier setup had one deadline miss; final cumulative count2 includes that earlier miss plus this one. Throughput remained nominal, but the strict no-additional-misses criterion FAILS.

| Stage | Ordinary mean / max us | Spectrum mean / max us |
| --- | ---: | ---: |
| TOTAL | 3287.540 / 3841 | 3589.382 / 4327 |
| IN | 553.633 / 579 | 554.338 / 567 |
| DSP | 2502.512 / 2880 | 2503.355 / 2834 |
| STAT | 34.611 / 86 | 34.789 / 78 |
| FFT | 0.000 / 0 | 297.412 / 506 |
| OUT | 196.103 / 367 | 197.614 / 339 |
| CICMIX | 498.725 / 504 | 498.684 / 503 |
| FILTER | 1463.538 / 1645 | 1463.768 / 1566 |
| DEMOD | 345.178 / 461 | 345.548 / 453 |
| CAPTURE | 2.483 / 19 | 2.395 / 3 |
| FFTFWD | 861.655 / 927 | 861.724 / 885 |
| FFTBINS | 176.906 / 288 | 176.965 / 230 |
| FFTINV | 356.449 / 398 | 356.539 / 381 |

Receiver DSP is the largest mean stage, with its FFT filter the largest receiver substage; spectrum preparation adds work on the late block class. Per-stage maxima are independent and nested: do not sum maxima or infer that their peaks occurred on the late block. Exact late-block cause is not proven. Inclusive measurement still wraps process_samples plus stream_audio. Bookkeeping outside that window was measured separately: {"max_us": 66, "sum_us": 62697}.

## Report handoff and limits

Write this section as functional tuning/bandwidth observations plus a separate timing limitation. Do not call all three checks a pass. No further optimization or retest was performed to erase this failure. Keep the earlier setup miss, cumulative maxima and near-rail history separate from final interval deltas.

Measurements are operator-reported; serial settings/counters are automated evidence; photos were reported saved but not inspected. UI responsiveness was not separately verified in this timing run. No speaker audibility, antenna/mixer/LO operation, RF sensitivity or complete receiver validation is claimed. No all-mode or long-duration timing guarantee.

Machine-readable summary: `tuning-bandwidth-2026-10-16-summary01.json`; session plan/manual history: `tuning-bandwidth-2026-10-16-plan01.json`; all-block raw/JSON: `tuning-bandwidth-timing-2026-10-16-run01.*`. Raw logs have been hash-verified; hashes are in the summary JSON.
