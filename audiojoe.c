/* Begin ==================== Configurable constants ======================== */ 

const int DEBUG = 0, 
  VERBOSE = 0;
		  
// If no command-line argument was passed then this probability is used	(0 to 1)  
const float default_chance_to_cum = 0.5f; 
// How often encouraging phrases are said during a build up (0 to 1)
const float ENCOURAGING_PHRASE_FREQUENCY = 0.2f; 

/* End ====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#ifdef _WIN32
 #include <io.h>
 #include <fcntl.h>
 #include <Windows.h>
 #include <wincrypt.h>
#endif

void read_wav_header(unsigned int *samp_rate, unsigned int *bits_per_samp,
		     unsigned int *num_samp);
int read_wav_data(int *data, unsigned int samp_rate,
		  unsigned int bits_per_samp, unsigned int num_samp);
int conv_bit_size(unsigned int in, int bps);

int findpeaks(signed long* v, int startind, int endind, int *peakx, int *peaky, int *peakprom);
void select_most_prom(int *peakprom, int *peakx, int *peaky, int n, int k);
void remove_close_peaks(int *peakx, int *peakprom, int npeaks, const int min_x_distance);

void gen_sub_peak(signed long *intensity_timeline, int *peakx, int npeaks);
void export_subs();

// Random number generator stuff
#ifdef _WIN32
HCRYPTPROV hCryptProv;
#endif
void init_rand() {
  srand(time(0)); // Fallback to crappy rand()
#ifdef _WIN32
  // Windows
  if (!CryptAcquireContext(&hCryptProv, NULL, NULL,
			   PROV_RSA_FULL, 0)) {
    printf("Error during CryptAcquireContext!\n");
  }
#else
  // Unix / POSIX
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  srandom(ts.tv_nsec ^ ts.tv_sec);
#endif
}
float randomf() {
  float r = rand(); // Fallback if below fails
	
#ifdef _WIN32
  long rand_buf;
  if (CryptGenRandom(hCryptProv, sizeof(rand_buf),
		     (BYTE *)&rand_buf))
    r = abs(rand_buf % 256)/256.0;
#else
  r = abs(random() % 256)/256.0;
#endif
  return r;				
}

#ifndef _WIN32
int max(int a, int b) {
  return a > b ? a : b;
}
int min(int a, int b) {
  return a < b ? a : b;
}
#endif
int filter(int *data, int i) {
  return abs(data[i] - data[i-4]);
}

// Sample time (inverse of sample frequency)
float SAMPLE_T = 1.0f; // Declared globally for convenience

float chance_to_cum;
int main(int argc, char **argv)
{
  const int TARGET_FS = 11;
  const int MIN_PEAK_DISTANCE = 30*TARGET_FS; // Minimum distance (in samples, 1*TARGET_FS = 1 second) between detected points
  const int N_MOST_PROM = 5; // Number of points to detect
  const int FILTER_LEN1 = 2*TARGET_FS;
  const int FILTER_LEN2 = 10*TARGET_FS;
  const int MAX_LAG = 64;
	
  // If the optional parameter chance_to_cum has been passed:
  if (argc > 0) {
    chance_to_cum = max(0, min(atoi(argv[1]), 100))/100.0f;
    if (VERBOSE) fprintf(stderr, "!Got chance: %f\n", chance_to_cum);
  } else {
    // Use default chance
    chance_to_cum = default_chance_to_cum;
  }
	
#ifdef _WIN32
  // Re-open stdin in binary mode
  _setmode(_fileno(stdin), _O_BINARY);
#else
  // Re-open stdin in binary mode
  freopen(NULL, "rb", stdin);
#endif
	
  // Initialize random number generator
  init_rand();
		
  unsigned int samp_rate, bits_per_samp, num_samp;
  read_wav_header(&samp_rate, &bits_per_samp, &num_samp);

  int *data = (int *) malloc(num_samp * sizeof(int));
  num_samp = read_wav_data(data, samp_rate, bits_per_samp, num_samp);
  if(DEBUG) 
    printf("samp_rate=[%d] bits_per_samp=[%d] num_samp=[%d]\n",
	   samp_rate, bits_per_samp, num_samp);
	
  const int DOWN_SAMP_RATIO = samp_rate/TARGET_FS;
  SAMPLE_T = (float)DOWN_SAMP_RATIO/samp_rate;
  const int NUM_DOWN_SAMP = num_samp/DOWN_SAMP_RATIO + 1;
  signed long bests[NUM_DOWN_SAMP];

  unsigned int i;	
  for (i = 4; i < num_samp; ++i) {
    if (i % DOWN_SAMP_RATIO <= 0) {
      bests[i/DOWN_SAMP_RATIO] = 0;
    }
		
    int p_hh = filter(data, i); // p_hh = filter([1 0 0 0 -1], 1, data);
    bests[i/DOWN_SAMP_RATIO] += p_hh;
    if (i % DOWN_SAMP_RATIO >= MAX_LAG+4) {
      bests[i/DOWN_SAMP_RATIO] -= filter(data, i - MAX_LAG);
    }
  }
  signed long bestss[NUM_DOWN_SAMP];
  int t = 0, prev_t;
  for (i = 0; i < NUM_DOWN_SAMP+FILTER_LEN1/2; i++) {
    prev_t = t;
    t = min(max(0, i-FILTER_LEN1/2), NUM_DOWN_SAMP-1);
    bestss[t] = bestss[prev_t];
    if (i < NUM_DOWN_SAMP)
      bestss[t] += bests[i];
    if(i >= FILTER_LEN1)
      bestss[t] -= bests[i - FILTER_LEN1]; 
  }
  signed long bestsss[NUM_DOWN_SAMP];
  t = 0;
  for (i = 0; i < NUM_DOWN_SAMP+FILTER_LEN2/2; i++) {
    prev_t = t;
    t = min(max(0, i-FILTER_LEN2/2), NUM_DOWN_SAMP-1);
    bestsss[t] = bestsss[prev_t];
    if (i < NUM_DOWN_SAMP)
      bestsss[t] += bestss[i]/FILTER_LEN1;
    if(i >= FILTER_LEN2)
      bestsss[t] -= bestss[i - FILTER_LEN2]/FILTER_LEN1; 
  }
	
  int peakx[NUM_DOWN_SAMP], peaky[NUM_DOWN_SAMP], peakprom[NUM_DOWN_SAMP];
  int npeaks = findpeaks(bestsss, 0, NUM_DOWN_SAMP-1, peakx, peaky, peakprom);
	
  // Modify peak weights by the time at which they occur. Penalize early peaks.
  for (i = 0; i < npeaks; i++) {
    peakprom[i] *= 0.2 + (0.8*(float)peakx[i]/NUM_DOWN_SAMP);
  }
	
  // Eliminate small peaks that are closer than MIN_PEAK_DISTANCE samples to a bigger one.
  remove_close_peaks(peakx, peakprom, npeaks, MIN_PEAK_DISTANCE);
		
  // Place the N_MOST_PROM biggest peaks first in the arrays
  select_most_prom(peakprom, peakx, peaky, npeaks, N_MOST_PROM);
	
  // Generate subtitles around the peaks
  gen_sub_peak(bestsss, peakx, npeaks);             
	
  export_subs();	
  return EXIT_SUCCESS;
}

void add_sub(char *string, int start_time_seconds, int duration_seconds);

// Generate subtitles for building up the intensity for a possible climax
void sub_build_up(signed long *intensity_timeline, int peak_x, int min_time, int max_time, float threshold) {
  const signed long peak_intensity = intensity_timeline[peak_x];
  int i;
  // Find point min_time to max_time seconds before highest peak where intensity is going above the threshold
  for(i = peak_x - min_time/SAMPLE_T; 
	
      i > max(0, peak_x - max_time/SAMPLE_T) 
	&& intensity_timeline[i] > threshold * peak_intensity; 
		 
      i--)
    { }
  const int buildup_start_x = i;
	
  // Text for initiating the build up
  add_sub("Now go faster!", buildup_start_x*SAMPLE_T -2, 2);
	
  // Count down and throw in some encouraging phrases
  const int countdown_seconds = (peak_x - buildup_start_x)*SAMPLE_T;
  int start_second = buildup_start_x*SAMPLE_T;
  for(i = countdown_seconds; i > 0; i--) {
    if (randomf() < ENCOURAGING_PHRASE_FREQUENCY && i < countdown_seconds && i > 2) {
      // Time for an encouraging phrase
      switch((int)(randomf()*2)) {
      case 0:
	add_sub("A little longer!", start_second, 1);
	break;
      case 1:
	add_sub("Keep going!", start_second, 1);
	break;
      default:
	break;
      }
    } else if (i <= 20) {
      // Just display the current countdown time
      char *str = (char*) malloc(256*sizeof(char));
      sprintf(str, "Only %d seconds left", i);
      add_sub(str, start_second, 1);
    }
    start_second += 1;
  }
}

void gen_sub_peak(signed long *intensity_timeline, int *peakx, int npeaks) {
  int peak_cum_x = max(peakx[0], peakx[1]); // The latter peak is the climax
  int peak_edge_x;
  if (npeaks >= 2) {
    if(randomf() < chance_to_cum) {
      // Then the former peak becomes the climax instead
      peak_cum_x = min(peakx[0], peakx[1]);
    } else {
      // The former peak will only be for edging and then moving on to the next
      peak_edge_x = min(peakx[0], peakx[1]);
      peak_edge_x = max(0, peak_edge_x -5/SAMPLE_T); // Adjust timestamp to start a little before peak
      sub_build_up(intensity_timeline, peak_edge_x, 15, 45, 0.2);
      add_sub("Edge and then calm down \nand get ready for the next one.", peak_edge_x*SAMPLE_T, 6);
    }
  }
	
  peak_cum_x = max(0, peak_cum_x -5/SAMPLE_T); // Adjust timestamp to start a little before peak
	
  sub_build_up(intensity_timeline, peak_cum_x, 8, 25, 0.5);
	
  if (randomf() < chance_to_cum) // Throw the dice...
    add_sub("Cum!", peak_cum_x*SAMPLE_T, 6);
  else {
    add_sub("Stop! Let go!", peak_cum_x*SAMPLE_T, 4);
    add_sub("No more touching yourself\nduring this video.", peak_cum_x*SAMPLE_T + 4, 10);
  }	
}

// Data structure for a subtitle (a piece of text that gets displayed on the screen during the video)
typedef struct {
  int index;
  int start_seconds;
  int duration_seconds;
  char* string;
} subtitle;

#define MAX_SUBTITLES 2048
// A global array of subtitles
subtitle subtitles[MAX_SUBTITLES];

int sub_i = 0;
// Add one subtitle to the global array
void add_sub(char *string, int start_time_seconds, int duration_seconds) {
  subtitles[sub_i] = (subtitle){-1, start_time_seconds, duration_seconds, string};
  if (sub_i < MAX_SUBTITLES-1)
    sub_i++;
}

// Time formatting functions
int seconds2timestamp_hours(int seconds) {
  return seconds/3600;
}
int seconds2timestamp_minutes(int seconds) {
  return (seconds - seconds2timestamp_hours(seconds)*3600)/60;
}
int seconds2timestamp_seconds(int seconds) {
  return (seconds - (seconds2timestamp_hours(seconds)*3600 + seconds2timestamp_minutes(seconds)*60));
}

// Calculate the timestamps for a subtitle and print its formatted infor to stdout
void export_sub(subtitle sub) {
	
  int start_hours   = seconds2timestamp_hours(sub.start_seconds);
  int start_minutes = seconds2timestamp_minutes(sub.start_seconds);
  int start_seconds = seconds2timestamp_seconds(sub.start_seconds);
	
  int end_time = sub.start_seconds + sub.duration_seconds;
  int end_hours   = seconds2timestamp_hours(end_time);
  int end_minutes = seconds2timestamp_minutes(end_time);
  int end_seconds = seconds2timestamp_seconds(end_time);
	
  if (sub.index == 1)
    fprintf(stderr, "You may not fast-forward past %02d:%02d:%02d", start_hours, start_minutes, start_seconds);
	
  //printf("%d, %d\n", start_time_seconds, end_time);
  printf("%d\n%02d:%02d:%02d,000 --> %02d:%02d:%02d,000\n%s\n\n", sub.index, start_hours, start_minutes, start_seconds, end_hours, end_minutes, end_seconds, sub.string);
	
  // TODO: De-allocate sub.string if it was allocated dynamically
  // No real need to do that right now because this is only run right before terminating anyways.
}

// Comparison function used for sorting the subtitles in export_subs()
int subtitle_comparator(const void * elem1, const void * elem2) {
  subtitle a = *((subtitle*)elem1);
  subtitle b = *((subtitle*)elem2);
  if (a.start_seconds > b.start_seconds) return  1;
  if (a.start_seconds < b.start_seconds) return -1;
  return 0;
}

// Sort and export all subtitles
void export_subs() {
  int i;
	
  // Sort the subtitles
  qsort (subtitles, sub_i, sizeof(*subtitles), subtitle_comparator);
	
  // Number the subtitles
  for(i = 0; i < sub_i; i++)
    subtitles[i].index = i+1;
	
  // Export the subtitles (to stdout, should be redirected to an .srt file)
  for(i = 0; i < sub_i; i++) {
    export_sub(subtitles[i]);
  }
}

// Place the k most prominent peaks first in the arrays (in-place)
void select_most_prom(int *peakprom, int *peakx, int *peaky, int n, int k) {
  int i, minIndex, minValue;
  for(i=0; i < k; i++) {
    minIndex = i;
    minValue = peakprom[i];
    int j;
    for(j = i+1; j < n; j++) {
      if (peakprom[j] > minValue) {
	minIndex = j;
	minValue = peakprom[j];
      }
    }
    //swap list[i] and list[minIndex]
    int temp = peakprom[i];
    peakprom[i] = peakprom[minIndex];
    peakprom[minIndex] = temp;
    temp = peakx[i];
    peakx[i] = peakx[minIndex];
    peakx[minIndex] = temp;
    temp = peaky[i];
    peaky[i] = peaky[minIndex];
    peaky[minIndex] = temp;
  }
}

// "Remove" peaks that are too close to another more prominent peak by setting their prominence to 0
void remove_close_peaks(int *peakx, int *peakprom, int npeaks, const int min_x_distance) {
  int i,j;
  for(i = 0; i < npeaks; i++) {
    for(j = 0; j < i; j++) {
      if (abs(peakx[i] - peakx[j]) < min_x_distance) {
	if(peakprom[i] < peakprom[j]) {
	  peakprom[i] = 0;
	  break;
	} else
	  peakprom[j] = 0;
      }
    }		
  }
}

// Return the index of the largest element with index between startind and endind in v.
int indexOfMax(int *v, int startind, int endind) {
  int maxEl;
  int is_init = 0;
  int i, index;
  for(i = startind; i <= endind; i++) {
    if (!is_init || v[i] > maxEl) {
      maxEl = v[i];
      index = i;
      is_init = 1;
    }
  }
  return index;
}
// Write x, y (height), and prominence of peaks found in the values of v.
int findpeaks(signed long* v, int startind, int endind, int *peakx, int *peaky, int *peakprom) {
  if (startind > endind) {
    if(DEBUG)printf("(in findpeaks()) Warning: startindex > endindex");
    return 0;
  }
  const int MAX_PEAKS = endind - startind + 1;
  if(DEBUG)printf("len:%d, ", MAX_PEAKS);
  int peaks_size = 0;
  int minlocs[MAX_PEAKS];
  int minlocs_size = 0;
  int i;
  for (i = startind+1; i < endind-1; i++) {
    if (v[i-1] < v[i] && v[i] >= v[i+1]) {
      if (minlocs_size == 0 && peaks_size == 0) {
	// Ensure that there's a minimum before all peaks
	minlocs[minlocs_size++] = v[startind]; 
      }
      peaky[peaks_size] = v[i];
      peakx[peaks_size] = i;
      peaks_size++;
    }
    if (v[i-1] > v[i] && v[i] <= v[i+1])
      minlocs[minlocs_size++] = v[i]; 
  }
  if (minlocs_size == peaks_size) {
    // Ensure that there's a minimum after all peaks
    minlocs[minlocs_size++] = v[endind];// Add the last point as minimum
  }
	
  int N = peaks_size;
  if(DEBUG) printf("peaks:%d\n", N);
  for(i = 0; i < N; i++)
    peakprom[i] = 0;
  int s;
  for(s = 0; s < N; s++) { 
    if(DEBUG)printf("\r%d", s);
    int e;
    for(e = s+1; e < N+1; e++) {
      // Current interval: [s, e]
      // Get the highest peak of current interval
      int h = indexOfMax(peaky, s, e);
      // Update prominence of the peak
      peakprom[h] = max(peakprom[h], peaky[h] - max(minlocs[s], minlocs[e]));
    }
  }
  if(DEBUG)printf("\n");
  return N;
}

// Read WAV header information (like in a .wav file) from stdin
void read_wav_header(unsigned int *samp_rate, unsigned int *bits_per_samp,
		     unsigned int *num_samp)
{
  unsigned char buf[256];

  /* ChunkID (RIFF for little-endian, RIFX for big-endian) */
  fread(buf, 1, 4, stdin);
  buf[4] = '\0';
  if (strcmp((char*)buf, "RIFF")) exit(EXIT_FAILURE);
  /* ChunkSize */
  fread(buf, 1, 4, stdin);
  /* Format */
  fread(buf, 1, 4, stdin);
  buf[4] = '\0';
  if (strcmp((char*)buf, "WAVE")) exit(EXIT_FAILURE);

  /* Subchunk1ID */
  fread(buf, 1, 4, stdin);
  buf[4] = '\0';
  if (strcmp((char*)buf, "fmt ")) exit(EXIT_FAILURE);
  /* Subchunk1Size (16 for PCM) */
  fread(buf, 1, 4, stdin);
  if (buf[0] != 16 || buf[1] || buf[2] || buf[3]) exit(EXIT_FAILURE);
  /* AudioFormat (PCM = 1, other values indicate compression) */
  fread(buf, 1, 2, stdin);
  if (buf[0] != 1 || buf[1]) exit(EXIT_FAILURE);
  /* NumChannels (Mono = 1, Stereo = 2, etc) */
  fread(buf, 1, 2, stdin);
  unsigned int num_ch = buf[0] + (buf[1] << 8);
  if (num_ch != 1) exit(EXIT_FAILURE);
  /* SampleRate (8000, 44100, etc) */
  fread(buf, 1, 4, stdin);
  *samp_rate = buf[0] + (buf[1] << 8) +
    (buf[2] << 16) + (buf[3] << 24);
  /* ByteRate (SampleRate * NumChannels * BitsPerSample / 8) */
  fread(buf, 1, 4, stdin);
  const unsigned int byte_rate = buf[0] + (buf[1] << 8) +
    (buf[2] << 16) + (buf[3] << 24);
  /* BlockAlign (NumChannels * BitsPerSample / 8) */
  fread(buf, 1, 2, stdin);
  const unsigned int block_align = buf[0] + (buf[1] << 8);
  /* BitsPerSample */
  fread(buf, 1, 2, stdin);
  *bits_per_samp = buf[0] + (buf[1] << 8);
  if (byte_rate != ((*samp_rate * num_ch * *bits_per_samp) >> 3))
    exit(EXIT_FAILURE);
  if (block_align != ((num_ch * *bits_per_samp) >> 3))
    exit(EXIT_FAILURE);

  /* Subchunk2ID */
  fread(buf, 1, 4, stdin);
  buf[4] = '\0';
  //printf("s%ss", buf);
  if (!strcmp((char*)buf, "LIST")) {
    fread(buf, 1, 4, stdin);
    const unsigned int list_chunk_length = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

    /* Skip LIST chunk */
    fread(buf, 1, list_chunk_length, stdin);
		
    /* Subchunk2ID */
    fread(buf, 1, 4, stdin);
    buf[4] = '\0';
    //printf("%d", buf);
  }
		
  if (strcmp((char*)buf, "data"))	exit(EXIT_FAILURE);
  /* Subchunk2Size (NumSamples * NumChannels * BitsPerSample / 8) */
  fread(buf, 1, 4, stdin);
  const unsigned int subchunk2_size = buf[0] + (buf[1] << 8) +
    (buf[2] << 16) + (buf[3] << 24);
  *num_samp = (subchunk2_size << 3) / (
				       num_ch * *bits_per_samp);
}

// Read WAV sound datan (like in a .wav file) from stdin and place it in data[]
int read_wav_data(int *data, unsigned int samp_rate,
		  unsigned int bits_per_samp, unsigned int num_samp)
{
  unsigned char buf;
  unsigned int i, j;
  for (i=0; i < num_samp; ++i) {
    if ((i % 1000) == 0) {
      fprintf(stderr, "\rMinutes decoded: %4.2f\r", i/samp_rate/60.0);
    }
    unsigned int tmp = 0;
    for (j=0; j != bits_per_samp; j+=8) {
      if(!fread(&buf, 1, 1, stdin)) { // stdin ended short of the given file size. Must be coming streaming from FFmpeg etc.
	num_samp = i; // Will also end the outer loop
	break;
      }
      tmp += buf << j;
    }
		
    data[i] = conv_bit_size(tmp, bits_per_samp);
  }
  fprintf(stderr, "\n");
  return num_samp;
}
// Convert unsigned to signed integers
int conv_bit_size(unsigned int in, int bps)
{
  const unsigned int max = (1 << (bps-1)) - 1;
  return in > max ? in - (max<<1) : in;
}
