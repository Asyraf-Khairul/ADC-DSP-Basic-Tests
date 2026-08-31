# ADC/DSP Standalone Test

This project is a diagnostic for the Pi Pico receive back-end. It proves the
ADC, DMA, basic I/Q DSP, PWM audio output, and SSD1306 status display without
starting the full PicoRX application.

```
GP27 / ADC1 (I) + GP26 / ADC0 (Q)
    -> ADC FIFO and DMA
    -> I/Q DC removal
    -> AM magnitude: sqrt(I^2 + Q^2)
    -> audio DC removal and decimation
    -> PWM DMA on GP16
```

## Pin map

| Function | Pico pin |
| --- | --- |
| I input | GP27 / ADC1 |
| Q input | GP26 / ADC0 |
| PWM audio | GP16 |
| OLED SDA | GP18 / I2C1 |
| OLED SCL | GP19 / I2C1 |

The default OLED address is `0x3C`. Change `OLED_I2C_ADDRESS` in `main.cpp`
to `0x3D` for an OLED with the alternate address.

## What the firmware does

`SAMPLE_RATE` is the aggregate ADC FIFO rate. With its default value of
500000 samples/s, I and Q alternate in the FIFO, so each channel is sampled
at 250000 samples/s. A DMA block holds 1000 ADC values, which is 500 I/Q
pairs and takes about 2 ms to capture.

For each pair, the DSP tracks and removes its DC bias, then calculates the
magnitude. The magnitude DC component is removed to make an audio signal. The
DSP decimates by 25, producing 20 audio samples per DMA block or 10000 audio
samples/s. PWM DMA repeats each audio sample 16 times, producing a 160 kHz
PWM carrier on GP16.

The copied NCO sources are retained from PicoRX but are not called in this
diagnostic. `LO_FREQUENCY` is only a reference configuration value. This
firmware does not generate an RF local oscillator.

## Build and flash

From this project directory in a Pico SDK-enabled shell:

```powershell
cmake -G Ninja -S . -B build -DPICO_BOARD=pico
cmake --build build
```

Flash `build\adc_dsp_standalone.uf2` by holding `BOOTSEL` while connecting
USB, then copying the UF2 to the mounted `RPI-RP2` drive.

No serial monitor is required.

## OLED status display

The display updates about five times per second. It should first show
`ADC DSP TEST`, then `ADC DSP RUN`.

| OLED item | Meaning |
| --- | --- |
| `RAW` | Direct 12-bit ADC readings. Valid range is 0 to 4095. |
| `I/Q` | Samples after DC-bias removal. They should vary around zero. |
| `MAG` | I/Q magnitude, `sqrt(I^2 + Q^2)`. |
| `AUD` | Magnitude after its DC part is removed: the recovered audio. |
| `PWM` | PWM duty value sent to GP16. |
| `B` | DMA blocks processed. It must steadily increase. |
| Bottom bar | Magnitude indicator, not a calibrated dB or S-meter reading. |

`B` is not signal strength. One block equals 1000 raw ADC samples, 500 I/Q
pairs, and 20 output audio samples. For example, `B1000` means the firmware
has processed one million raw ADC samples.

With no intentional signal, a small fluctuating magnitude bar is normal ADC
noise. `RAW` near 0 or 4095 is a warning that the ADC input is at a rail.

## ADC safety and bias

The ADC range is strictly 0 V to 3.3 V. Never apply a negative voltage or a
signal above 3.3 V to GP26 or GP27.

The existing input circuit has an independent 1 kOhm pull-down before the 470
Ohm series resistor for each ADC input. Those pull-downs hold each input near
0 V when no source is connected; they do not create a 1.65 V midpoint.

For a direct test without adding bias components, use the TG320 main output
in DC-coupled mode. Its DC offset must drive the RX input node to 1.65 V.
Connect generator ground to Pico GND. Do not use the TG320 TTL auxiliary
output.

Before continuing, use a multimeter to confirm both GPIO pins are near 1.65 V
when driven. The ADC should then report `RAW` near 2048. The DSP deliberately
removes this DC level, so `I/Q` returning close to zero after settling is
correct.

