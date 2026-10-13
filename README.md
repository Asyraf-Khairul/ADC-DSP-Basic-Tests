# Pico ADC/DSP test suite

A standalone bench harness for the DSP in your `SDR-RX` project. This now includes
AM, synchronous AM, FM, USB, LSB, CW, frequency tuning, FFT bandwidth, AGC/manual
gain, squelch, I/Q swap/correction, de-emphasis, bass/treble, impulse blanking,
automatic notch and noise reduction. BASIC remains available for simple
ADC/magnitude checks.

Read [TEST_GUIDE.md](TEST_GUIDE.md) for safe AFG1022 settings, individual test
steps, expected observations and report checkpoints.
For another computer, read [UNI_SETUP.md](UNI_SETUP.md): it includes a
copy-paste Codex prompt to retrieve this branch, set up tools and build both UF2s.
See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for source attribution and
documented differences from the receiver algorithms.

## Start here

1. Build the normal UF2 first if this is a fresh clone (see [UNI_SETUP.md](UNI_SETUP.md)).
   Flash `build/adc_dsp_standalone.uf2`, or its local
   `firmware/adc_dsp_suite.uf2` copy, using BOOTSEL.
2. Verify generator output voltage before connecting it to an ADC.
3. The OLED starts in BASIC. Press Button 2 (GP22) to open Settings.
4. For CH1 AM testing: carrier 8 kHz, safe 200 mVpp/+1.65 V bias, AM 1 kHz/30%,
   connected to GP27; select Preset, turn to `2 IF-AM`, and press Button 2 to apply.
5. Return with Button 1 (GP17), then short-press it to view page 1 for sample loss
   as well as measuring the recovered audio.
   A live OLED or nonzero MAG alone does not prove demodulation.

The UF2 files are build outputs, not evidence of hardware validation.
Do not claim all tests pass until you have run and recorded them on the board.

## Wiring

| Function | GPIO |
| --- | --- |
| I / ADC1 | GP27 |
| Q / ADC0 | GP26 |
| PWM audio (through board filter/amplifier) | GP16 |
| OLED SDA / SCL | GP18 / GP19 |
| Rotary encoder A / B | GP20 / GP21 |
| Button 1: page / back / hold for coarse tune | GP17 to GND |
| Button 2: menu / select / apply | GP22 to GND |

Buttons and encoder contacts use internal pull-ups, active low. Connect the
encoder common terminal to GND; no encoder push switch is required. If using
an encoder module, power its logic from 3.3 V. GPIO names are NOT physical pin
numbers. Pin assignments are in `controls.h`; GP5 is no longer used for a button.
`ENCODER_DIRECTION` is -1 for this encoder so clockwise moves down the menu;
set it to 1 for an encoder wired in the opposite direction.
OLED remains the existing **SH1106 128x64, address 0x3C** configuration.
The files named `ssd1306.*` provide drawing/page transfers; they do not make this
an SSD1306 initialization sequence. Change `display_init(18,19,0x3c)` in the
entry point if the address differs. Missing OLED does not stop DSP execution.

No RF LO is generated on GP0/GP1. This suite directly tests the ADC/DSP back-end.
Do not connect generator outputs against powered mixer/amplifier outputs.

## Encoder and OLED controls

Normal firmware needs no serial monitor for configuration or on-screen status.

| Action | Status pages | Settings menu | Editing a value |
| --- | --- | --- | --- |
| Turn encoder | Tune in receiver modes | Move selection | Adjust draft value |
| Short press Button 1 | Next OLED page (0..3) | Return to status | Cancel the edit |
| Hold Button 1 and turn | Coarse tune | Move selection | Adjust draft value |
| Press Button 2 | Open Settings | Select item | Apply value |

Buttons act on release after debounce. Releasing Button 1 after coarse tuning
does not also change pages. Encoder edges are captured in interrupts, including
while an OLED frame is being transferred; button sampling runs every millisecond.
The decoder expects four valid quadrature transitions per step.

Fine tuning defaults to 100 Hz per step. Coarse tuning uses ten times the fine
step, with a minimum of 1 kHz. Settings > Tune step offers 1, 10, 100 and 1000 Hz.
Tuning is limited to -12000..12000 Hz and does not act in BASIC mode.

