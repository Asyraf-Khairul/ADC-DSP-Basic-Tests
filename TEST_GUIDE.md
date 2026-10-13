# AFG1022 ADC/DSP verification

Record observations, not just PASS labels. Building firmware is not a hardware
test. None of the hardware checkpoints below has been executed by the coding
agent. Suggested thresholds are bench acceptance targets, not certified specs.

## Operating without a serial monitor

Use encoder A=GP20, B=GP21, Button 1=GP17 and Button 2=GP22. Button 2 opens
Settings/selects/applies; turning the encoder browses or edits; Button 1 backs
out/cancels. On status screens, a short Button 1 press cycles pages. Turning
the encoder tunes in receiver modes, with Button 1 held for coarse steps.

All configuration commands below have an OLED menu equivalent: `preset N`
means Settings > Preset > N, `gain N` means Settings > Gain > N, and so on.
`bw` is Bandwidth; `iqcorr` is IQ corr; `nr` is Noise red; `smooth`/`threshold`
are NR smooth/NR thresh; `cw` is CW tone. `page N` means cycle to page N.
Apply changes with Button 2 and allow at least two seconds to settle. In BASIC,
only gain and volume affect processing; receiver features require a receiver mode.

USB `help`, `status` and `stream 1` are optional logging aids for normal firmware.
Use page 0 for ADC/audio readings and page 1 for loss counters without a PC.
For exact audio peak values and detailed numerical logs, use optional `status`;
the OLED audio view and an oscilloscope provide waveform feedback without serial.
The separate software self-test remains a USB-serial test, not the normal firmware.

## 1. Safe wiring and baseline

1. Disconnect the receiver/mixer outputs from GP27 and GP26 before injecting:
   never connect an AFG output against another circuit's output.
2. Connect generator, Pico and oscilloscope grounds together. Use a scope
   high-impedance input, not a 50-ohm termination on the Pico ADC.
3. Configure the AFG load indication as High-Z. Start with a 1 kHz sine,
   200 mV **peak-to-peak**, and +1.65 V offset. Check the actual waveform with
   the scope BEFORE connecting to the ADC. Expect about 1.55 to 1.75 V.
   All peaks, including modulation, must remain between 0 and 3.3 V.
   Generator displayed amplitude depends on its load setting.
4. CH1 goes to GP27 (I), CH2 to GP26 (Q) for two-channel tests.
   For single-channel tests hold the unused ADC at a quiet 1.65 V bias.
   A floating ADC is not a valid zero/noise test. IF mode ignores the unused
   channel digitally, but its displayed RAW statistics still describe the pin.
5. Flash the normal `firmware/adc_dsp_suite.uf2`. Default preset is BASIC.
   Use the encoder/menu for setup and OLED pages for feedback. Optionally connect
   a USB serial terminal (115200, newline) and send `stream 1` for a log.
6. Send `preset 0`. Check each ADC's min/mean/max and AC RMS against the scope.
   Approximate voltage = count * measured ADC reference / 4095.
   For 3.3 V reference: mean ~2048 counts and AC RMS ~88 counts for 200 mVpp
   sine. Calibrate against your board; small ADC offsets are expected.
7. With both inputs driven by quiet 1.65 V DC, wait two seconds. AC RMS and
   magnitude should fall substantially relative to driven sine inputs.
   They do not have to be exactly zero.

Checkpoint: no clipped input samples, both channels respond independently,
correct bias, no floating input, and correct pin identification.

## 2. Streaming health -- repeat for EVERY mode

Send `page 1` or press GP17's button. Observe for at least 60 seconds.

- `ADC D`: dropped ADC blocks. `F`: FIFO overflow observations.
- `AUD U`: audio starvation; `D`: dropped audio blocks.
- `MAX`: longest measured processing/enqueue time in microseconds.
- `LATE`: blocks exceeding 4266 us. `IRQ`: both chained DMAs completed
  before their interrupt was serviced.
- `B`: processed blocks, nominally about 234.375 per second.
- `C`: cumulative ADC samples within 4 counts of either rail.

