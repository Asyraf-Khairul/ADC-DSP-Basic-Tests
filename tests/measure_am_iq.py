"""AM IQ timing evidence using the existing serial interface; never flash or reset."""
import datetime as dt
import hashlib
import json
import pathlib
import re
import subprocess
import time

import serial
import serial.tools.list_ports

ROOT = pathlib.Path(__file__).resolve().parents[1]
RESULTS = ROOT / 'tests/results'
for run in range(1, 1000):
    stem = RESULTS / f'am-iq-timing-2026-10-14-run{run:02d}'
    if not any(stem.with_suffix(ext).exists() for ext in ('.txt', '.json', '.md')):
        break
else:
    raise RuntimeError('No unused evidence filename')

def stamp():
    return dict(monotonic_ns=time.perf_counter_ns(),
                utc=dt.datetime.now(dt.timezone.utc).isoformat(timespec='microseconds'))

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).decode().strip()

ports = [p for p in serial.tools.list_ports.comports()
         if p.vid == 0x2e8a and p.pid == 0x000a and p.serial_number == 'E664A836A3704126']
if len(ports) != 1:
    raise RuntimeError(f'Expected one known Pico serial device; found {len(ports)}')
device = ports[0]
cache = (ROOT / 'build/CMakeCache.txt').read_text()
data = dict(
    run_name=stem.name, requested_mode='AM IQ only',
    source_commit=git('rev-parse', 'HEAD'), branch=git('branch', '--show-current'),
    initial_git_status=git('status', '--short'),
    local_build_settings=[s for s in cache.splitlines() if s.startswith((
        'CMAKE_BUILD_TYPE:', 'BENCH_SELF_TEST:', 'BENCH_OPERATION_BENCHMARK:',
        'CMAKE_CXX_FLAGS_RELEASE:'))],
    local_uf2_sha256={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest()
                      for p in ('firmware/adc_dsp_suite.uf2', 'build/adc_dsp_standalone.uf2')},
    runtime_identity=dict(port=device.device, usb_serial=device.serial_number,
                          vid=device.vid, pid=device.pid,
                          exact_running_binary_verified=False,
                          limitation='Serial help/status expose no build ID or firmware hash. Local UF2 identity is not a readback of flash.'),
    stimulus=dict(provenance='User-supplied conditions; instruments not queried',
                  mixer='disconnected', waveform_both='continuous sine, 1 kHz, modulation off',
                  generator_both=dict(amplitude_Vpp=0.300, load_indication='High-Z', offset_V=2.5),
                  dividers_ohm=[470, 1000], I_pin='GP27', Q_pin='GP26',
                  earlier_scope_both=dict(mean_V_approx=1.65, Vpp_approx=0.240,
                                          fresh_readings=False), phase='not independently verified'),
    physical_UI_confirmation='unverified', P1_P4_photographs='requested; not received',
    deadline_us=4266, nominal_adc_words_per_second=480000,
    adc_words_per_block=2048, nominal_blocks_per_second=234.375,
    publication_blocks=32, actions=[], records=[],
    constraints='No rebuild, flash, reboot, counter reset, sampling/DSP changes or other receiver mode. Only preset 1 before settling; status queries during interval.',
    raw_log=stem.with_suffix('.txt').name,
    assessment='incomplete')

def save():
    stem.with_suffix('.json').write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')

raw = stem.with_suffix('.txt').open('xb')

def marker(kind, value):
    raw.write(('# HOST ' + kind + ' ' + json.dumps(value) + '\n').encode())
    raw.flush()

def send(port, command):
    event = dict(command=command, **stamp())
    data['actions'].append(event)
    marker('TX', event)
    port.write((command + '\n').encode('ascii'))
    port.flush()
    return event

def receive(port):
    value = port.readline()
    when = stamp()
    if value:
        marker('RX', when)
        raw.write(value)
        raw.flush()
    return value.decode('ascii', 'replace').strip(), when

def drain(port, seconds):
    until = time.perf_counter() + seconds
    while time.perf_counter() < until:
        receive(port)

