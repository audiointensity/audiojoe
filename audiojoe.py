import wave
import sys
import struct
import io
from subprocess import Popen, PIPE
import msvcrt
import os
import numpy as np

def index_of_max(peaks, start_i, end_i):
    is_init = False
    index = 0
    for i in range(start_i, end_i+1):
        if not is_init or max_el < peaks[i]['prom']:
            max_el = peaks[i]['prom']
            index = i
            is_init = True
    return index

# Parameters:
#  array v     - an array of audio samples as integers
#  int start_i - start index in v
#  int end_i   -  end index in v
# Returns a list of dicts with {x, y, prom}
def find_peaks(v, start_i, end_i):
    if start_i > end_i:
        print "Warning (find_peaks): start index > end index"
        return []
    MAX_PEAKS = end_i - start_i + 1
    peaks = []
    peaks_i = 0
    minlocs = [0]*MAX_PEAKS
    minloc_i = 0
    for i in range(start_i+1, end_i-1):
        if v[i-1] < v[i] and v[i] >= v[i+1]:
            if minloc_i == 0:
                minlocs[minloc_i] = v[start_i]
                minloc_i += 1
            peaks.append({'x': i, 'y': v[i], 'prom': 0})
            peaks_i += 1
        if v[i-1] > v[i] and v[i] <= v[i+1]:
            minlocs[minloc_i] = v[i]
            minloc_i += 1
    if minloc_i == peaks_i:
        minlocs[minloc_i] = v[end_i]
        minloc_i += 1
    N = peaks_i
    for s in range(0, N):
        for e in range(s+1, N):
            h = index_of_max(peaks, s, e)
            peaks[h]['prom'] = max(peaks[h]['prom'], 
                                   peaks[h]['y'] - max(minlocs[s], minlocs[e]))
    return peaks
        
def penalize_early_peaks(peaks):
    def mod_one_peak(peak):
        global NUM_DOWN_SAMP
        mod_peak = peak
        mod_peak['prom'] *= 0.2 + (0.8 * peak['x']/NUM_DOWN_SAMP)
        return mod_peak
    return map(mod_one_peak, peaks)

import itertools
def remove_close_peaks(peaks, min_x_distance):
    for (peak_i, peak_j) in itertools.combinations(peaks, 2):
        if abs(peak_i['x'] - peak_j['x']) < min_x_distance:
            if peak_i['prom'] < peak_j['prom']:
                peak_i['prom'] = 0
            else:
                peak_j['prom'] = 0
    peaks = [peak for peak in peaks if peak['prom'] > 0 ]
    return peaks
    

def sub_build_up(subs, intensity_timeline, peak_x, min_time, max_time, threshold):
    # TODO
    return intensity_timeline

def add_sub(subs, string, start_s, dur_s):
    subs.append({'index': 0, 
                 'start_s': int(start_s),
                 'dur_s': int(dur_s),
                 'string': string})
    return subs

def urandf():
    return ord(os.urandom(1)[0])/256.0

def generate_subs(peaks, chance_to_cum):
    global SAMPLE_T
    subs = []
    peak_cum_x = max(peaks[0]['x'], peaks[1]['x'])
    if len(peaks) >= 2:
        if urandf() < chance_to_cum:
            # Then the former peak becomes the climax instead
            peak_cum_x = min(peaks[0]['x'], peaks[1]['x'])
        else:
            # The former peak will only be for edging and then moving on to the next
            peak_edge_x = min(peaks[0]['x'], peaks[1]['x'])
            # Adjust timestamp to start a little before peak
            peak_edge_x = max(0, peak_edge_x - 5/SAMPLE_T)
            sub_build_up(subs, peaks, peak_edge_x, 15, 45, 0.2)
            add_sub(subs, "Edge and then calm down \nand get ready for the next one.", 
                    peak_edge_x*SAMPLE_T, 6)
    peak_cum_x = max(0, peak_cum_x - 5/SAMPLE_T)

    sub_build_up(subs, peaks, peak_cum_x, 8, 25, 0.5)

    if urandf() < chance_to_cum:
        print "C at ", peak_cum_x * SAMPLE_T
        add_sub(subs, "Cum!", peak_cum_x * SAMPLE_T, 6)
    else:
        add_sub(subs, "Stop! Let go!", peak_cum_x*SAMPLE_T, 4);
        add_sub(subs, "No more touching yourself\nduring this video.", 
                peak_cum_x*SAMPLE_T + 4, 10);
    return subs