The menu includes Preset, Mode, Input, Tune, Tune step, Bandwidth, AGC, Gain,
Volume, Squelch, Hang, IQ swap, IQ corr, Deemph, Bass, Treble, Blanker, Notch,
Noise red, NR smooth, NR thresh and CW tone. The edit screen explains the
selected setting and flags controls that are inactive in BASIC or require IQ.
The values shown in the menu are requested settings. Apply queues them to the
DSP core; `Queued; wait 2s` confirms submission. The status pages show the applied
mode, input, tune, bandwidth and gain. Allow at least two seconds for settling.
If the queue is full, `Busy: press OK again` keeps the draft so it can be retried.
Back cancels an unsubmitted edit. Presets reset all DSP settings, and reboot
restores BASIC defaults; settings are not saved to flash.

Pages: 0 = ADC/audio values, 1 = streaming health, 2 = spectrum, 3 = audio waveform.
USB serial remains optional for full numerical logging and keyboard commands.
The separate synthetic self-test firmware still uses USB serial for its results.

## Presets

Choose Settings > Preset with the encoder, or optionally send `preset N` over USB.
Each preset restores all settings to reproducible defaults; settings are not
saved to flash.

| N | Mode | Input | Tune |
| --- | --- | --- | --- |
| 0 | BASIC envelope diagnostic | I/Q | 0 |
| 1 | Receiver AM | I/Q | 0 |
| 2 | Receiver AM | real IF on I | 8000 Hz |
| 3 | Synchronous AM | real IF on I | 8000 Hz |
| 4 | FM | real IF on I | 8000 Hz |
| 5 | FSK discriminator via FM | real IF on I | 8250 Hz |
| 6 | USB | I/Q | 0 |
| 7 | LSB | I/Q | 0 |
| 8 | CW | real IF on I | 8000 Hz |

Real IF sends the selected input into the receiver I branch and replaces Q
with midscale internally. Frequency shifting/filtering select one spectral
image; I/Q correction and swap are forced off. Both physical pins' raw ADC
statistics remain visible. This is not simultaneous two-channel AM modulation.

## Optional USB commands

Lowercase, one command per line. All value ranges are checked. An `OK queued`
reply means the command was accepted; `status` shows the applied settings
and revision. Allow at least two seconds to settle; PLL/adaptive processing
may need longer. Reboot to clear upstream filter histories between independent
measurements. Select only one feature at a time when first characterizing it.

| Command | Values / meaning |
| --- | --- |
| `mode` | `basic am amsync fm usb lsb cw` |
| `input` | `iq`, `i` (real IF on GP27), `q` (real IF on GP26) |
| `tune` | -12000..12000 Hz; use +8000 for the documented real-IF tests |
| `bw` | 0..4: very narrow through very wide, mode-dependent |
| `agc` | 0 fast, 1 medium, 2 slow, 3 long, 4 manual |
| `gain` | 0..10: gain or AGC ceiling = 2 to this power |
| `volume` | 0..100 percent PWM output scaling; default 25 |
| `squelch` | -1 off, 0..12 receiver-relative threshold steps |
| `hang` | 0..7 = 50,100,200,500,1000,2000,3000,5000 ms |
| `swap`, `iqcorr` | 0/1; IQ input only |
| `deemph` | 0 off, 1 ~50 us, 2 ~75 us |
| `bass`, `treble` | 0 off, 1..4 increasing effect |
| `blanker` | 0 off, 1..6 increasing sensitivity |
| `notch`, `nr` | 0 off, 1 on |
| `smooth` | 1..15 noise estimator smoothing |
| `threshold` | 0 adaptive noise threshold, or 1..6 |
| `cw` | nominal 100..2000 Hz sidetone; integer phase-step quantization |
| `page` | 0 ADC/audio stats, 1 health, 2 spectrum, 3 audio waveform |
| `stream` | 0 off, 1 print status once per second |
| `status`, `help` | current state / command reference |

