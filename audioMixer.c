// Incomplete implementation of an audio mixer. Search for "REVISIT" to find things
// which are left as incomplete.
// Note: Generates low latency audio on BeagleBone Black; higher latency found on host.
#include "audioMixer.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> // needed for mixer

//#include "intervalTimer.h"


static snd_pcm_t *handle;

#define DEFAULT_VOLUME 80

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_SIZE (sizeof(short)) 			// bytes per sample
// Sample size note: This works for mono files because each sample ("frame') is 1 value.
// If using stereo files then a frame would be two samples.

//#define MAX_PLAYBACKBUFFER_SIZE SAMPLE_RATE/20

static unsigned long playbackBufferSize = 0;
static short *playbackBuffer = NULL;


// Currently active (waiting to be played) sound bites
#define MAX_SOUND_BITES 30
typedef struct {
	// A pointer to a previously allocated sound bite (wavedata_t struct).
	// Note that many different sound-bite slots could share the same pointer
	// (overlapping cymbal crashes, for example)
	wavedata_t *pSound;

	// The offset into the pData of pSound. Indicates how much of the
	// sound has already been played (and hence where to start playing next).
	int location;
} playbackSound_t;
static playbackSound_t soundBites[MAX_SOUND_BITES];

// Playback threading
void* playbackThread(void* arg);
static _Bool stopping = false;
static pthread_t playbackThreadId;
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

static int volume = DEFAULT_VOLUME;



void AudioMixer_init(void)
{
	AudioMixer_setVolume(DEFAULT_VOLUME);

	// Initialize the currently active sound-bites being played
	// REVISIT:- Implement this. Hint: set the pSound pointer to NULL for each
	//     sound bite.
	//printf("Init Slots \n");
	for(int i=0; i<MAX_SOUND_BITES; i++){
		soundBites[i].pSound = NULL;
	}
	//printf("Init Slots Done\n");

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));

	// Launch playback thread:
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);
}



// Client code must call AudioMixer_freeWaveFileData to free dynamically allocated data.
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound)
{
	assert(pSound);

	// The PCM data in a wave file starts after the header:
	const int PCM_DATA_OFFSET = 44;

	// Open the wave file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - PCM_DATA_OFFSET;
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Search to the start of the data in the file
	fseek(file, PCM_DATA_OFFSET, SEEK_SET);

	// Allocate space to hold all PCM data
	pSound->pData = malloc(sizeInBytes);
	if (pSound->pData == 0) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	// Read PCM data from wave file into memory
	int samplesRead = fread(pSound->pData, SAMPLE_SIZE, pSound->numSamples, file);
	if (samplesRead != pSound->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pSound->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}

	fclose(file);
}



void AudioMixer_freeWaveFileData(wavedata_t *pSound)
{
	pSound->numSamples = 0;
	free(pSound->pData);
	pSound->pData = NULL;
}



void AudioMixer_queueSound(wavedata_t *pSound)
{
	// Ensure we are only being asked to play "good" sounds:
	assert(pSound->numSamples > 0);
	assert(pSound->pData);

	bool SlotFound = false;
	int SlotIDX = MAX_SOUND_BITES-1;

	while(SlotIDX>=0 && !SlotFound){
			
		if(soundBites[SlotIDX].pSound == NULL){	//if the slot is empty

			// Pre-allocate a piece of memory and copy the data
			wavedata_t* preAllocate_pSsound = malloc(sizeof(*pSound));
			int SampleNums = pSound->numSamples;

			preAllocate_pSsound->pData = malloc(SampleNums * sizeof(pSound->pData));
			short* preAllocate_pSound_pData = preAllocate_pSsound->pData;
			short* pSound_pData = pSound->pData;

			for(int j=0; j<SampleNums; j++){
				preAllocate_pSound_pData[j] = pSound_pData[j];
			}
			preAllocate_pSsound->numSamples = SampleNums;
			SlotFound = true;

			pthread_mutex_lock(&audioMutex);
			{
				// let the pSound pointer pointes to the pre-allocate memory
				soundBites[SlotIDX].pSound = preAllocate_pSsound; 
			}
			pthread_mutex_unlock(&audioMutex);
		}
		SlotIDX --;
	}

	if(SlotFound == false){
		printf("No empty sound bite slot avaliable \n");
	}

}



void AudioMixer_cleanup(void)
{
	printf("Stopping audio...\n");

	// Stop the PCM generation thread
	stopping = true;
	pthread_join(playbackThreadId, NULL);

	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	//  in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	free(playbackBuffer);
	playbackBuffer = NULL;

	printf("Done stopping audio...\n");
	fflush(stdout);
}



int AudioMixer_getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}



// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void AudioMixer_setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *mixerHandle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    // const char *selem_name = "PCM";	// For ZEN cape
    const char *selem_name = "Speaker";	// For USB Audio

    snd_mixer_open(&mixerHandle, 0);
    snd_mixer_attach(mixerHandle, card);
    snd_mixer_selem_register(mixerHandle, NULL, NULL);
    snd_mixer_load(mixerHandle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(mixerHandle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(mixerHandle);
}




// Fill the playbackBuffer array with new PCM values to output.
//    playbackBuffer: buffer to fill with new PCM data from sound bites.
//    size: the number of values to store into playbackBuffer
static void fillPlaybackBuffer(short *playbackBuffer, int size)
{
	// clear playbackBuffer
	memset(playbackBuffer, 0, size*sizeof(*playbackBuffer));

	for(int i=0; i<MAX_SOUND_BITES; i++){
		
		wavedata_t* temp_pSound = soundBites[i].pSound;
		// if the slot is not empty
		if(temp_pSound!=NULL){

			pthread_mutex_lock(&audioMutex);
			{
				int currentLocation = soundBites[i].location;
				int totalNumSamples = temp_pSound->numSamples;
				short *temp_pSound_pData = temp_pSound->pData;

				// adding the Buffer
				int j=0;
				while(j<playbackBufferSize && currentLocation<totalNumSamples){

					int sum = playbackBuffer[j] + temp_pSound_pData[currentLocation];
					if(sum > SHRT_MAX){
						sum = SHRT_MAX;
					}
					else if(sum < SHRT_MIN){
						sum = SHRT_MIN;
					}
					playbackBuffer[j] = sum;

					j++;
					currentLocation ++;
				}

				// if the sound is fully played
				if(currentLocation >= totalNumSamples){
					//free pSoubdData
					AudioMixer_freeWaveFileData(temp_pSound);

					//free slots
					free(soundBites[i].pSound);
					soundBites[i].pSound = NULL;
					currentLocation = 0;
				}

				//update the location
				soundBites[i].location = currentLocation;	
			}
			pthread_mutex_unlock(&audioMutex);
		}
	}
}





void* playbackThread(void* arg)
{

	while (!stopping) {
		// Generate next block of audio
		//Interval_markInterval(INTERVAL_LOW_LEVEL_AUDIO);
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);

		// Output the audio
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);

		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n",
					frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && frames < playbackBufferSize) {
			printf("Short write (expected %li, wrote %li)\n",
					playbackBufferSize, frames);
		}
	}

	return NULL;
}