Target: all loss/overflow/late-IRQ counters remain zero. Processing should fit
the 4266.67 us block period; occasional deadline misses need investigation
even if buffering prevents data loss. Counters are cumulative since boot.
Record start and end values. If B stops advancing or any loss counter rises,
do not claim continuous sampling; reboot and investigate workload/wiring.
Display/USB run on core 0; DSP/ADC/audio run on core 1.

### BASIC timing failure reported on 13 October 2026

The user transcribed the following readings from two photographs of the
encoder-enabled build based on commit `81dbbd9`. Retain both original photographs
with the bench records; the image files have not been supplied in this workspace.
The interval between photographs was not independently timed here.

| Reading | Start | End | Increase |
| --- | --- | --- | --- |
| B | 384 | 5248 | 4864 |
| ADC dropped blocks | 716 | 9807 | 9091 |
| Audio underruns | 712 | 9843 | 9131 |
| LATE | 384 | 5248 | 4864 |
| ADC FIFO overflows | 0 | 0 | 0 |
| Audio dropped blocks | 0 | 0 | 0 |
| Late DMA interrupts | 0 | 0 | 0 |
| Near-rail samples | 0 | 0 | 0 |
| MAX | 12538 us | 12538 us | 0 |

Every processed block missed the 4266.67 us budget in this observation. The ADC
response test is separate evidence; this run does not pass continuous processing.

Local build inspection confirmed `CMAKE_BUILD_TYPE=Release`, `-O3`, and
`BENCH_SELF_TEST=OFF`. Before profiling, the build UF2 and firmware alias shared
SHA256 `73B2AEDB8756269531031F38A55B418862C458F32848AA4EF3E92007771F56B5`.
A backup is retained locally as `firmware/adc_dsp_suite-81dbbd9-baseline.uf2`.
An additional read-only `status` request to the connected Pico on COM8 returned:

```text
B=34144 rev=0 BASIC IQ tune=0 bw=2 agc=4 gain=2^4 vol=25
I min/mean/max/rms=1938/2076/2210/88 Q=1933/2066/2211/90 MAG=123 AUD rms/peak=349/619 clips=0
us=12368 max=12538 deadline=34144 ADCdrop=64094 FIFO=0 AUDunder=64090 AUDdrop=0 IRQlate=0
```

Compiled-code inspection confirms `sqrtf` and floating-point helper calls in
the BASIC loop, which evaluates 1024 I/Q magnitudes per block. This identifies
a candidate cost, not a measurement proving which operation dominates.

### Repeat with stage timings

The profiling build keeps the 120 MHz system clock, ADC divider 99, 2048-word
blocks, 480 kword/s input, 15 ksample/s audio, PWM wrap 499, queues and diagnostic
counters unchanged. It adds timings without changing the DSP calculations.

1. Flash the rebuilt normal `firmware/adc_dsp_suite.uf2` and start from a fresh boot.
   Retain the same input waveform/bias and settings for comparison.
2. Leave BASIC selected. On P1, record every health counter at the start and
   end of a measured 60-second interval. Keep both photos.
3. Short-press Button 1 to reach P4. Photograph `IN`, `DSP`, `STAT`, `FFT` and
   `OUT`. Each pair is latest/maximum elapsed microseconds since boot.
4. `IN` measures input statistics; `DSP` is the BASIC or receiver path; `STAT`
   is audio statistics; `FFT` is spectrum preparation; `OUT` is `stream_audio()`.
   BASIC has no FFT work, so its FFT readings are zero after a fresh boot.
5. Optional USB `status` includes the same stage timings plus `PROC`, the full
   `process_samples()` call. `us` remains the combined processing/output time.
   Timings include interrupt service and the small instrumentation overhead.
6. Use the measured dominant stage to select an optimization. A successful
   rebuild alone is not a timing fix. Each receiver mode requires its own fresh
   boot, preset selection and timed observation before demodulation results count
   as evidence of sustained processing. Maxima from separate stages need not have
   occurred in the same block.

### Profiling measurements on the connected Pico, 13 October 2026

With the user's approval, the profiling firmware was loaded through the Pico's
BOOTSEL drive and booted. The new `TIMING` status line confirmed that the added
instrumentation was running. The prior UF2 is backed up as described above.
The source is `81dbbd9` plus the local stage-timing changes; profiling UF2 SHA256
is `A30EE264C65BEEA3C5E9FBD52BB876DAB32D9DEC83E199090DA55F88E13C03E7`.