BASIC uses only gain and volume from the processing controls; it is not the
receiver chain. Receiver modes use the imported DSP. MAG is in internal units,
not calibrated dBm; it is not directly comparable between BASIC and receiver
modes. ADC RMS is **AC-only** over a 4.267 ms block. Audio RMS includes DC and
is measured before output volume. Spectrum is auto-scaled and qualitative;
its center is the digitally tuned frequency (relative span -15..+15 kHz).
Audio view is a short, untriggered 64-sample record and is not an oscilloscope.

## How the code fits together

```text
GP27 / GP26 (alternating, 480 kword/s total)
  -> continuous chained ADC DMA + six-block queue
  -> bench.cpp input selection and diagnostics
  -> receiver_dsp: CIC /16 -> 30 kHz complex
     -> DC removal, IQ correction, digital frequency shift
     -> 256-point overlap FFT filter /2 -> 15 kHz complex
     -> AM / AMSYNC / USB / LSB / FM / CW
     -> audio processing, AGC, squelch
  -> 64 PCM samples per block
  -> output volume and 16x linear interpolation
  -> chained PWM DMA, GP16 at 240 kHz
```

System clock is 120 MHz; ADC clock is 48 MHz with a 100-cycle period.
Each raw block has 2048 words and spans 4.2667 ms. The ADC samples I and Q
2.083 us apart; they are not simultaneous. PWM wrap is 499.
Core 1 owns acquisition, receiver processing and audio queues.
Core 0 owns OLED, USB commands and buttons. Short locked snapshots separate them.
Counters expose overload; continuous operation still needs board verification.

- `main.cpp`: cores, settings queue, front-panel and optional serial dispatch.
- `controls.*`: GP20/21 encoder interrupts and GP17/22 button debounce.
- `front_panel.*`: menu navigation, editing, value labels and tuning steps.
- `settings.cpp`: presets and validated serial settings shared with the menu.
- `stream.*`: ADC/PWM DMA and queue health.
- `bench.*`: presets, validated controls, receiver adapter and block statistics.
- `receiver_dsp/`: copied/adapted receiver algorithms, no dependency on the other folder.
- `display.*`, `ssd1306.*`: existing SH1106 driver plus four bench pages.
- `tests/self_test.cpp`: synthetic DSP-only test entry point.
- Old `adc.*`, `dsp.*`, `pwm.*`, `nco.*`, `clocks*` are retained as legacy
  reference files; CMake does not compile them into this suite.

Host-side control tests (no board required):

```sh
g++ -std=c++17 -Wall -Wextra -Werror -I. tests/front_panel_test.cpp front_panel.cpp settings.cpp -o build/front_panel_test.exe
./build/front_panel_test.exe
```

These check menu navigation, cancel/retry, preset selection, setting limits,
tuning increments, quadrature bounce and button debounce. Physical encoder
direction, detent spacing and operation during DSP load still need board testing.

## Build

Use the installed Pico VS Code extension / Pico SDK 2.1.0, RP2040 `pico` board.
Configure Release, then build `adc_dsp_standalone`:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBENCH_SELF_TEST=OFF
cmake --build build
```

Normal artifact: `build/adc_dsp_standalone.uf2`.
If CMake picks Windows' Python Store alias, pass
`-DPython3_EXECUTABLE=C:/Users/asyra/.pico-sdk/python/3.13.7/python.exe`.
Use the Pico terminal or supply `PICO_SDK_PATH`, `PICO_TOOLCHAIN_PATH` and
`CMAKE_MAKE_PROGRAM` for the installed toolchain/Ninja.

Optional software-only build (separate directory):

```sh
cmake -S . -B build-selftest -G Ninja -DCMAKE_BUILD_TYPE=Release -DBENCH_SELF_TEST=ON
cmake --build build-selftest
```

This firmware waits for a USB serial connection and runs synthetic tests.
It neither captures physical ADC inputs nor drives audio. Restore the normal
firmware afterward. See the guide for what the checks do and do not establish.

## Scope

No PSK/PM data decoder, symbol synchronization, packet decoder, BER tester,
RF front-end, LO or complete radio UI is included. FSK/PM stimuli can exercise
the FM discriminator, but that is not data decoding. Noise/impulse rejection
needs an appropriate mixed stimulus; it cannot be proven with a clean sine alone.
