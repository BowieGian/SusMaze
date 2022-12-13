#ifndef GAME_AUDIO_H
#define GAME_AUDIO_H

#include <stdio.h>
#include <alsa/asoundlib.h>
#include "audioMixer.h"
#include "timer.h"

#define HITTING_BLOCK_VOICE_FILE "./source-file/soundfile/100052__menegass__gui-drum-bd-soft.wav"
#define PICK_UP_KEY_VOICE_FILE "./source-file/soundfile/pick_up_key.wav"
#define STEP_VOICE_FILE "./source-file/soundfile/step.wav"
#define KEYBOARD_VOICE_FILE "./source-file/soundfile/keyboard.wav"
#define UNLOCK_VOICE_FILE "./source-file/soundfile/unlock.wav"

void GameAudio_GenerateSounds();
void GameAduio_ClearSounds();

void GameAudio_HittingBlock();
void GameAudio_PickUpKey();
void GameAudio_MoveCharacter();
void GameAudio_Keyboard();
void GameAudio_Unlock();


#endif