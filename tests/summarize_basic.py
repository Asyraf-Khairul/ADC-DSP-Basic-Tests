"""Produce numerical summary and attach the user's physical observations to the log."""
import csv
import hashlib
import json
import math
import pathlib
import subprocess

root = pathlib.Path(__file__).resolve().parents[1]
results = root / "tests/results"
rows = list(csv.DictReader((results / "basic-numerical-comparison.csv").open()))
chosen = [r for r in rows if r["candidate"] == "lookup_q8"]
summary = []
for gain in (0,4,10):
    for phase in ("all","settled"):
        subset = [r for r in chosen if int(r["gain_log2"]) == gain and r["phase"] == phase]
        count = sum(int(r["samples"]) for r in subset)
        entry = dict(gain_log2=gain, phase=phase, samples=count)
        for prefix in ("mag","pcm"):
            entry[prefix+"_max_abs"] = max(float(r[prefix+"_max_abs"]) for r in subset)
            entry[prefix+"_mean_signed"] = sum(float(r[prefix+"_mean_signed"])*int(r["samples"]) for r in subset)/count
            entry[prefix+"_rms_error"] = math.sqrt(sum(float(r[prefix+"_rms_error"])**2*int(r["samples"]) for r in subset)/count)
        meaningful = [r for r in subset if r["amplitude_error_percent"]]
        entry["max_abs_amplitude_percent_when_reference_ac_rms_ge_16"] = max(abs(float(r["amplitude_error_percent"])) for r in meaningful)
        entry["clip_disagreements"] = sum(int(r["clip_disagreement"]) for r in subset)
        summary.append(entry)
data = dict(host_compiler=subprocess.check_output(["g++","--version"]).decode().splitlines()[0],
    host_flags="-std=c++17 -O3 -ffp-contract=off -Wall -Wextra -Werror",
    scenarios=sorted(set(r["scenario"] for r in chosen)), blocks_per_scenario_and_gain=1024,
    settled_blocks="768..1023", magnitude_units="ADC-count equivalent", audio_units="signed PCM counts",
    amplitude_metric="AC RMS difference in percent, blank below reference AC RMS 16 PCM",
    summary=summary)
(results / "basic-numerical-summary.json").write_text(json.dumps(data,indent=2))
stream_path = results / "basic-optimized-streaming-2026-10-13.json"
stream = json.loads(stream_path.read_text())
stream["physical_UI_confirmation"] = "User confirmed OLED, encoder and both buttons remained responsive during the run"
stream["current_scope_recheck"] = dict(provenance="user reported in GP27/GP26 order",
    GP27_mean_V=1.653,GP26_mean_V=1.652,GP27_Vpp=0.240,GP26_Vpp=0.240,
    AFG_CH1_offset_V=2.5,AFG_CH2_offset_V=2.5,
    note="Scope and AFG readings supplied by user; ADC counter/timing logs acquired by the collector")
stream_path.write_text(json.dumps(stream,indent=2))
tested = dict(commit=subprocess.check_output(["git","rev-parse","HEAD"],cwd=root).decode().strip(),
    branch="testing/dsp-suite", flags="Release -g -O3 -DNDEBUG; no global fast-math",
    normal_options="BENCH_SELF_TEST=OFF BENCH_OPERATION_BENCHMARK=OFF",
    builds=[
        dict(name="instrumented baseline",sha256="a30ee264c65beea3c5e9fbd52bb876dab32d9dec83e199090da55f88e13c03e7",
            snapshot="firmware/evidence/baseline-profile-81dbbd9",log="tests/results/basic-profile-2026-10-13.txt"),
        dict(name="isolated operation benchmark",sha256="c9ee45a5892dd5db01a97d4412222190d11be049e1b00b6f5c28c6b9961af327",
            snapshot="firmware/evidence/operation-benchmark",options="BENCH_OPERATION_BENCHMARK=ON",
            log="tests/results/basic-operation-benchmark-2026-10-13.txt"),
        dict(name="first streaming candidate, 64-bit operations in flash",sha256="58a8e7addf02f1e154a0b2053722f34591d73df43b0c2a9b59561693bc03b23f",
            local_changes="bench.cpp uses BasicKernel; compatible Q16 DC and Q8 lookup from operation-benchmark snapshot, Q24 audio DC, no RAM placement",
            log="tests/results/basic-optimized-startup-2026-10-13.txt",result="FAIL max=4728us LATE=37"),
        dict(name="RAM candidate retaining 64-bit hot-loop operations",sha256="2348b63514417f74cbf3990a64437ecf8dde6dcbc0a3ea693a98d51f564ed9d5",
            snapshot="firmware/evidence/optimized-ram-final",log="tests/results/basic-ram-startup-outputs-off-2026-10-13.txt",
            result="FAIL max=4547us LATE=2"),
        dict(name="final RAM and 32-bit hot-loop candidate",sha256=hashlib.sha256((root/"firmware/adc_dsp_suite.uf2").read_bytes()).hexdigest(),
            snapshot="firmware/evidence/optimized-32bit-final",log="tests/results/basic-optimized-streaming-2026-10-13.txt",
            result="PASS BASIC timing and counters; physical UI and current scope/AFG readings confirmed by user")])
(results / "basic-tested-builds.json").write_text(json.dumps(tested,indent=2))
print(json.dumps(data["summary"],indent=2))
