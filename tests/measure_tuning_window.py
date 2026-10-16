"""Section4.5.8 USB IF-I all-block timing at fixed tune5500,bw4.
No settings changes, firmware changes, rates or cumulative-counter resets. Scope data remain manual.
"""
import argparse
import datetime as dt
import hashlib
import json
import pathlib
import re
import subprocess
import time
import serial

parser=argparse.ArgumentParser()
parser.add_argument('--name',required=True)
parser.add_argument('--port',default='COM8')
parser.add_argument('--seconds',type=float,default=62)
parser.add_argument('--restart',required=True)
parser.add_argument('--mode',choices=('usb',),default='usb')
parser.add_argument('--artifact',default='tests/artifacts/fm-optimized-v2.json',help='Expected firmware artifact metadata, relative to repository root')
args=parser.parse_args()
if args.seconds<60: parser.error('At least 60 seconds required')
root=pathlib.Path(__file__).resolve().parents[1]
stem=root/'tests/results'/args.name
if any(stem.with_suffix(x).exists() for x in ('.txt','.json')): parser.error('Choose an unused run name')
artifact_path=root/args.artifact
artifact=json.loads(artifact_path.read_text())
artifact_binary=artifact_path.parent/artifact['artifact']
artifact_hash=hashlib.sha256(artifact_binary.read_bytes()).hexdigest()
if artifact_hash != artifact['sha256']: parser.error('Artifact hash does not match metadata')
identity=artifact['source_identity']
plan_path=root/'tests/results/tuning-bandwidth-2026-10-16-plan01.json'
plan=json.loads(plan_path.read_text())
data=dict(name=args.name,client_local_date='2026-10-16',timezone='Pacific/Auckland',source_identity=identity,
    publication_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root).decode().strip(),
    branch=subprocess.check_output(['git','branch','--show-current'],cwd=root).decode().strip(),
    tracked_source_diff=subprocess.check_output(['git','diff','HEAD','--stat'],cwd=root).decode(),
    artifact_metadata=args.artifact,expected_UF2_sha256=artifact_hash,restart_method=args.restart,
    physical_evidence=plan['operator_measurements'],photographs=plan['photographs'],
    scope_readings='Operator-reported GP27 mean1.65V,min1.52V,max1.78V;GP26 DC1.65V. No scope image inspected. Generator exact current readbacks not independently confirmed.',
    setup='Instructed generator CH1 continuous8kHz sine,300mVpp High-Z,+2.5V offset,modulationoff via divider toGP27;GP26 externally biased; mixer disconnected; speaker retained. User confirmed connections generally.',
    prior_setup_faults='One earlier deadline miss, cumulative max4408us; near-rail398614 pre-existing and stable. Preserve these; assess interval deltas separately.',
    audio_probe_location='Filtered R1/C2 junction instructed, operator reports output FFT readings and two saved photos; not independently inspected.',
    identity_limit='Serial self-report matched metadata; no independent flash readback.',
    UI_confirmation='Not requested or verified for this run',actions=[],records=[],assessment='incomplete')