def snapshot(port, phase):
    request = send(port, 'status')
    lines = []
    first = None
    until = time.perf_counter() + 8
    while time.perf_counter() < until:
        line, when = receive(port)
        if line.startswith('B='):
            lines = [line]
            first = when
        elif lines and line:
            lines.append(line)
        if lines and line.startswith('TIMING '):
            break
    else:
        raise RuntimeError('Incomplete status response; raw bytes retained')
    head = re.fullmatch(r'B=(\d+) rev=(\d+) (\S+) (\S+) tune=(-?\d+) bw=(\d+) agc=(\d+) gain=2\^(\d+) vol=(\d+)', lines[0])
    health = re.fullmatch(r'us=(\d+) max=(\d+) deadline=(\d+) ADCdrop=(\d+) FIFO=(\d+) AUDunder=(\d+) AUDdrop=(\d+) IRQlate=(\d+)', lines[2])
    if not head or not health or len(lines) != 5:
        raise RuntimeError(f'Unexpected status format: {lines}')
    settings = dict(mode=head[3], input=head[4])
    settings.update(zip(('tune_Hz', 'bw', 'agc', 'gain_log2', 'volume_percent'), map(int, head.groups()[4:])))
    settings.update({k: int(v) for k, v in re.findall(r'(\w+)=(-?\d+)', lines[3])})
    settings['gain_factor'] = 1 << settings['gain_log2']
    counters = dict(zip(('deadline_misses', 'adc_drops', 'fifo_overflows', 'audio_underruns', 'audio_drops', 'late_dma_interrupts'), map(int, health.groups()[2:])))
    counters.update(processed_blocks=int(head[1]), near_rail_samples=int(re.search(r'clips=(\d+)', lines[1])[1]))
    timings = {k: dict(latest_us=int(v), cumulative_max_since_boot_us=int(m))
               for k, v, m in re.findall(r'(PROC|OUT|IN|DSP|STAT|FFT)=(\d+)/(\d+)', lines[4])}
    record = dict(phase=phase, request=request, first_line_received=first,
                  complete_received=when,
                  request_to_first_line_ms=(first['monotonic_ns']-request['monotonic_ns'])/1e6,
                  revision=int(head[2]), settings=settings, counters=counters,
                  total_latest_us=int(health[1]), total_cumulative_max_since_boot_us=int(health[2]),
                  stages=timings, raw_lines=lines)
    data['records'].append(record)
    save()
    return record

expected = dict(mode='AM', input='IQ', tune_Hz=0, bw=2, agc=4, gain_log2=4,
                volume_percent=25, sq=-1, hang=3, swap=0, iqcorr=0, deemph=0,
                bass=0, treble=0, blanker=0, notch=0, nr=0, smooth=10,
                threshold=1, cw=700, gain_factor=16)