The BASIC preset and input configuration were left unchanged. The host requested
status every 10 seconds over a 60.093-second interval after startup; the firmware
publishes snapshots every 32 blocks, so these are snapshot counter differences,
not cycle-exact acquisition-rate measurements.

| Counter | Start | End | Increase |
| --- | --- | --- | --- |
| B | 3200 | 8032 | 4832 |
| ADC dropped blocks | 6070 | 15251 | 9181 |
| Audio underruns | 6067 | 15247 | 9180 |
| LATE | 3200 | 8032 | 4832 |
| ADC FIFO overflows | 0 | 0 | 0 |
| Audio dropped blocks | 0 | 0 | 0 |
| Late DMA interrupts | 0 | 0 | 0 |
| Near-rail samples | 0 | 0 | 0 |

| Stage | Final block (us) | Maximum since boot (us) |
| --- | --- | --- |
| IN: ADC statistics | 751 | 866 |
| DSP: BASIC processing | 11421 | 11862 |
| STAT: audio statistics | 33 | 89 |
| FFT: spectrum preparation | 0 | 0 |
| PROC: entire process_samples call | 12207 | 12598 |
| OUT: stream_audio call | 187 | 285 |
| Overall worker processing/output | 12394 | 12809 |

Result: **FAIL** for sustained BASIC operation. Every processed block still
missed the deadline, and both ADC loss and audio starvation continued. In the
final snapshot, BASIC DSP accounts for approximately 92% of combined processing
time. This localizes the main cost to the BASIC path, but does not isolate the
square root from its DC tracking and other floating-point work. DSP calculations
were intentionally preserved during this measurement; optimization and a fresh
hardware retest remain to be done. Receiver-mode timing has not been tested here.

Raw evidence: [before profiling](tests/results/basic-before-profile-2026-10-13.txt)
and [60-second profiling log](tests/results/basic-profile-2026-10-13.txt).

### BASIC optimization and completed hardware run, 13 October 2026

**BASIC timing and streaming acceptance passed.** The final run lasted
60.0190985 seconds after settling. B advanced from 53920 to 68000, or
14080 blocks / 234.591994 blocks/s. The nominal rate is 234.375 blocks/s;
the difference is within the 32-block status publication granularity.
Maximum combined processing/output time since boot was 3730 us, leaving
536 us (12.6%) below the unchanged 4266 us deadline threshold.
All loss, overflow, late-IRQ, deadline and near-rail counters were zero at
both endpoints, and no counters were reset during the interval.

| Measurement | Instrumented baseline | Final optimized build |
| --- | --- | --- |
| Host interval | 60.093 s | 60.0190985 s |
| Processed blocks increase | 4832 | 14080 |
| Processed blocks/s | about 80.4 | 234.591994 |
| Deadline misses increase | 4832 | 0 |
| ADC drops increase | 9181 | 0 |
| Audio underruns increase | 9180 | 0 |
| FIFO / audio drops / late IRQ / near rails increases | all 0 | all 0 |
| Maximum total processing time since boot | 12809 us | 3730 us |
| Final sampled IN / DSP / STAT / OUT | 751 / 11421 / 33 / 187 us | 730 / 1988 / 36 / 204 us |
| Final sampled total | 12394 us | 2960 us |

The collector cycled OLED pages every ten seconds without changing DSP settings.
Settings remained BASIC IQ, tune 0, BW 2, manual gain 16, volume 25%, revision 0.
The user also browsed the OLED menu and confirmed responsive encoder/buttons.
The user reported both AFG channels as continuous 1 kHz sine, 300 mVpp setting,
High-Z indication, modulation off, offset readbacks 2.5 V. With the existing
470-ohm/1-kohm dividers and mixer disconnected, the scope readings reported
at GP27/GP26 were 1.653/1.652 V mean and 240 mVpp on both. These electrical
readings are user observations; the automated log contains the ADC statistics.
Flashing used USB resets with Pico power connected throughout. Generator
off/on confirmations were retained in the session; no physical power cycle was
performed for the final run. Startup is recorded separately in the linked log.

