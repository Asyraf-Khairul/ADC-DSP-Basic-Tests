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
