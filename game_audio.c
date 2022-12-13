#include "game_audio.h"

static wavedata_t HittingBlockSound;
static wavedata_t PickUpKeySound;
static wavedata_t CharacterMoveSound;
static wavedata_t KeyBoardSound;
static wavedata_t UnlockSound;

void GameAudio_GenerateSounds()
{
    AudioMixer_readWaveFileIntoMemory(HITTING_BLOCK_VOICE_FILE, &HittingBlockSound);
    AudioMixer_readWaveFileIntoMemory(PICK_UP_KEY_VOICE_FILE, &PickUpKeySound);
    AudioMixer_readWaveFileIntoMemory(STEP_VOICE_FILE, &CharacterMoveSound);
    AudioMixer_readWaveFileIntoMemory(KEYBOARD_VOICE_FILE, &KeyBoardSound);
    AudioMixer_readWaveFileIntoMemory(UNLOCK_VOICE_FILE, &UnlockSound);

}

void GameAduio_ClearSounds()
{
    AudioMixer_freeWaveFileData(&HittingBlockSound);
    AudioMixer_freeWaveFileData(&PickUpKeySound);
    AudioMixer_freeWaveFileData(&CharacterMoveSound);
    AudioMixer_freeWaveFileData(&KeyBoardSound);
    AudioMixer_freeWaveFileData(&UnlockSound);

    HittingBlockSound.pData = NULL;
    PickUpKeySound.pData = NULL;
    CharacterMoveSound.pData = NULL;
    KeyBoardSound.pData = NULL;
    UnlockSound.pData = NULL;
}

void GameAudio_HittingBlock()
{
    AudioMixer_queueSound(&HittingBlockSound);
}


void GameAudio_PickUpKey()
{   
    AudioMixer_queueSound(&PickUpKeySound);
}


void GameAudio_MoveCharacter()
{
    AudioMixer_queueSound(&CharacterMoveSound);
}


void GameAudio_Keyboard()
{
    AudioMixer_queueSound(&KeyBoardSound);
}


void GameAudio_Unlock()
{
    AudioMixer_queueSound(&UnlockSound);
}