#### What changed and why

An isolated synthetic Pico benchmark measured original floating-point DC
tracking at 7355.1 us mean and magnitude calculation at 3244.6 us mean per
1024 pairs. The original complete kernel took 10805.5 us. This confirmed that
DC tracking was the larger cost; replacing square root alone was insufficient.
The initial compatible fixed-point DC candidate took 1439.8 us and lookup
magnitude 1138.4 us. These isolated measurements were made without live ADC/PWM
DMA and are evidence for selecting the approach, not the final streaming pass.
The benchmark used its own source snapshot and UF2; it is not present in the
normal streaming entry point.

`basic_dsp.*` implements the fast BASIC kernel. DC states are Q16 with integer
emulation of binary32 subtraction/addition rounding in the normal biased range.
The input DC coefficient remains 1/1024. Each of all 1024 I/Q pairs still gets
a magnitude before the same 16-pair averaging. A 257-entry linearly interpolated
table computes `max(abs(I),abs(Q))*sqrt(1+(min/max)^2)` in fixed point, rounded
to Q8. Audio DC state uses Q24 with the same 1/256 coefficient. Gain factors,
truncation toward zero and +/-32767 limiting are preserved. Split 32-bit sums
and products replace expensive 64-bit work in the pair loop; the kernel and
table execute/read from RAM. The 64-bit audio state is updated only 64 times
per block. Receiver algorithms, acquisition, DMA, queue capacities, output
interpolation, clocks, health counters and the inclusive timing window are unchanged.
No global fast-math flags were added.

Before:

```cpp
basic_i += (raw[pos] - basic_i) / 1024;
basic_q += (raw[pos + 1] - basic_q) / 1024;
sum += std::sqrt(i * i + q * q);
```

After (inside the same 16-pair group):

```cpp
update_dc(raw[pos], i_dc);
update_dc(raw[pos + 1], q_dc);
sum_q8 += magnitude_q8(in_i - i_dc, in_q - q_dc);
```

The numerical implementation changes: magnitude is a bounded approximation,
and fixed-point rounding differs slightly from float. The filter coefficients
and order of magnitude/averaging/audio-DC operations do not change.

#### Numerical comparison

The frozen original float kernel and both fixed-point candidates processed
identical raw blocks and initial state, at gains 1, 16 and 1024. Each of 14
scenarios ran 1024 blocks (65536 audio/magnitude groups); the last 256 blocks
were separately summarized as settled. Cases include quiet midscale, biased
quiet DC, noise floor, very small signals, I-only, Q-only, equal phase,
quadrature, unequal amplitudes, bias steps, amplitude steps, near rails,
limiting and near-zero DC. This is host software comparison, distinct from the
physical streaming test. Host GCC 14.3.0 used `-O3 -ffp-contract=off` to retain
the original sequence of binary32 operations.

Predeclared acceptance was group-magnitude absolute error below one ADC count.
The standalone lookup bound was 0.25 ADC counts versus exact magnitude of
Q16-centered inputs. Q8 input rounding, ratio normalization/rounding, table
linear-interpolation error (second derivative <=1), Q16 coefficient rounding
and output rounding give a conservative combined bound below 0.25. A million
random centered pairs plus rail/axis corners measured maximum error 0.123811.
A separate million full-range ADC updates reproduced the float DC state exactly
in the normal biased range. Below 128 ADC counts, Q16 adds a small DC tracking
quantization; quiet-low-DC and near-rail cases are included in the comparison.

Across all 14 scenarios, group-magnitude errors were maximum 0.013183594,
mean signed +0.000499724 and RMS 0.003319921 ADC-count-equivalent units.

| Gain | PCM maximum absolute error | PCM mean signed error | PCM RMS error |
| --- | --- | --- | --- |
| 1 | 1 | approximately 0 | 0.005323 |
| 16 (default) | 1 | +0.001039 | 0.104477 |
| 1024 | 8 | -0.140365 | 1.058920 |

