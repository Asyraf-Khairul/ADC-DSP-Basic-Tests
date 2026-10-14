"""Compare fresh-process receiver outputs; never use this host run as Pico timing."""
import argparse
import csv
import hashlib
import json
import math
import pathlib
import struct
import subprocess

parser=argparse.ArgumentParser()
parser.add_argument('--name',required=True)
args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
dest=root/'tests/results'/args.name
if dest.with_suffix('.json').exists() or dest.with_suffix('.csv').exists():
    parser.error('Evidence exists; choose a new name')
names=['am_unmodulated','am_depth10','am_depth30','am_depth60','am_depth90',
       'am_weak','am_low_amplitude','am_high_amplitude','am_biased',
       'am_bias_step','am_amplitude_step','am_high_gain_limiting','silence',
       'two_sines_timing_stimulus','quiet_near_zero','near_rails',
       'am_tuned','amsync','lsb','usb','fm','cw',
       'am_effects','amsync_effects','lsb_effects','usb_effects','fm_effects','cw_effects',
       'am_swapped','am_swapped_iqcorr','am_real_i','am_real_q','am_tune_min','am_tune_max',
       'am_bw_narrow','am_bw_wide','am_swap_tune_transition']
record=struct.Struct('<64hi256BB')
rows=[]; waveforms=[]
def ac_rms(a):
    mean=sum(a)/len(a)
    return math.sqrt(sum((v-mean)**2 for v in a)/len(a))
def frequency(a):
    if ac_rms(a)<16: return None
    mean=sum(a)/len(a)
    crossings=[n+(mean-a[n])/(a[n+1]-a[n]) for n in range(len(a)-1) if a[n]<=mean<a[n+1]]
    return (len(crossings)-1)*15000/(crossings[-1]-crossings[0]) if len(crossings)>1 else None
for scenario,name in enumerate(names):
    records=[]; hashes=[]
    for kind in ('reference','candidate'):
        out=root/'build'/f'receiver-{kind}-{scenario}.bin'
        subprocess.run([str(root/'build'/f'receiver_{kind}.exe'),str(scenario),str(out)],check=True)
        data=out.read_bytes(); hashes.append(hashlib.sha256(data).hexdigest())
        records.append(list(record.iter_unpack(data)))
    ref,opt=records
    assert len(ref)==len(opt)==512
    for phase,begin in (('all',0),('settled',256)):
        a=[v for r in ref[begin:] for v in r[:64]]
        b=[v for r in opt[begin:] for v in r[:64]]
        errors=[y-x for x,y in zip(a,b)]
        rms_a=ac_rms(a); rms_b=ac_rms(b)
        display=[int(y)-int(x) for r,s in zip(ref[begin:],opt[begin:]) for x,y in zip(r[65:321],s[65:321])]
        rows.append(dict(scenario=name,phase=phase,samples=len(a),pcm_max_abs=max(map(abs,errors)),
            pcm_rms=math.sqrt(sum(x*x for x in errors)/len(errors)),pcm_mean=sum(errors)/len(errors),
            reference_ac_rms=rms_a,candidate_ac_rms=rms_b,
            amplitude_difference_percent=100*(rms_b/rms_a-1) if rms_a>=16 else None,
            reference_zero_crossing_Hz=frequency(a) if phase=='settled' else None,
            candidate_zero_crossing_Hz=frequency(b) if phase=='settled' else None,
            pcm_rail_disagreements=sum((abs(x)>=32767)!=(abs(y)>=32767) for x,y in zip(a,b)),
            soft_limit_region_disagreements=sum((abs(x)>=16383)!=(abs(y)>=16383) for x,y in zip(a,b)),
            magnitude_max_abs=max(abs(r[64]-s[64]) for r,s in zip(ref[begin:],opt[begin:])),
            spectrum_max_abs=max(map(abs,display)),spectrum_pixels_differ=sum(x!=0 for x in display),
            spectrum_scale_max_abs=max(abs(r[321]-s[321]) for r,s in zip(ref[begin:],opt[begin:])),
            reference_sha256=hashes[0],candidate_sha256=hashes[1]))
    if scenario in (2,4,10,11):
        for index,(a,b) in enumerate(zip([v for r in ref[256:260] for v in r[:64]], [v for r in opt[256:260] for v in r[:64]])):
            waveforms.append(dict(scenario=name,sample=index,time_seconds=index/15000,reference_pcm=a,candidate_pcm=b))
    print(name, 'identical' if hashes[0]==hashes[1] else 'DIFFERENCES',flush=True)
for suffix,items in (('.csv',rows),('-waveforms.csv',waveforms)):
    path=dest.with_suffix('.csv') if suffix=='.csv' else dest.parent/(dest.name+suffix)
    with path.open('x',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=list(items[0])); w.writeheader(); w.writerows(items)
summary=dict(reference_commit='34bde79a126fc4a7737a4e7194b63833e2aace05',
    implementation='Host GCC, -std=gnu++17 -O3 -fwrapv -ffp-contract=off; single-thread SDK shims, fresh process per scenario',
    limitations=['Host float library may differ from Pico SDK; this comparison is not hardware numerical validation.',
                 'Zero-crossing frequency is descriptive and may count harmonics on clipped/noisy waveforms; silence is excluded below 16 PCM AC RMS.',
                 'Soft limiting is continuous; output threshold crossings are reported, not a firmware saturation flag.',
                 'Other-mode cases are numerical regressions, not hardware timing or demodulation-quality validation.'],
    scenarios=len(names),blocks_per_scenario=512,settled_blocks='256..511',rows=rows,
    pcm_bit_identical=all(r['pcm_max_abs']==0 for r in rows),
    full_record_bit_identical=all(r['reference_sha256']==r['candidate_sha256'] for r in rows))
dest.with_suffix('.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in summary.items() if k not in ('rows','limitations')},indent=2))