try:
    marker('IDENTITY_AND_CONDITIONS', {k: v for k, v in data.items() if k != 'records'})
    with serial.Serial(device.device, 115200, timeout=0.25, write_timeout=2) as port:
        # Disable automatic USB reports only. This does not affect DSP or counters.
        send(port, 'stream 0')
        drain(port, 0.5)
        send(port, 'help')
        drain(port, 1)
        data['pre_selection'] = snapshot(port, 'before_preset_1')
        send(port, 'preset 1')
        drain(port, 0.5)
        for _ in range(10):
            confirmed = snapshot(port, 'confirm_preset_1')
            if confirmed['settings'] == expected:
                break
            time.sleep(0.5)
        else:
            raise RuntimeError('AM IQ preset was not confirmed')
        data['confirmed_settings'] = confirmed['settings']
        settle_start = time.perf_counter_ns()
        time.sleep(5.1)
        start = snapshot(port, 'interval_start')
        data['settling_seconds_after_confirmed_AM'] = (start['request']['monotonic_ns']-settle_start)/1e9
        if start['settings'] != expected:
            raise RuntimeError('Settings changed while settling')
        measured = [start]
        origin = start['first_line_received']['monotonic_ns']
        print('INTERVAL_STARTED ' + stem.name + ' -- check UI and capture P1/P4 without applying settings', flush=True)
        sample = 0
        while True:
            sample += 1
            target = origin + sample * 2_000_000_000
            time.sleep(max(0, (target-time.perf_counter_ns())/1e9))
            record = snapshot(port, 'interval_sample')
            measured.append(record)
            elapsed = (record['first_line_received']['monotonic_ns']-origin)/1e9
            if record['settings'] != expected or record['revision'] != start['revision']:
                raise RuntimeError('Settings/revision changed during measurement; interval invalid')
            if any(record['counters'][k] < measured[-2]['counters'][k] for k in record['counters']):
                raise RuntimeError('Counter decreased: possible reset or wrap; interval invalid')
            if sample % 5 == 0:
                print(json.dumps(dict(elapsed_seconds=elapsed, counters=record['counters'],
                                      latest_total_us=record['total_latest_us'], stages=record['stages'])), flush=True)
            if elapsed >= 62:
                record['phase'] = 'interval_end'
                break
        end = measured[-1]
        changes = {k: end['counters'][k]-start['counters'][k] for k in start['counters']}
        rate = changes['processed_blocks']/elapsed
        # Publication quantization is <32 blocks at nominal sustained throughput.
        # Add measured endpoint request/receive delays as a conservative host bound.
        tolerance_blocks = 32 + 234.375 * (start['request_to_first_line_ms']+end['request_to_first_line_ms'])/1000
        rate_ok = abs(changes['processed_blocks'] - elapsed*234.375) <= tolerance_blocks
        faults = [k for k in changes if k not in ('processed_blocks', 'near_rail_samples') and changes[k] != 0]
        stages = {k: dict(start_latest_us=start['stages'][k]['latest_us'],
                          end_latest_us=end['stages'][k]['latest_us'],
                          largest_sampled_latest_us=max(r['stages'][k]['latest_us'] for r in measured),
                          start_cumulative_max_since_boot_us=start['stages'][k]['cumulative_max_since_boot_us'],
                          end_cumulative_max_since_boot_us=end['stages'][k]['cumulative_max_since_boot_us'])
                  for k in start['stages']}
        timing_pass = rate_ok and not faults and max(r['total_latest_us'] for r in measured) <= 4266
        data['interval'] = dict(
            timestamp_basis='Difference between host perf_counter_ns reception timestamps for the first B= line in the selected start/end records; UTC timestamps also saved for every received line.',
            elapsed_seconds=elapsed, start=start, end=end, snapshot_count=len(measured),
            changes=changes, approximate_blocks_per_second=rate,
            nominal_rate_compatible=rate_ok, endpoint_publication_and_serial_tolerance_blocks=tolerance_blocks,
            largest_sampled_total_latest_us=max(r['total_latest_us'] for r in measured),
            stages=stages, timing_and_streaming_pass=timing_pass,
            streaming_faults_with_increases=faults,
            input_condition_issue_near_rail_increase=changes['near_rail_samples'] != 0)
        data['limitations'] = [
            'Counters and maxima at interval start include earlier operation and the preset transition/settling. Only endpoint counter differences are attributed to this record interval.',
            'Serial has no device timestamp. Reports contain a coherent status published every 32 processed blocks; host times are receive times, not exact block completion times.',
            'Largest sampled latest timings do not capture every block. Stage maxima since boot may originate from different blocks and must not be summed.',
            'Spectrum is prepared when the pre-increment block count is divisible by 64. Publication after increment at multiples of 32 misses those latest FFT values; FFT latest=0 does not imply zero spectrum cost. Its cumulative maximum is available.',
            'No fresh scope readings or direct AFG queries. Earlier voltage measurements are not current verification.',
            'Two unmodulated sine inputs test timing only; no demodulation-performance claim.',
            'OLED/control responsiveness and P1/P4 captures remain unverified until the user confirms them.'
        ]
        data['assessment'] = 'PASS observed AM IQ timing/streaming interval' if timing_pass else 'FAIL observed AM IQ timing/streaming interval'
        save()
        marker('INTERVAL_RESULT', data['interval'])
        print(json.dumps({k: data[k] for k in ('run_name', 'assessment', 'interval')}, indent=2), flush=True)
except Exception as exc:
    data['assessment'] = 'INCOMPLETE or INVALID; retain raw evidence'
    data['error'] = repr(exc)
    save()
    marker('ERROR', data['error'])
    raise
finally:
    raw.close()