If a DC-offset-capable source is unavailable, add a 1 kOhm pull-up from 3V3
to each separate RX input node, before its 470 Ohm resistor. Together with the
existing 1 kOhm pull-down, it forms a 1.65 V divider.

## Direct two-TG320 test

Two TG320s can prove both ADC paths, DMA, DSP operation, OLED feedback, and
GP16 PWM. They cannot provide a stable, phase-locked quadrature source.

### 1. Check firmware operation

1. Disconnect both TG320 outputs.
2. Power the Pico and OLED.
3. Confirm `ADC DSP RUN` is shown and `B` increases.
4. Expect `RAW` near zero because the 1 kOhm pull-downs are active.

### 2. Test I only

1. Turn TG320 A output off, set it to sinewave, 10 kHz, minimum amplitude,
   and 0 V offset.
2. Connect its main output centre conductor to `RX_I`, before the 470 Ohm
   resistor. Connect its shield to Pico GND.
3. Set the oscilloscope to DC coupling and 1 Mohm input. Probe `RX_I` and
   connect the probe ground to Pico GND.
4. Turn TG320 A on and increase positive DC offset until `RX_I` and GP27 are
   about 1.65 V.
5. Increase amplitude until the scope reads about 300 mVpp at `RX_I`.

The scope waveform must stay between about 1.50 V and 1.80 V. The OLED `RAW`
value should vary around 2048, roughly 1860 to 2235 for a real 300 mVpp input.

### 3. Test Q only

Repeat the I test with TG320 B connected to `RX_Q`, before its 470 Ohm
resistor. Verify GP26 sits around 1.65 V and its `RAW` value changes around
2048.

### 4. Drive I and Q together

1. Keep TG320 A on RX_I and TG320 B on RX_Q.
2. Set both for sinewave, 10 kHz, approximately 300 mVpp at the RX nodes, and
   1.65 V DC offset.
3. Probe RX_I with scope channel 1 and RX_Q with channel 2. Both scope ground
   clips connect to Pico GND.
4. Trigger from channel 1 and tune the TG320 B frequency as close as possible
   to TG320 A.

A true quadrature signal has Q shifted by 90 degrees. At 10 kHz that is a
25 microsecond shift. The independent TG320s have no shared reference or
phase-lock connection, so their relative phase drifts. This is acceptable for
testing two ADC channels, but it is not a precision I/Q or audio-demodulation
test.

Expected OLED behavior during this test:

```
RAW: both channels vary around 2048
I/Q: both values change sign
MAG: clearly higher than the no-signal noise level
bar: longer than the no-signal bar
```

## GP16 PWM check

Probe GP16 with the oscilloscope. Start around 2 us/div. The output should be
PWM close to 160 kHz, with duty cycle controlled by `AUD`.

For an audible output, filter GP16 before an amplified speaker:

```
GP16 -> 1 kOhm resistor -> filtered output
filtered output -> 10 nF capacitor -> GND
```

Connect an oscilloscope or amplified speaker to the filtered output. Do not
connect a passive speaker directly to GP16.

Two steady TG320 sinewaves do not normally create a sustained audible tone:
a constant I/Q magnitude has no AM envelope to demodulate. Phase drift between
the two generators can cause slow variations, which is expected.

## 3 MHz and full AM/audio test

Do not connect a 3 MHz generator directly to GP26 or GP27. Each ADC channel
runs at 250 ksample/s, so a direct 3 MHz signal aliases and does not provide a
valid baseband test.

Use 3 MHz only at the RF input of a suitable analogue PicoRX front-end or an
external mixer that converts RF into safe, low-frequency, DC-biased I/Q
signals for GP27 and GP26.

The TG320 is a single-channel generator with DC offset, but it does not have
the TG330's internal AM function or a shared I/Q reference. A full end-to-end
speaker test needs a coherent amplitude-modulated I/Q source, such as:

* The PicoRX analogue I/Q front-end plus an AM-modulated RF source.
* A phase-locked dual-channel generator with AM capability.
* An external I/Q modulator or 90-degree phase-shift network.

For a successful full test, the magnitude bar rises and falls with the AM
envelope, `AUD` changes at the modulation frequency, GP16 PWM duty changes,
and the filtered amplified speaker output reproduces the modulation tone.
