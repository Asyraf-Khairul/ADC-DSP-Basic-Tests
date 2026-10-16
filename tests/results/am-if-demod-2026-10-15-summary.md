# Section 4.5.5 evidence summary ? 15 October 2026

The test supports AM-dependent output near 1 kHz, an increasing output level with modulation depth and one repeatable 30% reference. It does not establish a full AM-receiver specification pass. All new real-IF evidence remains local.

Published source: testing/am-iq-optimized, bfa22b8c150ee87c1808418a08cb066160e3834d. Running am-optimized-v1 source fingerprint matched the tested artifact; its embedded base is 34bde79. No firmware rebuild, reflash or cumulative-counter reset was performed for this test.

## Timing ? volume 25%

AM / IF-I, tune +8000 Hz, BW 2, manual gain exponent 4 (factor 16), remaining Preset 2 settings unchanged. After 5.1 seconds settling, host interval was 62.0448071 s; device interval 62.046272 s. Processed 14542 blocks at 234.373469 blocks/s. All streaming-fault, deadline-miss and near-rail increases were zero. Ordinary maximum 3951 us; spectrum-block maximum 4203 us against 4266 us, leaving 63 us (1.48%). Cumulative boot maximum 4417 us was pre-existing. Deadline misses 6?6 and near-rail count 189643?189643 belong in the starting-value record; at least one subsequent deadline miss occurred outside the interval. Thus the observed interval passed with narrow margin, not fault-free operation since boot.

## Signal observations ? volume 100%

Setup requested/confirmed broadly: mixer disconnected; existing 470 ohm / 1 kohm dividers; CH1 8 kHz sine, 300 mVpp displayed, High-Z, +2.5 V offset; internal 1 kHz sine AM; input GP27. GP26 was intended to have DC bias, but raw diagnostics showed variation and the actual bias source was not established. User reported about 160 mVpp at the input, without a complete fresh min/max voltage verification.

Output probe at R1/C2 filter junction, speaker connected. Schematic shows C1 100 uF, R1 100 ohm, C2 470 nF to ground; no amplifier shown. Initial output was dominated by roughly 60?68 mVpp PWM ripple, consistent with the 240 kHz output carrier. The speaker was inaudible.

At gain exponent 4 and volume 25%, user-reported AUD rose from 70 (AM 0%) to 230 (AM 30%). This is broadband digital RMS, not a 1 kHz component measurement. Code inspection found that volume-25% PWM conversion requires sample magnitude 527 before duty changes by one count; a previously sampled peak of 384 would produce constant duty for its block after any interpolation transient. This motivated a volume-only trial at 100%, not a firmware change. It is a supported mechanism, not an isolated proof that it caused all earlier symptoms.

The user then observed a peak near 1 kHz on scope CH1 FFT (requested view 0?5 kHz, acquisition 10 ms/div). At volume 100%, gain exponent 4:

| AM depth | Reported level near 1 kHz |
| --- | ---: |
| 0% | Background approximately -80 dBm; no visible peak |
| 20% | Peak approximately -62 dBm (corrected user reading) |
| 30% | Peak approximately -56 dBm |
| 40% | Peak approximately -53 dBm |
| 60% | Peak approximately -48 dBm |
| 30% repeated | Approximately -56 dBm again |

The 30% peak is about 24 dB above the AM-off background, supporting the proposed 10 dB reduction target under unchanged FFT/gain/volume/load. The off value is noise floor, not a resolved residual tone; exact residual suppression is not claimed. Levels increase with depth, but 20?60% gives 14 dB versus ideal 9.54 dB, so proportionality is not established. Exact recovered frequency within ?2%, clipping/distortion and audible speaker output remain unverified. No new timing capture was performed at volume 100%.

## Report structure

Use: objective and bench targets; setup and measurement provenance; method (separate volume-25% timing from volume-100% spectral measurements); results table; discussion of PWM quantization and remaining nonlinear/inaudible-output issues; limitations and bounded conclusion. Do not treat speaker silence as proof of no recovered electrical component, or the observed FFT peak as proof that all acceptance criteria passed.

Detailed evidence: am-if-demod-2026-10-14-run01.{txt,json,md}, preflight01 and audio-check01 files, plus am-if-demod-2026-10-15-summary.json. Screenshots were viewed in chat, not saved as local image artifacts. Match final report figures to user-retained captures; do not invent them. New evidence has not been committed or pushed.