For settled signals with reference AC RMS >=16 PCM, maximum AC RMS amplitude
differences were 0%, 0.008390% and 0.247914% at gains 1, 16 and 1024 respectively.
Below that reference level the report uses absolute errors rather than unstable
relative percentages. All per-scenario results, including transient/settled
signed errors and limiting counts, are in the CSV. One near-rail sample at
gain 1024 reached saturation in the reference and not the candidate (1073 vs
1072 clipped samples); there were no settled clipping disagreements. This small
threshold-crossing difference is documented rather than claiming bit-identical audio.

#### Reproduction and saved evidence

The optimization, profiling source and recorded test results are published on
`testing/dsp-suite`, based on commit `81dbbd9` plus the preserved profiling
changes. Baseline and tested source archives, local patches, per-file SHA256,
CMake caches, Ninja files, ELF and UF2 remain on the test PC under the ignored
`firmware/evidence/` directory; they are not GitHub downloads. The original
instrumented source/firmware are in
`baseline-profile-81dbbd9`; the final tested source/firmware are in
`optimized-32bit-final`, with a final documentation snapshot added separately.
Earlier failed startup checks are retained: the first candidate reached
4728 us / 37 LATE; the RAM candidate still using 64-bit hot-loop operations
reached 4547 us / 2 LATE. Their near-rail/startup readings are not hidden or
included in the successful final run. The intermediate operation benchmark and
failed checks have their own hashes/metadata.

Build uses SDK 2.1.0, Arm GCC 13.3.1, Release `-g -O3 -DNDEBUG -fwrapv`,
`BENCH_SELF_TEST=OFF`, `BENCH_OPERATION_BENCHMARK=OFF`. The normal build UF2
and refreshed `firmware/adc_dsp_suite.uf2` have matching SHA256:

```text
Baseline profiling UF2: A30EE264C65BEEA3C5E9FBD52BB876DAB32D9DEC83E199090DA55F88E13C03E7
Final optimized UF2:    43F9DAB0EDDCAF592257C37771D3721FC90F6B6EB86420D681081113146F67C1
```

- [All tested builds and hashes](tests/results/basic-tested-builds.json)
- [Operation benchmark log](tests/results/basic-operation-benchmark-2026-10-13.txt)
- [Numerical per-scenario CSV](tests/results/basic-numerical-comparison.csv)
- [Numerical aggregate/error definitions](tests/results/basic-numerical-summary.json)
- [Final startup snapshot](tests/results/startup-basic-optimized-streaming-2026-10-13.txt)
- [Final 60-second raw log](tests/results/basic-optimized-streaming-2026-10-13.txt)
- [Final counters, timing and user observations](tests/results/basic-optimized-streaming-2026-10-13.json)

Reproduce software comparison and normal build:

```sh
g++ -std=c++17 -O3 -ffp-contract=off -Wall -Wextra -Werror -I. tests/basic_numerical_test.cpp basic_dsp.cpp -o build/basic_numerical_test.exe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBENCH_SELF_TEST=OFF -DBENCH_OPERATION_BENCHMARK=OFF
cmake --build build
```

Run `build/basic_numerical_test.exe` to emit CSV; `tests/measure_basic.py` uses
optional pyserial 3.5 for the live log. Review wiring, flash the normal UF2,
hold settings fixed and recheck scope voltages before another live run.
This pass establishes sustained BASIC operation under the recorded bench
conditions. Receiver-mode timing and demodulation performance remain untested.

## 3. Quadrature and sideband discrimination

1. Both AFG channels: continuous 1 kHz sine, 200 mVpp, +1.65 V bias.
2. Align their phases, then set 90 degrees separation. Verify equal frequency,
   equal amplitude and a quarter-period (250 us) separation on the scope.
3. In `preset 0`, MAG should be more constant than with same-phase inputs.
   ADC sampling is interleaved, not simultaneous, so perfection is not expected.
4. Send `preset 6` (USB), then `gain 0`, keeping `agc 4`.
   Measure audio amplitude, then reverse Q phase from +90 to -90 degrees.
   One orientation should pass a 1 kHz tone much more strongly.
5. Send `preset 7`, `gain 0` (LSB). The strong/weak orientation should reverse.
   `swap 1` also reverses spectral orientation in IQ input mode.