def seconds2timestamp(seconds):
    hours = seconds/3600
    minutes = (seconds - hours * 3600)/60
    seconds = (seconds - (hours * 3600 + minutes * 60))
    return (hours, minutes, seconds)

def export_sub(sub):
    
    (start_hours, start_minutes, start_seconds) = seconds2timestamp(sub['start_s'])
    end_s = sub['start_s'] + sub['dur_s']
    (end_hours, end_minutes, end_seconds) = seconds2timestamp(end_s)

    #if sub['index'] == 1: 
    #    print stderr "You may not fast-forward past XX:XX:XX" # TODO
    print("%d\n%02d:%02d:%02d,000 --> %02d:%02d:%02d,000\n%s\n\n" % ( 
          sub['index'], start_hours, start_minutes, start_seconds, 
          end_hours, end_minutes, end_seconds, 
          sub['string'] ))


def export_subs(subs):
    subs.sort(key = lambda sub: sub['start_s'])
    for i in range(0, len(subs)):
        subs[i]['index'] = i+1
        export_sub(subs[i])


msvcrt.setmode(sys.stdin.fileno(), os.O_BINARY)

#with open('../audiointensity/singlepeak.wav', 'rb') as infile:
#        p=Popen(['ffmpeg', '-i', '-', '-acodec', 'pcm_s16le', '-f','wav', '-'], stdin=infile , stdout=PIPE , stderr=PIP)E
#        fobj = io.BytesIO(p.stdout.read())
#wavein = wave.open(fobj, 'rb')

wf = io.BytesIO(sys.stdin.read())
wavein = wave.open(wf, 'rb');
(nchannels, sampwidth, samp_rate, nframes, comptype, compname) = wavein.getparams ()
frames = wavein.readframes (nframes * nchannels)
audio_samples = struct.unpack_from ("%dh" % nframes * nchannels, frames)
#print "Params:", wavein.getparams()
#print len(frames)
#print audio_samples[0:50]


TARGET_FS = 11
DOWN_SAMP_RATIO = samp_rate/TARGET_FS;
FILTER_LEN_1 = 2 * TARGET_FS
FILTER_LEN_2 = 10 * TARGET_FS
SAMPLE_T = float(DOWN_SAMP_RATIO)/samp_rate
N_MOST_PROM = 5
MIN_PEAK_DISTANCE = 30*TARGET_FS

chance_to_cum = 0.5
if len(sys.argv) > 1: # If optional param was passed
    chance_to_cum = max(0, min(int(sys.argv[1]), 100))/100.0

# bests = filter data with [1 0 0 0 -1]
bests = np.convolve([1, 0, 0, 0, -1], audio_samples, 'same')

# downsample bests (crudely)
bests = bests[range(0, len(bests), DOWN_SAMP_RATIO)]

NUM_DOWN_SAMP = len(bests)

# bestss = filter bests with [1 0 .... 0 -1] (filter_len1 apart)
FILTER_1 = [0]*FILTER_LEN_1
FILTER_1[0] = FILTER_1[-1] = 1
bestss = np.convolve(FILTER_1, bests, 'same')

# bestsss = filter bestss with [1 0 ... 0 -1] (filter_len2 apart)
FILTER_2 = [0]*FILTER_LEN_2
FILTER_2[0] = FILTER_2[-1] = 1
bestsss = np.convolve(FILTER_2, bestss, 'same')

# peaks = find_peaks(bestsss)
peaks = find_peaks(bestsss, 0, len(bestsss))

# peakprom = f(peakprom, peakx) # Weight prom by time, penalize early peaks
peaks = penalize_early_peaks(peaks)

# remove_close_peaks()
peaks = remove_close_peaks(peaks, MIN_PEAK_DISTANCE)

# select_most_prom()
#peaks = select_most_prom(peaks, N_MOST_PROM)
from heapq import nlargest
peaks = nlargest(N_MOST_PROM, peaks, lambda peak: peak['prom'])

print "top:", peaks

# subs = generate_subs()
subs = generate_subs(peaks, chance_to_cum)

# export_subs(subs)
export_subs(subs)

wavein.close()


