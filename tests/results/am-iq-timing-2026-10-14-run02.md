# AM IQ timing run02 - 14 October 2026

**FAIL for sustained timing/streaming under the observed conditions.** Both AFG outputs were reported ON, both ADC channels varied around midscale, and no additional near-rail samples occurred. Every processed block between the selected records added a deadline miss. No demodulation performance claim is made.

## Identity and procedure

Local branch `testing/dsp-suite`, source commit `34bde79a126fc4a7737a4e7194b63833e2aace05`. The tracked source was unchanged. Local Release cache: `-g -O3 -DNDEBUG`, `BENCH_SELF_TEST=OFF`, `BENCH_OPERATION_BENCHMARK=OFF`. Both local UF2 files match SHA256 `43f9dab0eddcaf592257c37771d3721fc90f6b6eb86420d681081113146f67c1`. This identifies local files, not a readback of the running firmware. The serial interface exposes no build ID/hash, so the exact flashed revision is not independently verified. Connected device: `COM8`, USB serial `E664A836A3704126`.

No rebuild, flash, reboot, diagnostic reset, DSP calculation change or sampling-rate change was performed. `stream 0` disabled automatic USB reports; `help` and `status` were inspected; `preset 1` was applied before settling. After AM IQ was confirmed, settling lasted 5.1003712 seconds. Only `status` queries were sent during the interval; all 32 measured snapshots retained settings revision 4 and identical settings. OLED and physical controls remained available.

All settings: AM, IQ, tune 0 Hz, BW 2, AGC 4/manual, gain exponent 4 (factor 16), volume 25%, squelch -1/off, hang 3, swap 0, IQ correction 0, de-emphasis 0, bass 0, treble 0, blanker 0, notch 0, noise reduction 0, smoothing 10, threshold 1, CW 700 Hz.

## Timing window and counter results

Start first-line receipt: `2026-10-14T04:59:30.984106+00:00` UTC (`2026-10-14T17:59:30.984106+13:00` NZDT).
End first-line receipt: `2026-10-14T05:00:33.056927+00:00` UTC (`2026-10-14T18:00:33.056927+13:00` NZDT).
The elapsed time is **62.0724053 seconds**, computed using host monotonic high-resolution timestamps, not rounded wall-clock strings. Request, first-line receipt, completion and raw-line timestamps are retained. UTC clock difference is not used to calculate duration.

| Counter | Start | End | Increase |
| --- | ---: | ---: | ---: |
| Processed blocks | 65376 | 78336 | 12960 |
| Deadline misses | 62138 | 75098 | 12960 |
| ADC dropped blocks | 7618 | 9220 | 1602 |
| ADC FIFO overflows | 0 | 0 | 0 |
| Audio underruns | 7622 | 9224 | 1602 |
| Audio dropped blocks | 0 | 0 | 0 |
| Late DMA interrupts | 0 | 0 | 0 |
| Near-rail samples | 73115 | 73115 | 0 |

Throughput: **208.788429 blocks/s**, versus nominal **234.375 blocks/s** from 480000 ADC words/s / 2048 words/block. Reports use coherent status published every 32 completed blocks; the device exposes no block timestamp. Nominal publication spacing is about 136.533 ms (longer under overload). The processed-block deficit is much larger than publication granularity and endpoint serial delay: the conservative nominal-throughput allowance used here is 61.826 blocks. ADC drops and underruns independently establish loss.

The starting counters are preserved historical values; they include earlier operation, the CH1-off run and the mode transition/settling. They are **not** losses attributed to run02. Only differences between the selected records are attributed to this measured record interval. Near-rail count 73115 is historical and remained constant.

## Processing stages and maxima

Deadline: **4266 us**. Largest sampled latest total: **4864 us**. The unchanged **8188 us** total maximum existed before this interval; it is cumulative since boot and must not be called this run's measured maximum.

| Timing (us) | Start latest | End latest | Largest sampled latest | Start max since boot | End max since boot |
| --- | ---: | ---: | ---: | ---: | ---: |
| Total process + output | 4790 | 4792 | 4864 | 8188 | 8188 |
| Process subtotal | 4587 | 4588 | 4658 | 8000 | 8000 |
| ADC statistics | 731 | 731 | 795 | 1082 | 1140 |
| Receiver DSP | 3820 | 3821 | 3842 | 4104 | 4143 |
| Audio statistics | 35 | 36 | 58 | 82 | 82 |
| Spectrum preparation | 0 | 0 | 0 | 3508 | 3508 |
| Audio preparation | 203 | 204 | 214 | 318 | 425 |

Receiver DSP is the dominant sampled stage: 3821 us, about 79.7% of the final sampled total. The receiver-DSP maximum increased from 4104 to 4143 us even though the largest sampled latest DSP value was 3842 us, illustrating that serial snapshots miss blocks. ADC statistics and audio preparation also add to the inclusive total. These are elapsed times including interrupt service, not isolated CPU-cycle measurements.

Spectrum preparation runs before increment when the block count is divisible by 64, while status is published after increment at multiples of 32. Consequently every published latest FFT value is zero. This is a sampling-phase limitation, not evidence that spectrum preparation is free or disabled. Its cumulative maximum is 3508 us at both endpoints; its maximum within this interval cannot be isolated from this run's serial reports. Stage maxima may come from different blocks and must not be summed. PROC is a subtotal including IN/DSP/STAT/FFT, not an additional stage.

The evidence identifies the receiver DSP path as the dominant ordinary-block cost. Further investigation should profile its internal stages and separately measure spectrum preparation before choosing an optimization. No such code changes were made, and no other receiver modes were tested.

## Inputs, UI and limits

User-supplied setup: mixer disconnected; both AFG channels continuous 1 kHz sine, 300 mVpp setting, High-Z indication, +2.5 V offset, modulation off; existing 470-ohm/1-kohm dividers; I at GP27 and Q at GP26. User explicitly confirmed both outputs on before run02. Earlier scope readings (~1.65 V mean / 240 mVpp per channel) are **not fresh readings**. No direct generator/scope query was performed, and phase was not independently verified.

Start ADC counts: `I min/mean/max/rms=1943/2077/2205/87 Q=1940/2075/2208/87 MAG=224 AUD rms/peak=1696/3040 clips=73115`

End ADC counts: `I min/mean/max/rms=1942/2073/2206/87 Q=1940/2072/2204/87 MAG=229 AUD rms/peak=1684/3040 clips=73115`

No increase in near-rail samples occurred. This supports the observed input condition but does not substitute for fresh scope voltage measurements. The separate run01 had CH1 OFF and is retained with that correction; its interval is not pooled with run02.

User confirmed: **Responsive; P1 and P4 photographed.** The photographs have not been supplied or independently inspected. Timing/streaming still fails despite responsive controls. The two unmodulated inputs establish no AM demodulation-performance result.

## Evidence

- [Raw serial output and host timestamps](am-iq-timing-2026-10-14-run02.txt)
- [Machine-readable summary and every parsed status](am-iq-timing-2026-10-14-run02.json)
- Collector: `tests/measure_am_iq.py`
- Raw log SHA256: `ce901fab34e8ab61944bd7a16b18adb4faf3dd6de470cd9004e9f1463dc5db4e`

Every parsed status was checked against the raw log, endpoint arithmetic was recomputed, and constant settings/revision and status-only interval commands were verified. Files are local; no commit or push was performed. The Pico was left in the tested AM IQ configuration, so live counters may continue increasing after the selected end record.
