#include "defines.h"
#include <Windows.h>
#include <dsound.h>
#include <math.h>

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

global_variable LPDIRECTSOUNDBUFFER globalSecondaryBuffer;

struct win32_audio_player
{
    int samplesPerSecond;
    int toneHz;
    int16 toneVolume = 3000;
    uint32 runningSampleIndex;
    int period;
    int bytesPerSample;
    int bufferSize;
    bool isPlaying = false;
};

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if (DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        // TODO(gabriel): Double-check that this works on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // TODO(gabriel): DSBCAPS_GLOBALFOCUS?
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if (SUCCEEDED(Error))
                    {
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(gabriel): Diagnostic
                    }
                }
                else
                {
                    // TODO(gabriel): Diagnostic
                }
            }
            else
            {
                // TODO(gabriel): Diagnostic
            }

            // TODO(gabriel): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &globalSecondaryBuffer, 0);
            if (SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO(gabriel): Diagnostic
        }
    }
    else
    {
        // TODO(gabriel): Diagnostic
    }
}

internal void Win32WriteSquareWave(win32_audio_player *player, DWORD regionSize, void *regionStart)
{
    DWORD regionSampleCount = regionSize / player->bytesPerSample;
    int16 *sampleStart = (int16 *)regionStart;
    for (int i = 0; i < regionSampleCount; i++)
    {
        int16 sampleValue = ((player->runningSampleIndex / (player->period / 2)) % 2) ? player->toneVolume : -player->toneVolume;

        //each sample is [LEFT RIGHT] (16 bits each)
        *sampleStart++ = sampleValue;
        *sampleStart++ = sampleValue;
        ++player->runningSampleIndex;
    }
}

internal void Win32WriteSineWave(win32_audio_player *player, DWORD regionSize, void *regionStart)
{
    DWORD regionSampleCount = regionSize / player->bytesPerSample;
    int16 *sampleStart = (int16 *)regionStart;
    for (int i = 0; i < regionSampleCount; i++)
    {
        real32 t = 2.0f * Pi32 * ((real32)player->runningSampleIndex / (real32)player->period);
        int16 sampleValue = player->toneVolume * sinf(t);

        *sampleStart++ = sampleValue;
        *sampleStart++ = sampleValue;
        ++player->runningSampleIndex;
    }
}

internal void Win32PlaySound(win32_audio_player *player)
{
    DWORD playCursor;
    DWORD writeCursor;
    if (SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
    {
        DWORD startLockByte = (player->runningSampleIndex * player->bytesPerSample) % player->bufferSize;
        DWORD bytesToLock = 0;
        if (startLockByte == playCursor)
        {
            bytesToLock = 0;
        }
        else if (startLockByte > playCursor)
        {
            bytesToLock = player->bufferSize - bytesToLock;
            bytesToLock += playCursor;
        }
        else
        {
            bytesToLock = playCursor - startLockByte;
        }

        void *region1;
        void *region2;
        DWORD region1Size;
        DWORD region2Size;

        if (SUCCEEDED(globalSecondaryBuffer->Lock(
                startLockByte, bytesToLock,
                &region1, &region1Size,
                &region2, &region2Size,
                0)))
        {

            Win32WriteSineWave(player, region1Size, region1);
            Win32WriteSineWave(player, region2Size, region2);
            globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
        }

        if (!player->isPlaying)
        {
            globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            player->isPlaying = true;
        }
    }
}