6. Calculate rejection = 20 log10(strong RMS / weak RMS) with the same
   generator amplitude, manual gain, output volume and scope bandwidth.
   First target: >15 dB; report the measured value and noise-floor limit.

Measure audio AFTER your board's PWM reconstruction filter, with a
high-impedance scope probe. GP16 itself is 240 kHz PWM, not a sine.
Do not drive a speaker/headphone load straight from a GPIO. Use the board's
audio filter/amplifier and low initial volume.

## 4. Single-channel AM (AFG CH1 Mod)

1. CH1 -> GP27; CH2 is not a modulated Q source. Set sine carrier 8 kHz,
   200 mVpp and +1.65 V offset, then verify actual pin limits.
2. On CH1 select Mod -> AM, internal sine, modulation 1 kHz, depth 30%.
3. Send `preset 2`. This selects real IF on I, tune +8000 Hz, AM,
   normal bandwidth, manual gain 16, volume 25%, squelch off.
4. Expect a ~1 kHz recovered audio waveform. Disable modulation while leaving
   the carrier on: MAG can remain high, but the 1 kHz audio should collapse.
5. Compare 20%, 40%, 60% depth at unchanged carrier amplitude. Under manual gain
   and below clipping/compression, audio amplitude should rise roughly with depth.
   Reduce gain if the waveform flattens or AUD peak approaches 16383/32767.
6. Send `preset 3` to try synchronous AM. Allow several seconds for PLL settling.
   Record tone recovery and response to small carrier mistuning. This is not a
   guarantee that the PLL will lock at every offset/signal level.

First target: tone frequency within 2% of modulation; modulation-off tone at
least 10 dB below modulation-on. Compare the tone component, not broadband RMS.
To inject on GP26 instead, send `input q`; no rewiring convention changes.

## 5. FM and FSK

FM:

1. CH1 carrier 8 kHz, same safe amplitude/bias.
2. Mod -> FM, internal sine 500 Hz, deviation 500 Hz.
3. Send `preset 4`. Expect recovered ~500 Hz audio.
4. Compare deviation 250 and 500 Hz with `agc 4`, `gain 0`.
   Audio should approximately double before filter/limiter effects.
5. Vary carrier amplitude moderately without clipping; recovered FM amplitude
   should be less sensitive to carrier amplitude than AM.

FSK discriminator test:

1. CH1 Mod -> FSK; carrier 8 kHz, hop 8.5 kHz, internal rate 100 Hz.
2. Send `preset 5`: FM discriminator tuned midway at 8250 Hz.
3. Expect alternating positive/negative audio levels, rounded by filters.
   A coupling capacitor in the audio path may cause droop. Compare the OLED
   audio view and the filtered analog output.
4. Stop hopping: the output becomes a steady discriminator offset; an
   AC-coupled audio output may settle back toward zero.

This is frequency discrimination, NOT bit recovery, packet decoding or BER.

## 6. PM stimulus (no dedicated PM decoder)

Use CH1 PM with 8 kHz carrier, internal 500 Hz sine and 30 degree deviation,
then `preset 4` (FM demodulation).
For sinusoidal PM, peak frequency deviation = phase deviation in radians
times modulation frequency: here about 262 Hz. Expect 500 Hz output from the
phase derivative. Doubling PM modulation frequency at fixed phase deviation
approximately doubles discriminator amplitude while within the passband.
This verifies the FM discriminator's response to PM, not a PM data decoder.

## 7. Filters and additional processing

Always start from a preset, change ONE setting, use manual gain for transfer
measurements, and save before/after traces with settings and health counters.

