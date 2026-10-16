"""Timestamped serial evidence for tuning/bandwidth bench sessions.

Only help/status and explicitly supplied ordinary settings are sent. No reboot,
flash, diagnostic reset or measurement-window command is supported here.
"""
import argparse
import datetime as dt
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

import serial
from serial.tools import list_ports

p = argparse.ArgumentParser()
p.add_argument('--name', required=True)
p.add_argument('--port', default='COM8')
p.add_argument('--set', action='append', default=[])
a = p.parse_args()
allowed = {'mode', 'input', 'tune', 'bw', 'agc', 'gain', 'volume', 'squelch',
           'hang', 'swap', 'iqcorr', 'deemph', 'bass', 'treble', 'blanker',
           'notch', 'nr', 'smooth', 'threshold', 'cw'}
for command in a.set:
    if len(command.split()) != 2 or command.split()[0] not in allowed or '\n' in command or '\r' in command:
        p.error('Only ordinary setting key/value pairs are allowed')
root = Path(__file__).resolve().parents[1]
if Path(a.name).name != a.name:
    p.error('Name must be a filename stem')
stem = root / 'tests/results' / a.name
if any(stem.with_suffix(ext).exists() for ext in ('.txt', '.json')):
    p.error('Choose a new evidence name')
artifact_path = root / 'tests/artifacts/fm-optimized-v2.json'
artifact = json.loads(artifact_path.read_text())
artifact_hash = hashlib.sha256((artifact_path.parent / artifact['artifact']).read_bytes()).hexdigest()
if artifact_hash != artifact['sha256']:
    p.error('Artifact hash mismatch')
identity = artifact['source_identity']
expected_build = ('BUILD label=' + identity['BENCH_BUILD_LABEL'] + ' source_sha256='
                  + identity['BENCH_SOURCE_SHA256'] + ' base=' + identity['BENCH_BASE_COMMIT'])
data = dict(session=a.name, client_local_date='2026-10-16', timezone='Pacific/Auckland',
            branch=subprocess.check_output(['git', 'branch', '--show-current'], cwd=root).decode().strip(),
            commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root).decode().strip(),
            tracked_changes=subprocess.check_output(['git', 'diff', 'HEAD', '--stat'], cwd=root).decode(),
            ports=[dict(device=x.device, serial_number=x.serial_number, hwid=x.hwid) for x in list_ports.comports()],
            artifact_sha256=artifact_hash, expected_identity=identity, actions=[], records=[],
            operator_setup='User reports connected and everything connected; current input voltages and generator readbacks not yet supplied.',
            physical_setup_verified=False, photographs_inspected=[], runtime_identity_verified=False,
            identity_limit='Serial self-report compared to local artifact metadata; no independent flash readback.',
            assessment='Incomplete: setup and electrical measurements pending')

def stamp():
    return dict(utc=dt.datetime.now(dt.timezone.utc).isoformat(timespec='microseconds'),
                host_monotonic_ns=time.perf_counter_ns())

with stem.with_suffix('.txt').open('xb') as raw:
    def mark(kind, value):
        raw.write(('# HOST ' + kind + ' ' + json.dumps(value) + '\n').encode())
        raw.flush()

    try:
        mark('SESSION', data)
        with serial.Serial(a.port, 115200, timeout=.2, write_timeout=2) as port:
            def send(command):
                event = dict(command=command, **stamp())
                data['actions'].append(event)
                mark('TX', event)
                port.write((command + '\n').encode())
                port.flush()

            def receive():
                b = port.readline()
                when = stamp()
                if b:
                    mark('RX', when)
                    raw.write(b)
                    raw.flush()
                return b.decode('ascii', 'replace').strip(), when

            def drain(seconds):
                end = time.perf_counter() + seconds
                lines = []
                while time.perf_counter() < end:
                    line, _ = receive()
                    if line:
                        lines.append(line)
                return lines

            def snapshot(label):
                send('status')
                lines = []
                end = time.perf_counter() + 8
                while time.perf_counter() < end:
                    line, when = receive()
                    if line.startswith('B='):
                        lines = [line]
                        first = when
                    elif lines and line:
                        lines.append(line)
                    if lines and line == 'ENDSTATUS':
                        record = dict(label=label, first_line_received=first,
                                      complete_received=when, lines=lines)
                        data['records'].append(record)
                        return record
                raise RuntimeError('No complete status; partial raw data retained')

            data['connection_lines'] = drain(.5)
            send('help')
            data['help_lines'] = drain(1.2)
            before = snapshot('before_any_settings')
            if expected_build not in data['help_lines']:
                raise RuntimeError('Unexpected runtime identity; no settings changed')
            data['runtime_identity_verified'] = True
            if not any(re.search(r'^MEASURE .*\bactive=0\b', line) for line in before['lines']):
                raise RuntimeError('Existing measurement active or unrecognised; no settings changed')
            for command in a.set:
                send(command)
                replies = drain(.35)
                if any('error' in line.lower() or 'unknown' in line.lower() for line in replies):
                    raise RuntimeError('Setting rejected: ' + command)
            if a.set:
                snapshot('settings_requested_before_settling')
            data['settling_seconds'] = 5.2
            drain(5.2)
            final = snapshot('after_settling')
            print('\n'.join(final['lines']), flush=True)
    except Exception as exc:
        data['error'] = repr(exc)
        mark('ERROR', repr(exc))
        raise
    finally:
        raw.flush()
        data['raw_log_sha256'] = hashlib.sha256(stem.with_suffix('.txt').read_bytes()).hexdigest()
        stem.with_suffix('.json').write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
print('Saved ' + str(stem), flush=True)
