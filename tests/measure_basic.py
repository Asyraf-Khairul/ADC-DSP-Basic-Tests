"""Read-only BASIC counter/timing run; optional serial page cycling never changes DSP."""
import argparse
import hashlib
import json
import pathlib
import re
import sys
import time
import serial

parser = argparse.ArgumentParser()
parser.add_argument("--port", default="COM8")
parser.add_argument("--duration", type=float, default=60)
parser.add_argument("--name", default="basic-optimized-streaming-2026-10-13")
args = parser.parse_args()
if args.duration < 60:
    parser.error("Final streaming measurements must last at least 60 seconds")
root = pathlib.Path(__file__).resolve().parents[1]
log = root / "tests/results" / (args.name + ".txt")
startup_path = log.parent / ("startup-" + log.name)
if any(path.exists() for path in (log, log.with_suffix(".json"), startup_path)):
    parser.error("Existing test evidence is preserved; choose a new --name")
sys.stdout.reconfigure(line_buffering=True)

def status(port):
    port.reset_input_buffer()
    port.write(b"status\n")
    lines = []
    for _ in range(40):
        line = port.readline().decode("ascii", "replace").strip()
        if not line:
            continue
        if line.startswith("B="):
            lines = []
        lines.append(line)
        if line.startswith("TIMING "):
            return "\n".join(lines) + "\n"
    raise RuntimeError("Full profiling status response not received")

def parse(text):
    b = re.search(r"^B=(\d+) rev=(\d+) (.+)", text, re.M)
    h = re.search(r"^us=(\d+) max=(\d+) deadline=(\d+) ADCdrop=(\d+) FIFO=(\d+) AUDunder=(\d+) AUDdrop=(\d+) IRQlate=(\d+)", text, re.M)
    c = re.search(r"clips=(\d+)", text)
    if not (b and h and c):
        raise RuntimeError("Incomplete snapshot")
    values = dict(zip(("Total", "Maximum", "Late", "ADCdrop", "FIFO", "AUDunder", "AUDdrop", "IRQlate"), map(int, h.groups())))
    values.update(B=int(b[1]), Revision=int(b[2]), Settings=b[3], Clips=int(c[1]))
    return values

firmware = root / "firmware/adc_dsp_suite.uf2"
header = ("UF2 SHA256: " + hashlib.sha256(firmware.read_bytes()).hexdigest() + "\n"
    "Source: 81dbbd9 plus local profiling and BASIC optimization\n"
    "Fresh USB firmware reboot; Pico power kept connected\n"
    "Serial page cycling every 10 seconds; physical UI confirmation collected separately\n")
with serial.Serial(args.port, 115200, timeout=3, write_timeout=2) as port:
    startup = status(port)
    startup_path.write_text(startup)
    time.sleep(2)
    first_text = status(port)
    first = parse(first_text)
    if first["Settings"] != "BASIC IQ tune=0 bw=2 agc=4 gain=2^4 vol=25":
        raise RuntimeError("Expected unchanged default BASIC settings")
    with log.open("w", encoding="utf-8") as output:
        output.write(header + "HOST elapsed_ms=0\n" + first_text)
        output.flush()
        print(first_text, end="")
        start = time.monotonic()
        sample = 0
        while True:
            sample += 1
            target = min(sample*10, args.duration)
            time.sleep(max(0, target-(time.monotonic()-start)))
            port.write(f"page {sample%5}\n".encode())
            latest_text = status(port)
            elapsed = time.monotonic()-start
            latest = parse(latest_text)
            record = f"HOST elapsed_ms={elapsed*1000:.3f}\n" + latest_text
            output.write(record); output.flush(); print(record, end="")
            if latest["Settings"] != first["Settings"] or latest["Revision"] != first["Revision"]:
                raise RuntimeError("DSP settings changed during measurement; retain partial log")
            if elapsed >= args.duration:
                break
changes = {k: latest[k]-first[k] for k in ("B", "Late", "ADCdrop", "FIFO", "AUDunder", "AUDdrop", "IRQlate", "Clips")}
rate = changes["B"]/elapsed
passed = latest["Maximum"] <= 4266 and latest["Late"] == 0 and abs(rate-234.375) <= 1 and all(
    changes[k] == 0 for k in changes if k != "B")
summary = dict(duration_seconds=elapsed, processed_blocks_per_second=rate,
    maximum_total_us=latest["Maximum"], first=first, last=latest, increases=changes,
    timing_and_counter_pass=passed, physical_UI_confirmation="pending", current_scope_recheck="pending")
log.with_suffix(".json").write_text(json.dumps(summary, indent=2))
print(json.dumps(summary, indent=2))
raise SystemExit(0 if passed else 2)