| Feature | Procedure | Evidence to record |
| --- | --- | --- |
| FFT bandwidth | USB quadrature test: sweep both continuous inputs together from 500 Hz to 5 kHz; compare `bw 0` and `bw 4` | Normalized response vs frequency; wider setting passes a wider band |
| Digital tuning | In IF-AM, change generator carrier by +500 Hz, then change `tune 8000` to `tune 8500` | Restored centering/passband; for FM, reduced discriminator DC offset |
| Manual gain | Stable tone, `gain 0`, `gain 1`, `gain 2` | Rough doubling per step until compression; avoid the soft limiter |
| AGC | `gain 8` sets gain ceiling; compare `agc 4` vs `agc 0..3` while stepping signal amplitude | Audio amplitude less variable with AGC; attack/hang/recovery traces |
| Squelch | `squelch 6`, `hang 3`; sweep input level downward; repeat with `squelch -1` | Mute threshold and ~500 ms hang; thresholds are receiver-relative, not calibrated dBm |
| CW | CH1 8 kHz unmodulated sine, `preset 8`; change `cw 700` to `cw 1000` | Audible/output beat changes; phase-step quantization makes 700 nominal about 696 Hz |
| IQ correction | Stable USB/LSB tone; introduce 20% amplitude mismatch or 10 degree phase error; compare `iqcorr 0` / `iqcorr 1` after settling | Desired/image amplitudes; any measured rejection improvement; valid ONLY in IQ mode |
| Auto notch | USB desired tone; `notch 1`, wait >1 s; then `notch 0` | Stable tone attenuated; do not leave on for ordinary single-tone demod tests |
| Noise reduction | Desired tone PLUS controlled noise, `nr 0/1`; `threshold 0` adaptive or 1..6 fixed; `smooth 10` | Tone amplitude AND noise power in a fixed bandwidth; SNR/distortion comparison |
| Impulse blanker | Desired tone PLUS brief repeatable impulses; compare `blanker 0` and 1..6 | Reduced transient peak/energy without excessive wanted-signal loss |
| De-emphasis | IF-FM, fixed deviation, vary modulation 500 Hz to 2 kHz; compare `deemph 0/1/2` | Increasing high-frequency attenuation; effects 1/2 correspond to ~50/75 us filters at 15 kHz |
| Bass/treble | Fixed-level recovered tone, sweep audio frequency; compare `bass 0..4`, `treble 0..4` | Relative low-/high-frequency gain, clipping check |
| Spectrum/audio views | `page 2`, `page 3`; vary source frequency/modulation | FFT peak motion and recovered waveform changes; qualitative displays, not calibrated analyzers |

The filter bin spacing before the final decimation is 30000/256 = 117.1875 Hz.
USB/LSB normal bandwidth accepts bins 3..22 (~352..2578 Hz); this is bin
selection, not an exact brick-wall or measured -3 dB specification.

Controlled noise/impulse/multi-tone tests require an arbitrary waveform or
proper passive summing/biasing network. Do NOT directly join the two AFG BNC
outputs. Two independent sine channels alone do not establish noise reduction,
impulse rejection, AGC dynamics under fading, or receiver sensitivity.
The AFG1022's CH2 is continuous-only; it cannot supply a second independently
modulated quadrature signal. Real-IF presets address that limitation for the
single-channel AM/FM/PM/FSK tests.

## 8. Software-only self-test

Flash `firmware/dsp_software_selftest.uf2` and open USB serial. It waits for a
connected terminal, then feeds synthetic raw ADC words into the actual DSP.
It checks command validation, silence, AM, AM-sync, FM, I/Q sideband rejection,
Q-input routing, CW, and silent/effects corner cases. It can take several
minutes on an RP2040. Save the PASS/FAIL lines and final failure count.
It does not test ADC hardware, live DMA, output filtering or real-time speed.
These tests were compiled, not executed on a board during development.
Flash the normal firmware again before using the AFG.

## 9. Report contents and limits

Include firmware commit/hash, wiring diagram, AFG settings, measured pin
voltages, oscilloscope configuration, exact serial commands, before/after
traces, health-counter start/end values, measured errors and pass criteria.
Separate software-only, injected-ADC and full-radio results. No RF front-end,
LO/mixer, antenna reception, calibrated RF sensitivity, PSK decoder or bit-error
rate is established by these tests.

Instrument references:

- [Tektronix AFG1022 Quick Start manual](https://download.tek.com/manual/AFG1022-Quick-Start-User-Manual-EN.pdf):
  AM/FM/PM/FSK operation and voltage/load settings.
- [Tektronix AFG1022 channel limitations](https://www.tek.com/tw/node/468361):
  modulation, sweep and burst on CH1; CH2 continuous-only.
