# FM physical demodulation evidence — 15 October 2026

Direct ADC injection for Section 4.5.6. Same fm-optimized-v2 firmware; no rebuild, flash, DSP-setting write or cumulative-counter reset was performed for these physical tests. New evidence remains local and uncommitted.

## Results

| Check | Observation | Assessment |
| --- | --- | --- |
| Frequency | Clear FFT peak reported at 500 Hz | Meets proposed 490-510 Hz bench target as reported; accuracy not independently checked |
| FM off | 500 Hz peak disappeared into approximately -83 dBm background; initial on peak -50 dBm | Nominal 33 dB peak/background separation supports 10 dB target; residual tone unresolved |
| Deviation | 250 Hz: -55 dBm; 500 Hz: approximately -48 dBm; both repeated | +7 dB, about 2.24x amplitude vs ideal 2x/+6.02 dB; no precise proportionality claim |
| Timing | 62.066821 s, 14547 blocks, 234.376431 blocks/s; interval maximum 3937 us | PASS observed FM real-I timing interval; deadline 4266 us |
| Output clipping / distortion | User reports no clipping; method/capture not supplied; distortion not quantified | Manual observation only |
| Speaker audibility / saved captures | User heard no tone at gain x1, volume25; FM-on/off photos not taken | Audibility not demonstrated; later captures must be labeled as later captures |

## Setup and provenance

{
  "generator_CH1": {
    "connection": "Existing 470 ohm / 1 kohm divider to GP27",
    "carrier_Hz": 8000,
    "amplitude_setting_Vpp": 0.3,
    "offset_setting_V": 2.5,
    "load_indication": "High-Z",
    "modulation": "Internal sine FM, 500 Hz",
    "final_deviation_Hz": 500,
    "provenance": "Instructed intended setup; user tentatively confirmed everything set. Detailed instrument readbacks not independently supplied."
  },
  "generator_CH2": {
    "connection": "Existing divider to GP26",
    "waveform": "DC instructed",
    "offset_setting_V": "initial +2.5 V instructed; actual readback not supplied"
  },
  "mixer": "Disconnected instructed; not independently inspected",
  "speaker": "Remains connected per established setup; actual impedance/load not measured",
  "scope_CH1": "R1 100 ohm / C2 470 nF junction to circuit ground, after GP16 coupling capacitor; instructed probe point, not independently physically inspected",
  "GP27_scope_user_readings_V": {
    "mean": 1.65,
    "minimum": 1.54,
    "maximum": 1.78,
    "derived_Vpp": 0.24
  },
  "GP26_scope_user_DC_V": 1.65,
  "input_limit_result": "Reported GP27 extrema within 0 to 3.3 V; GP26 near midscale. Does not exclude all transients.",
  "scope_FFT": "CH1; 10 ms/div acquisition and 0-2 kHz FFT span instructed; actual full readbacks, window and amplitude reference not recorded. Prior photo was waveform mode at 1 ms/div, not the later FFT."
}

Receiver settings verified over serial:

```json
{
  "mode": "FM",
  "input": "IF-I",
  "tune": 8000,
  "bw": 2,
  "agc": 4,
  "gain_log2": 0,
  "volume": 25,
  "sq": -1,
  "hang": 3,
  "swap": 0,
  "iqcorr": 0,
  "deemph": 0,
  "bass": 0,
  "treble": 0,
  "blanker": 0,
  "notch": 0,
  "nr": 0,
  "smooth": 10,
  "threshold": 1,
  "cw": 700
}
```

Runtime identity matches the prepared artifact (self-report; no independent flash readback): {"BENCH_BUILD_LABEL": "fm-optimized-v2", "BENCH_SOURCE_SHA256": "c9424dd398f20e91897c62007188ab5e2d88b0e70e1e3567bc74c89aa570dae6", "BENCH_BASE_COMMIT": "bfa22b8c150ee87c1808418a08cb066160e3834d"}. UF2 SHA256 77b8f17f2facca31221a7e537a34cc139ef9cc8a222d3e657b803949d3561e0e. Local source commit 63f6610e8ab9e894b30543816e5c8033ace43c4a; binary embeds parent bfa22b8 plus source fingerprint because it was built before committing.

The photo showed dense output ripple, 240.4 kHz, 63.2 mVpp, 1 ms/div and Stopped acquisition. Those readings are not selective 500 Hz audio measurements. Later FFT acquisition at 10 ms/div and 0-2 kHz span was instructed; full instrument readbacks/window were not supplied. The image was viewed in chat, not archived locally.

## Measurement sequence and uncertainty

| Condition | Deviation Hz | Level dBm | Note |
| --- | ---: | ---: | --- |
| FM on, initial | 500 | -50 | Clear peak; later user said an earlier reading might have been misread, unspecified which. |
| FM off, carrier retained as instructed | off | -83 | Peak disappeared; level is background at 500 Hz, not resolved residual tone. |
| FM on, lower deviation | 250 | -55 | Measured at 500 Hz FFT location; repeated at same reported level. |
| FM on, higher deviation | 500 | -48 | Approximate reading, final repeat confirmed yes to approximately -48 dBm. |

