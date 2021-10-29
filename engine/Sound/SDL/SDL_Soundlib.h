#ifndef C2E_SDL
#error "this is the SDL version"
#endif

#ifndef SDL_SOUNDLIB_H
#define SDL_SOUNDLIB_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../stub/stub_CDRomDevice.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <list>
#include <map>

#define MAX_ACTIVE_SOUNDS 32

typedef enum {
  NO_SOUND_ERROR = 0,
  SOUNDCACHE_UNINITIALIESED,
  SOUNDCACHE_TOO_SMALL,
  SOUND_NOT_FOUND,
  SOUND_HANDLE_UNDEFINED,
  SOUND_CHANNELS_FULL,
  SOUND_MIXER_SUSPENDED,
  SOUND_MIXER_CANT_OPEN
} SOUNDERROR;

typedef int SOUNDHANDLE;

// Volumes range from -10000 (silence) to 0 (full volume)
const int SoundMinVolume = -10000;

// SoundManager - front end class for all sound routines
//
//
// The Sound Manager is a means of manipulating a sound cache.
// The user can call sounds to be played, leaving the
// manager to update the cache by cleaning out the oldest
// unused sounds.
//
// Sounds can be of two types - effects, or controlled sounds
//
// Controlled sounds can be looped or played  once only, and
// can be adjusted (panning, volume) while playing.  They
// may also be stopped at any point.
//
// Effects play once only, and cannot be adjusted once started.
// Additionally, effects may be delayed by a number of ticks.
// Delayed sounds are placed on a queue maintained by the manager.

// to play midi sounds
// class MidiModule;

class SoundManager : public PersistentObject {
  CREATURES_DECLARE_SERIAL(SoundManager)
public:
  //////////////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////////////
  class SoundException : public BasicException {
  public:
    SoundException(std::string what, uint16 line)
        : BasicException(what.c_str()), lineNumber(line) {
      ;
    }

    uint16 LineNumber() { return lineNumber; }

  private:
    uint16 lineNumber;
  };

  enum SIDText {
    sidFileNotFound = 0,
    sidNotAllFilesCouldBeMunged,
    sidUnknown,
    sidResourcesAlreadyInUse,
    sidWaveFormatNotSupported,
    sidInvalidParameter,
    sidNoAggregation,
    sidNotEnoughMemory,
    sidFailedToCreateDirectSoundObject,
    sidFailedToSetCooperativeLevel,
    sidPrimaryBufferNotCreated,
    sidPrimaryBufferCouldNotBeSetToNewFormat,
    sidFriendlyFailedToCreateDirectSoundObject,
  };

  // functions
public:
  // If construction fails (eg no sound card), the sound functions
  // can still be called - they will just return without action.
  // (Constructor throws exception upon failure).
  SoundManager();
  ~SoundManager();

  void StopAllSounds();

  // Called by on timer
  void Update();

  //  Is mixer running?
  bool SoundEnabled();

  //	Set size (K) of cache in bytes
  //  Flushes the existing cache
  SOUNDERROR InitializeCache(int size);

  //  Clears all stored sounds from
  //  The sound cache
  SOUNDERROR FlushCache();

  //  Sets the overall sound volume
  SOUNDERROR SetVolume(long volume);
  int GetVolume();

  //  Begin to fade all sounds (but
  //  leaves them "playing silently"
  //  SDL NOTE: fade is instant :-)
  SOUNDERROR FadeOut();

  //  Fade all sounds back in
  SOUNDERROR FadeIn();

  //  Stop the mixer playing
  //  (Use on KillFocus)
  SOUNDERROR SuspendMixer();

  //  Restart the mixer
  //  After it has been suspended
  SOUNDERROR RestoreMixer();

  //  Ensures a sound is loaded into the cache
  SOUNDERROR PreLoadSound(DWORD wave);

  //  Load and Play sound immediately or
  //  preload sound and queue it to be played
  //  after 'ticks' has elapsed
  SOUNDERROR PlaySoundEffect(DWORD wave, int ticks = 0, long volume = 0,
                             long pan = 0);

  //  Begins a controlled sound and returns its handle
  SOUNDERROR StartControlledSound(DWORD wave, SOUNDHANDLE &handle,
                                  long volume = 0, long pan = 0,
                                  BOOL looped = FALSE);

  SOUNDERROR UpdateControlledSound(SOUNDHANDLE handle, long volume, long pan);

  //  Stops the specified sound (handle is
  //  then no longer valid) Sound can be
  //  optionally faded out
  SOUNDERROR StopControlledSound(SOUNDHANDLE handle, BOOL fade = FALSE);

  //  Has the selected sound finished playing?
  BOOL FinishedControlledSound(SOUNDHANDLE handle);

  bool PlayMidiFile(std::string &fileName);

  void StopMidiPlayer();

  void SetVolumeOnMidiPlayer(int32 volume);
  int GetVolumeOnMidiPlayer();

  void MuteMidiPlayer(bool mute);

  void SetMNGFile(const std::string &mng);

  bool IsMixerFaded();

  void InitialiseCDRomDevice();
  CDRomDevice &GetCDRomDevice();
  void CloseCDRomDevice();
  void SetVolumeOnCDRomDevice(int volume);

  void ReleaseAccessTotheSoundCard();
  void RestoreAccessTotheSoundCard();

private:
  int GetOverallFaded();

  struct CachedSound {
    Mix_Chunk *chunk;
    int refcount;
    int staleness;
  };

  struct ActiveSound {
    int channel; // SDL_mixer channel

    CachedSound *wave; // 0=none
    bool controlled;
    int volume;
    // loops, fadeouts etc...
  };

  struct QueuedSoundItem {
    DWORD wave;
    int ticks;
    long vol;
    long pan;
  };

  // list of existing SoundManagers
  // Sample rate, channels, mixer callback etc shared by all running
  // soundmanagers.
  static std::list<SoundManager *> ourManagers;

  // to allocate each SoundManager a set of channels...
  static int ourChannelAlloc;

  bool myInitFlag; // false if this soundmanager disabled.
  FILE *myMungedFP;
  uint8 *myMungeBuf; // hack used for loading munged sounds
  int myMungeBufSize;

  // should probably store actual structs instead of ptrs...
  std::map<DWORD, CachedSound *> mySoundCache;

  // one ActiveSound struct for each SDL_mixer channel
  ActiveSound myActiveSounds[MAX_ACTIVE_SOUNDS];

  //
  std::list<QueuedSoundItem> mySoundQueue;

  int myOverallVolume;            // 0..128
  int myUnconvertedOverallVolume; // 0..128
  bool myFaded;

  CachedSound *EnsureInCache(DWORD wave);
  void StartSoundOneShot(CachedSound *s, int vol, int pan);
  int StartSoundControlled(CachedSound *s, int vol, int pan, BOOL looping);
  void UpdateSoundQueue();
  Mix_Chunk *LoadWAVFromMunged(DWORD wave);

  int ToSDLVolume(int ourvol);   // volume conversion
  int FromSDLVolume(int ourvol); // volume conversion

  CDRomDevice myStubCDRom;
};

#endif // SDL_SOUNDLIB_H