def stamp(): return dict(mono_ns=time.perf_counter_ns(),utc=dt.datetime.now(dt.timezone.utc).isoformat(timespec='microseconds'))
def save(): stem.with_suffix('.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
raw=stem.with_suffix('.txt').open('xb')
def mark(kind,obj):
    raw.write(('# HOST '+kind+' '+json.dumps(obj)+'\n').encode()); raw.flush()
def send(command):
    event=dict(command=command,**stamp()); data['actions'].append(event); mark('TX',event)
    port.write((command+'\n').encode()); port.flush(); return event
def receive():
    value=port.readline(); when=stamp()
    if value: mark('RX',when); raw.write(value); raw.flush()
    return value.decode('ascii','replace').strip(),when
def drain(seconds):
    result=[]; until=time.perf_counter()+seconds
    while time.perf_counter()<until:
        line,_=receive()
        if line: result.append(line)
    return result
def fields(line): return {k:int(v) for k,v in re.findall(r'(\w+)=(-?\d+)(?![\d/])',line)}
def snapshot(phase):
    request=send('status'); lines=[]; until=time.perf_counter()+10
    while time.perf_counter()<until:
        line,when=receive()
        if line.startswith('B='): lines=[line]; first=when
        elif lines and line: lines.append(line)
        if line=='ENDSTATUS' and lines: break
    else: raise RuntimeError('Missing ENDSTATUS; preserve partial raw log')
    by_prefix={line.split(' ')[0]:line for line in lines}
    h=re.fullmatch(r'B=(\d+) rev=(\d+) (\S+) (\S+) tune=(-?\d+) bw=(\d+) agc=(\d+) gain=2\^(\d+) vol=(\d+)',lines[0])
    if not h: raise RuntimeError('Invalid status header')
    settings=dict(mode=h[3],input=h[4],**dict(zip(('tune','bw','agc','gain_log2','volume'),map(int,h.groups()[4:]))),**fields(lines[3]))
    groups={}
    for key in ('ORDINARY','SPECTRUM'):
        line=by_prefix['WINDOW_'+key]
        groups[key]=dict(N=int(re.search(r'N=(\d+)',line)[1]),LATE=int(re.search(r'LATE=(\d+)',line)[1]),
                        stages={k:dict(max_us=int(a),sum_us=int(b)) for k,a,b in re.findall(r'(\w+)=(\d+)/(\d+)',line)})
    record=dict(phase=phase,request=request,first_line_received=first,complete_received=when,
                B=int(h[1]),revision=int(h[2]),settings=settings,health=fields(lines[2]),
                clips=int(re.search(r'clips=(\d+)',lines[1])[1]),
                measure=fields(by_prefix['MEASURE']),start=fields(by_prefix['WINDOW_START']),
                end=fields(by_prefix['WINDOW_END']),groups=groups,bookkeeping=fields(by_prefix['BOOKKEEP']),
                raw_lines=lines)
    data['records'].append(record); save(); return record

expected=dict(mode='USB',input='IF-I',tune=5500,bw=4,agc=4,gain_log2=4,volume=25,
              sq=-1,hang=3,swap=0,iqcorr=0,deemph=0,bass=0,treble=0,blanker=0,notch=0,nr=0,smooth=10,threshold=1,cw=700)
port=None; measuring=False
try:
    mark('RUN',data)
    until=time.perf_counter()+20
    while True:
        try:
            port=serial.Serial(args.port,115200,timeout=0.25,write_timeout=2); break
        except serial.SerialException:
            if time.perf_counter()>=until: raise
            time.sleep(0.1)
    data['connection_serial']=drain(0.5)
    send('stream 0'); send('help'); help_lines=drain(0.8)
    data['runtime_help']=help_lines
    expected_build='BUILD label='+identity['BENCH_BUILD_LABEL']+' source_sha256='+identity['BENCH_SOURCE_SHA256']+' base='+identity['BENCH_BASE_COMMIT']
    if expected_build not in help_lines: raise RuntimeError('Running build identity does not match the locally prepared image')
    data['runtime_source_identity_verified']=True
    before=snapshot('existing_settings_before_settling')
    if before['measure']['active']: raise RuntimeError('Existing capture is active; refusing to replace it')
    if before['settings']!=expected: raise RuntimeError('Settings differ; refusing to change them')
    data['mode_transition']='None; no receiver settings changed'
    data['settings_verification']='All settings explicitly matched; preset number itself not exposed'
    time.sleep(5.1)
    settled=snapshot('settled_before_measurement')
    if settled['settings']!=expected: raise RuntimeError('Settings changed during settling')
    event_start=send('measure start'); measuring=True
    for _ in range(10):
        first=snapshot('measurement_start_acknowledged')
        if first['measure']['active']: break
        time.sleep(0.1)
    else: raise RuntimeError('Capture did not start')
    capture_id=first['measure']['id']; t0=first['first_line_received']['mono_ns']
    print('MEASUREMENT_ACTIVE '+args.name+' -- leave generator and DSP settings unchanged',flush=True)
    sample=0
    while True:
        sample+=1
        target=t0+min(sample*5,args.seconds)*1e9
        time.sleep(max(0,(target-time.perf_counter_ns())/1e9))
        r=snapshot('measurement_sample')
        elapsed=(r['first_line_received']['mono_ns']-t0)/1e9
        if r['settings']!=expected or r['revision']!=first['revision'] or r['measure']['id']!=capture_id or r['measure']['settings_changed']:
            raise RuntimeError('Settings or capture changed during measurement')
        if sample%2==0: print(json.dumps(dict(elapsed=elapsed,health=r['health'],ordinary_max_us=r['groups']['ORDINARY']['stages']['TOTAL']['max_us'],spectrum_max_us=r['groups']['SPECTRUM']['stages']['TOTAL']['max_us'])),flush=True)
        if elapsed>=args.seconds: break
    event_stop=send('measure stop')
    for _ in range(10):
        last=snapshot('measurement_stopped')
        if not last['measure']['active']: break
        time.sleep(0.1)
    else: raise RuntimeError('Capture did not stop')
    measuring=False
    if last['measure']['id']!=capture_id or last['measure']['settings_changed'] or last['settings']!=expected or last['revision']!=first['revision']: raise RuntimeError('Invalid final capture')
    changes={k:last['end'][k]-last['start'][k] for k in ('B','deadline','clips','ADCdrop','FIFO','AUDunder','AUDdrop','IRQlate')}
    device_seconds=(last['end']['device_us']-last['start']['device_us'])/1e6
    host_seconds=(last['first_line_received']['mono_ns']-t0)/1e9
    count=sum(g['N'] for g in last['groups'].values())
    if count!=changes['B'] or sum(g['LATE'] for g in last['groups'].values())!=changes['deadline']: raise RuntimeError('Window counts disagree with cumulative counter differences')
    maximum=max(g['stages']['TOTAL']['max_us'] for g in last['groups'].values())
    rate=count/device_seconds
    passed=(device_seconds>=60 and host_seconds>=60 and abs(rate-234.375)<1 and maximum<=4266
            and all(changes[k]==0 for k in changes if k not in ('B','clips')))
    data['interval']=dict(host_first_start_record_to_stopped_record_seconds=host_seconds,
        device_capture_seconds=device_seconds,host_start_command=event_start,host_stop_command=event_stop,
        start=last['start'],end=last['end'],counter_increases=changes,blocks_per_second=rate,
        groups=last['groups'],all_block_maximum_total_us=maximum,headroom_us=4266-maximum,
        sampled_latest_max_us=max(r['health']['us'] for r in data['records'] if r['phase'].startswith('measurement')),
        cumulative_max_since_boot_at_start_us=first['health']['max'],
        cumulative_max_since_boot_at_end_us=last['health']['max'],
        start_revision=last['start']['rev'],end_revision=last['end']['rev'],
        bookkeeping=last['bookkeeping'],pass_timing_and_streaming=passed, near_rail_increase=changes['clips'], scope_input_limits_operator_reported=True, scope_input_limits_independently_verified=False)
    data['assessment']=('PASS' if passed else 'FAIL')+' observed '+args.mode.upper()+' IF-I timing/streaming interval; near-rail changes reported separately'
    samples=[r for r in data['records'] if r['phase'].startswith('measurement')]
    audio=[tuple(map(int,re.search(r'AUD rms/peak=(\d+)/(\d+)',r['raw_lines'][1]).groups())) for r in samples]
    data['digital_audio_snapshots']=dict(rms_min=min(x[0] for x in audio),rms_max=max(x[0] for x in audio),peak_min=min(x[1] for x in audio),peak_max=max(x[1] for x in audio),limitation='Broadband digital samples before volume/PWM, not selective electrical-output tone level')
    save(); mark('RESULT',data['interval'])
    print(json.dumps({'assessment':data['assessment'],'interval':data['interval']},indent=2),flush=True)
except Exception as e:
    data['error']=repr(e); data['assessment']='INCOMPLETE or INVALID'; save(); mark('ERROR',repr(e)); raise
finally:
    if port:
        if measuring:
            send('measure stop')
        port.close()
    raw.close()
    data['raw_log_sha256']=hashlib.sha256(stem.with_suffix('.txt').read_bytes()).hexdigest()
    save()