The operator flagged an earlier reading as potentially misread without specifying which. All observations remain preserved. Final -55/-48 dBm levels repeated, but resolution and uncertainty were not quantified. The 7 dB difference implies a voltage-amplitude ratio 10^(7/20) = 2.239 assuming a common FFT reference and unchanged measurement configuration; this is about 11.9% above ideal doubling. It does not prove absence of compression or clipping. Do not convert these dBm readings to speaker power.

## Final timing interval

Host record-to-record duration 62.0643484 s; device window 62.066821 s. At least 5.1 seconds settling preceded capture. Receiver revision 1 remained unchanged. All-block capture was started/stopped; cumulative counters were preserved.

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| B | 1005734 | 1020281 | 14547 |
| deadline | 0 | 0 | 0 |
| clips | 3347771 | 3347771 | 0 |
| ADCdrop | 0 | 0 | 0 |
| FIFO | 0 | 0 | 0 |
| AUDunder | 0 | 0 | 0 |
| AUDdrop | 0 | 0 | 0 |
| IRQlate | 0 | 0 | 0 |

Cumulative maximum at interval start/end: 4184/4184 us. Interval all-block maximum: 3937 us; largest serial-sampled latest total: 3393 us. Serial publication and USB latency mean snapshots cannot capture every block; interval counters do. Headroom: 329 us.

| Class | Count | Late | Mean total us | Maximum total us |
| --- | ---: | ---: | ---: | ---: |
| ORDINARY | 14320 | 0 | 3362.793 | 3937 |
| SPECTRUM | 227 | 0 | 3766.581 | 3862 |

Stage means / maxima (us):

| Stage | Ordinary | Spectrum |
| --- | ---: | ---: |
| TOTAL | 3362.793 / 3937 | 3766.581 / 3862 |
| IN | 553.769 / 576 | 554.392 / 564 |
| DSP | 2578.076 / 2982 | 2577.300 / 2619 |
| STAT | 34.603 / 84 | 34.524 / 43 |
| FFT | 0.000 / 0 | 402.216 / 452 |
| OUT | 195.652 / 415 | 196.247 / 213 |
| CICMIX | 498.788 / 504 | 498.784 / 503 |
| FILTER | 1541.303 / 1750 | 1540.855 / 1570 |
| DEMOD | 357.215 / 459 | 357.683 / 374 |
| CAPTURE | 4.372 / 20 | 3.780 / 19 |
| FFTFWD | 861.726 / 921 | 861.634 / 878 |
| FFTBINS | 254.560 / 404 | 254.282 / 271 |
| FFTINV | 356.494 / 385 | 356.449 / 359 |

Timing includes sample processing plus audio preparation and interrupt service. DSP/filter stages are nested; do not sum them or independent maxima. Window bookkeeping remains separately measured, {"max_us": 66, "sum_us": 59229}.

Pre-window near-rail history: {"preflight_near_rail": 58700, "first_FM_settings_check_near_rail": 3128306, "final_timing_window_start_near_rail": 3347771, "increase_since_first_FM_settings_check": 219465, "interpretation": "Counter increased during the setup/physical-test session before the final timed window. No continuous log identifies when or why; do not attribute all to a specific wiring change or silently claim zero near-rail events over the entire session."}. This setup-session increase must be distinguished from the final interval delta; no time-resolved evidence assigns every detection to a particular condition.

## Operator follow-up

{
  "recorded_utc": "2026-10-15T01:51:55.254093+00:00",
  "reply": "i havent took pictures as i iwll do it when the web write up my report sectiion and the output isnt clipped and when are you doing the timing check run i wnt to hear it",
  "audibility": "User reports no audible speaker tone at final measurement settings.",
  "output_waveform_clipping": "User reports output is not clipped; measurement method and a validating output-waveform capture were not supplied.",
  "captures_saved": "FM-on/off FFT photos not yet taken; user intends to collect later for report. Later captures must be labeled with actual settings/date and not presented as contemporaneous evidence for this interval."
}

No new UI responsiveness check in this session. Physical output clipping, noise/distortion and audibility remain separately assessed. This result does not validate an RF front end, mixer or complete antenna-to-speaker receiver.

## Evidence

- tests/results/fm-demod-2026-10-15-preflight01.json
- tests/results/fm-demod-2026-10-15-settings01.json
- tests/results/fm-demod-timing-2026-10-15-run01.json
- Raw timestamped timing log: fm-demod-timing-2026-10-15-run01.txt
- Raw log SHA256: 3912c273297e61710e43b3101aa4925cccb71086c5d9afbce68df2281858d309


Final listening observation: no audible speaker tone reported. The electrical FFT evidence remains valid within its stated limits, but no audible-output pass is claimed. The cause of inaudibility is undiagnosed; the unverified FFT dBm reference cannot establish delivered speaker power. No volume/gain change, rebuild, flash, reset, commit or push was performed in response.
