#ifndef C2E_SDL
#error "this is the SDL version"
#else

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../Soundlib.h"

#include "../../App.h"
#include "../../DirectoryManager.h"
#include "../../FilePath.h"
#include "../../General.h" // for BuildFspNoCache


CREATURES_IMPLEMENT_SERIAL(SoundManager)

// static
std::list<SoundManager *> SoundManager::ourManagers;
int SoundManager::ourChannelAlloc = 0;

SoundManager::SoundManager()
    : myInitFlag(false), myMungedFP(0), myMungeBuf(0),
      myOverallVolume(128), // max volume
      myUnconvertedOverallVolume(0), myFaded(false) {
  if (ourManagers.empty()) {
    // Signed audio is supported by all cards so we use that
    if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0) {
      std::cerr << "  " << SDL_GetError() << std::endl;
      throw SoundException("Sound initialisation failed (Game "
                           "will run silently)",
                           __LINE__);
    }
  }

  // snarf some audio channels for this manager.
  int firstchannel = ourChannelAlloc;
  ourChannelAlloc += MAX_ACTIVE_SOUNDS;

  // should check return code? naaaa...
  int foo = Mix_AllocateChannels(firstchannel + MAX_ACTIVE_SOUNDS);

  ourManagers.push_back(this);

  // clear our channel structs
  int i;
  for (i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
    myActiveSounds[i].wave = 0;
    myActiveSounds[i].channel = firstchannel + i;
  }

  // Default MNG file
  SetMNGFile(theApp.GetDefaultMNG());

  myInitFlag = true;
}

SoundManager::~SoundManager() {
  // should do more cleanup here!!!

  if (myMungedFP)
    fclose(myMungedFP);

  delete myMungeBuf;
}

void SoundManager::StopAllSounds() {
  // shoud implement this!
  // stop the sound
  int i;
  for (i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
    if (myActiveSounds[i].wave) {
      Mix_HaltChannel(myActiveSounds[i].channel);
      myActiveSounds[i].wave->refcount--;
      myActiveSounds[i].wave = 0;
    }
  }
}

// Called by on timer
void SoundManager::Update() {
  if (!myInitFlag)
    return;

  UpdateSoundQueue();

  int i;
  for (i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
    if (myActiveSounds[i].wave) {
      // kill any oneshot sounds which have finished playing
      // (controlled sounds need to be explictly killed).
      if (!myActiveSounds[i].controlled) {
        if (!Mix_Playing(myActiveSounds[i].channel)) {
          // sound finished
          myActiveSounds[i].wave->refcount--;
          myActiveSounds[i].wave = 0;
        }
      }
    }
  }

  // update cache
  std::map<DWORD, CachedSound *>::iterator it, itnext;

  it = mySoundCache.begin();
  while (it != mySoundCache.end()) {
    itnext = it;
    ++itnext;

    // in-use sounds don't get stale
    if ((*it).second->refcount = 0) {
      if ((*it).second->staleness++ > 100) {
        Mix_FreeChunk((*it).second->chunk);
        delete (*it).second;
        mySoundCache.erase(it);
      }
    }
    it = itnext;
  }
}

//  Is mixer running?
bool SoundManager::SoundEnabled() { return true; }

//	Set size (K) of cache in bytes
//  Flushes the existing cache
SOUNDERROR SoundManager::InitializeCache(int size) {
  if (!myInitFlag)
    return NO_SOUND_ERROR;
  return NO_SOUND_ERROR;
}

//  Clears all stored sounds from
//  The sound cache
SOUNDERROR SoundManager::FlushCache() {
  if (!myInitFlag)
    return NO_SOUND_ERROR;
  return NO_SOUND_ERROR;
}

//  Sets the overall sound volume
SOUNDERROR SoundManager::SetVolume(long volume) {
  if (!myInitFlag)
    return NO_SOUND_ERROR;

  // Overall volume is store as an SDL volume, not a DX volume.
  myUnconvertedOverallVolume = volume;
  volume = ToSDLVolume(volume);
  myOverallVolume = volume;

  int i;
  for (i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
    if (myActiveSounds[i].wave) {
      int currentvol = Mix_Volume(myActiveSounds[i].channel, -1);
      // std::cout << "2 Mix SDL volume (SetVolume)" << (currentvol *
      // volume)/128 << std::endl;
      Mix_Volume(myActiveSounds[i].channel, (currentvol * volume) / 128);
    }
  }

  return NO_SOUND_ERROR;
}

//  Begin to fade all sounds (but
//  leaves them "playing silently"
SOUNDERROR SoundManager::FadeOut() {
  if (!myInitFlag)
    return NO_SOUND_ERROR;

  myFaded = true;
  return NO_SOUND_ERROR;
}

//  Fade all sounds back in
SOUNDERROR SoundManager::FadeIn() {
  if (!myInitFlag)
    return NO_SOUND_ERROR;
  myFaded = false;
  return NO_SOUND_ERROR;
}

//  Stop the mixer playing
//  (Use on KillFocus)
SOUNDERROR SoundManager::SuspendMixer() {
  if (!myInitFlag)
    return NO_SOUND_ERROR;
  return NO_SOUND_ERROR;
}

//  Restart the mixer
//  After it has been suspended
SOUNDERROR SoundManager::RestoreMixer() {
  if (!myInitFlag)
    return NO_SOUND_ERROR;
  return NO_SOUND_ERROR;
}

//  Ensures a sound is loaded into the cache
SOUNDERROR SoundManager::PreLoadSound(DWORD wave) {
  if (!myInitFlag)
    return NO_SOUND_ERROR;
  return NO_SOUND_ERROR;
}

//  Load and Play sound immediately or
//  preload sound and queue it to be played
//  after 'ticks' has elapsed
SOUNDERROR SoundManager::PlaySoundEffect(DWORD wave, int ticks, long volume,
                                         long pan) {
  if (!myInitFlag)
    return NO_SOUND_ERROR;

  if (ticks > 0) {
    // preload/sanity check...
    CachedSound *s = EnsureInCache(wave);
    if (!s)
      return SOUND_NOT_FOUND;

    // queue sound up for later...
    mySoundQueue.push_back(QueuedSoundItem());
    mySoundQueue.back().wave = wave;
    mySoundQueue.back().ticks = ticks;
    mySoundQueue.back().vol = volume;
    mySoundQueue.back().pan = pan;
    return NO_SOUND_ERROR;
  }

  CachedSound *s;
  s = EnsureInCache(wave);

  if (!s)
    return SOUND_NOT_FOUND;

  StartSoundOneShot(s, volume, pan);

  return NO_SOUND_ERROR;
}

//  Begins a controlled sound and returns its handle
SOUNDERROR SoundManager::StartControlledSound(DWORD wave, SOUNDHANDLE &handle,
                                              long volume, long pan,
                                              BOOL looped) {
  if (!myInitFlag)
    return NO_SOUND_ERROR;

  CachedSound *s = EnsureInCache(wave);

  if (!s)
    return SOUND_NOT_FOUND;

  int i = StartSoundControlled(s, volume, pan, looped);

  if (i < 0)
    return SOUND_CHANNELS_FULL;

  handle = (SOUNDHANDLE)i;

  return NO_SOUND_ERROR;
}

SOUNDERROR SoundManager::UpdateControlledSound(SOUNDHANDLE handle, long volume,
                                               long pan) {
  int i = (int)handle;

  // don't like this.
  if (i < 0 || i >= MAX_ACTIVE_SOUNDS)
    return SOUND_HANDLE_UNDEFINED;

  ASSERT(myActiveSounds[i].controlled);

  myActiveSounds[i].volume = volume;

  volume = ToSDLVolume(volume);
  // std::cout << "3 Mix SDL volume (UpdateControlledSound)" <<
  // (GetOverallFaded() * volume)/128 << std::endl;
  Mix_Volume(myActiveSounds[i].channel, (GetOverallFaded() * volume) / 128);
  return NO_SOUND_ERROR;
}

//  Stops the specified sound (handle is
//  then no longer valid) Sound can be
//  optionally faded out
SOUNDERROR SoundManager::StopControlledSound(SOUNDHANDLE handle, BOOL fade) {
  if (!myInitFlag)
    return NO_SOUND_ERROR;

  int i = (int)handle;

  // This is what the dsound version does.
  // I don't think it's so nice.
  if (i < 0 || i >= MAX_ACTIVE_SOUNDS)
    return SOUND_HANDLE_UNDEFINED;

  ASSERT(myActiveSounds[i].controlled);

  // stop the sound
  Mix_HaltChannel(myActiveSounds[i].channel);
  myActiveSounds[i].wave->refcount--;
  myActiveSounds[i].wave = 0;

  return NO_SOUND_ERROR;
}

//  Has the selected sound finished playing?
BOOL SoundManager::FinishedControlledSound(SOUNDHANDLE handle) {
  if (!myInitFlag)
    return true;

  int i = (int)handle;

  // this is what the dsound version does. ugh.
  if (i < 0 || i >= MAX_ACTIVE_SOUNDS)
    return true;

  if (Mix_Playing(myActiveSounds[i].channel))
    return false;
  else
    return true;
}

bool SoundManager::PlayMidiFile(std::string &fileName) {
  //	if( !myInitFlag )
  return false;
}

void SoundManager::StopMidiPlayer() {
  //	if( !myInitFlag )
}

void SoundManager::SetVolumeOnMidiPlayer(int32 volume) {
  //	if( !myInitFlag )
}

int SoundManager::GetVolumeOnMidiPlayer() { return 100; }

void SoundManager::MuteMidiPlayer(bool mute) {
  //	if( !myInitFlag )
}

// a little different to dsound version - here we open the file once and
// leave it open (dsound version opens and closes the file for each
// wave lookup).
void SoundManager::SetMNGFile(const std::string &mng) {
  if (myMungedFP)
    fclose(myMungedFP);

  FilePath filePath(mng, SOUNDS_DIR);
  std::string path = filePath.GetFullPath();

  myMungedFP = fopen(path.c_str(), "rb");
  if (!myMungedFP) {
    throw SoundException("SoundManager::SetMNGFile() failed", __LINE__);
  }
}

bool SoundManager::IsMixerFaded() { return myFaded; }

//----------------------------------------
// PRIVATE STUFF

SoundManager::CachedSound *SoundManager::EnsureInCache(DWORD wave) {
  std::map<DWORD, CachedSound *>::iterator it;

  // already in cache?
  it = mySoundCache.find(wave);
  if (it != mySoundCache.end())
    return (*it).second; // yep

  // nope - load it.
  CachedSound *s = new CachedSound;

  if (wave >= 0xff000000) {
    // it's a munged sound:
    s->chunk = LoadWAVFromMunged(wave);
    if (!s->chunk)
      return NULL;
  } else {
    // it's a standard .wav file:
    std::string wavFile = BuildFspNoCache(wave, "wav", SOUNDS_DIR);
    int siz = wavFile.size() - 1;
    if (siz < 0 || wavFile[siz] == '\\' || wavFile[siz] == '/') {
      // Sound file not found in BuildFspNoCache, a
      // directory was returned
      return NULL;
    }
    s->chunk = Mix_LoadWAV(wavFile.c_str());

    if (!s->chunk) {
      std::string error = "Couldn't load ";
      error += wavFile;
      std::cerr << error;
      return NULL;
    }
  }

  s->refcount = 0;
  s->staleness = 0;

  mySoundCache[wave] = s;
  return s;
}

// convert game (dsound?) style volume scale (-10000...0) to the SDL scale
// (0..128)
int SoundManager::ToSDLVolume(int ourvol) {
  int v = (ourvol + 10000);
  v = (v * 128) / 10000;
  if (v < 0)
    v = 0;
  else if (v > 128) // should be 127?
    v = 128;

  return v;
}

void SoundManager::StartSoundOneShot(SoundManager::CachedSound *s, int vol,
                                     int pan) {
  // find a free slot
  int i = 0;
  while (i < MAX_ACTIVE_SOUNDS) {
    if (!myActiveSounds[i].wave)
      break; // got one!
    ++i;
  }

  if (i >= MAX_ACTIVE_SOUNDS)
    return; // no free channels

  if (Mix_PlayChannel(myActiveSounds[i].channel, s->chunk, 0) < 0) {
    return;
  }

  int playvol = (ToSDLVolume(vol) * GetOverallFaded()) / 128;
  // std::cout << "4 Mix SDL volume (StartSoundOneShot)" << playvol <<
  // std::endl;
  Mix_Volume(myActiveSounds[i].channel, playvol);

  s->refcount++;
  s->staleness = 0;
  myActiveSounds[i].wave = s;
  myActiveSounds[i].controlled = false;
  myActiveSounds[i].volume = vol;
}

int SoundManager::StartSoundControlled(SoundManager::CachedSound *s, int vol,
                                       int pan, BOOL looping) {
  // find a free slot
  int i = 0;
  while (i < MAX_ACTIVE_SOUNDS) {
    if (!myActiveSounds[i].wave)
      break; // got one!
    ++i;
  }

  if (i >= MAX_ACTIVE_SOUNDS)
    return -1; // no free channels

  if (Mix_PlayChannel(myActiveSounds[i].channel, s->chunk, looping ? -1 : 0) <
      0) {
    return -1;
  }

  // std::cout << "Mix raw volume " << vol << std::endl;
  int playvol = (ToSDLVolume(vol) * GetOverallFaded()) / 128;
  // std::cout << "5 Mix SDL volume (StartSoundControlled)" << playvol <<
  // std::endl;
  Mix_Volume(myActiveSounds[i].channel, playvol);

  s->refcount++;
  s->staleness = 0;
  myActiveSounds[i].wave = s;
  myActiveSounds[i].controlled = true;
  myActiveSounds[i].volume = vol;

  return i;
}

void SoundManager::UpdateSoundQueue() {
  std::list<QueuedSoundItem>::iterator it, itnext;

  it = mySoundQueue.begin();
  while (it != mySoundQueue.end()) {
    itnext = it;
    ++itnext;

    if ((*it).ticks <= 0) {
      // time elapsed... go for it!
      CachedSound *s;
      s = EnsureInCache((*it).wave);

      if (s)
        StartSoundOneShot(s, (*it).vol, (*it).pan);

      mySoundQueue.erase(it);
    } else
      (*it).ticks--;

    it = itnext;
  }
}

// Munged sounds are all stored in the same file.
// They are basically the same format as .wav files, but are missing
// 16 bytes at the beginning. This causes a bit of a problem using the
// SDL_mixer load routines, so we do a bit of a hack and read the
// sound into a buffer, prepend the missing header and then use the standard
// SDL_mixer wave loader.
Mix_Chunk *SoundManager::LoadWAVFromMunged(DWORD wave) {
  uint32 index = wave - 0xff000000;

  if (!myMungedFP) {
    std::cerr << "Munged file pointer is NULL" << std::endl;
    return NULL;
  }

  // look it up in the table of sounds
  if (fseek(myMungedFP, (3 + index * 2) * sizeof(int32), SEEK_SET) < 0) {
    std::cerr << "Seek to munged index failed" << std::endl;
    return NULL;
  }

  int32 offset, size;
  fread(&offset, sizeof(int32), 1, myMungedFP);
  fread(&size, sizeof(int32), 1, myMungedFP);

  if (ferror(myMungedFP) != 0) {
    std::cerr << "Error reading munge index" << std::endl;
    return NULL;
  }

  // seek to start of wave
  if (fseek(myMungedFP, offset, SEEK_SET) < 0) {
    std::cerr << "Error seeking start of munge wave" << std::endl;
    return NULL;
  }

  // hack starts here :-)
  // alloc/grow read buffer
  if (myMungeBuf && myMungeBufSize < 16 + size) {
    delete[] myMungeBuf;
    myMungeBuf = NULL;
  }
  if (!myMungeBuf) {
    myMungeBuf = new uint8[16 + size];
    myMungeBufSize = 16 + size;
  }

  // fill in missing .wav header
  // (Pete left this out of the munged file to obsfucate the sounds)

  uint8 *p = myMungeBuf;
  *p++ = 'R';
  *p++ = 'I';
  *p++ = 'F';
  *p++ = 'F';

  *((uint32 *)p) = size - 8; // is this correct?
  p += 4;

  *p++ = 'W';
  *p++ = 'A';
  *p++ = 'V';
  *p++ = 'E';

  *p++ = 'f';
  *p++ = 'm';
  *p++ = 't';
  *p++ = ' ';

  if (fread(p, 1, size, myMungedFP) < 0) {
    std::cerr << "Reading final munge data failed" << std::endl;
    return NULL;
  }

  Mix_Chunk *mixChunk =
      Mix_LoadWAV_RW(SDL_RWFromMem(myMungeBuf, myMungeBufSize), 1);
  if (!mixChunk)
    std::cerr << "Call to Mix_LoadWAV_RW in munge wav load failed" << std::endl;
  return mixChunk;
}

void SoundManager::InitialiseCDRomDevice() {}

CDRomDevice &SoundManager::GetCDRomDevice() { return myStubCDRom; }

void SoundManager::CloseCDRomDevice() {}

void SoundManager::SetVolumeOnCDRomDevice(int volume) {}

void SoundManager::ReleaseAccessTotheSoundCard() {}

void SoundManager::RestoreAccessTotheSoundCard() {}

int SoundManager::GetVolume() { return myUnconvertedOverallVolume; }

int SoundManager::GetOverallFaded() { return myFaded ? 0 : myOverallVolume; }

extern "C" {
void SDL_ConvertEndian(SDL_AudioCVT *cvt, Uint16 format);
void SDL_ConvertSign(SDL_AudioCVT *cvt, Uint16 format);
void SDL_Convert8(SDL_AudioCVT *cvt, Uint16 format);
void SDL_Convert16LSB(SDL_AudioCVT *cvt, Uint16 format);
void SDL_Convert16MSB(SDL_AudioCVT *cvt, Uint16 format);
void SDL_ConvertMono(SDL_AudioCVT *cvt, Uint16 format);
void SDL_ConvertStereo(SDL_AudioCVT *cvt, Uint16 format);
void SDL_RateDIV2(SDL_AudioCVT *cvt, Uint16 format);
void SDL_RateMUL2(SDL_AudioCVT *cvt, Uint16 format);
void SDL_RateSLOW(SDL_AudioCVT *cvt, Uint16 format);
}

// Override of function in SDL, to apply patch to convert between
// all formats, not just powers of two.  The patch was from a message
// on the SDL mailing list:

// Date: Fri, 08 Jun 2001 16:04:51 -0700
// From: Michael Babcock <mbabcock@la.creatureshop.henson.com>
// Subject: [SDL] SDL_BuildAudioCVT() problems

// This override depends on link order - this object must be linked
// in before the SDL library.

/* Creates a set of audio filters to convert from one format to another.
   Returns -1 if the format conversion is not supported, or 1 if the
   audio filter is set up.
*/
// extern "C" int SDL_BuildAudioCVT(SDL_AudioCVT *cvt,
// 	Uint16 src_format, Uint8 src_channels, int src_rate,
// 	Uint16 dst_format, Uint8 dst_channels, int dst_rate)
// {
// 	/* Start off with no conversion necessary */
// 	cvt->needed = 0;
// 	cvt->filter_index = 0;
// 	cvt->filters[0] = NULL;
// 	cvt->len_mult = 1;
// 	cvt->len_ratio = 1.0;

// 	/* First filter:  Endian conversion from src to dst */
// 	if ( (src_format & 0x1000) != (dst_format & 0x1000)
// 	     && ((src_format & 0xff) != 8) ) {
// 		cvt->filters[cvt->filter_index++] = SDL_ConvertEndian;
// 	}

// 	/* Second filter: Sign conversion -- signed/unsigned */
// 	if ( (src_format & 0x8000) != (dst_format & 0x8000) ) {
// 		cvt->filters[cvt->filter_index++] = SDL_ConvertSign;
// 	}

// 	/* Next filter:  Convert 16 bit <--> 8 bit PCM */
// 	if ( (src_format & 0xFF) != (dst_format & 0xFF) ) {
// 		switch (dst_format&0x10FF) {
// 			case AUDIO_U8:
// 				cvt->filters[cvt->filter_index++] =
// 							 SDL_Convert8;
// 				cvt->len_ratio /= 2;
// 				break;
// 			case AUDIO_U16LSB:
// 				cvt->filters[cvt->filter_index++] =
// 							SDL_Convert16LSB;
// 				cvt->len_mult *= 2;
// 				cvt->len_ratio *= 2;
// 				break;
// 			case AUDIO_U16MSB:
// 				cvt->filters[cvt->filter_index++] =
// 							SDL_Convert16MSB;
// 				cvt->len_mult *= 2;
// 				cvt->len_ratio *= 2;
// 				break;
// 		}
// 	}

// 	/* Last filter:  Mono/Stereo conversion */
// 	if ( src_channels != dst_channels ) {
// 		while ( (src_channels*2) <= dst_channels ) {
// 			cvt->filters[cvt->filter_index++] =
// 						SDL_ConvertStereo;
// 			cvt->len_mult *= 2;
// 			src_channels *= 2;
// 			cvt->len_ratio *= 2;
// 		}
// 		/* This assumes that 4 channel audio is in the format:
// 		     Left {front/back} + Right {front/back}
// 		   so converting to L/R stereo works properly.
// 		 */
// 		while ( ((src_channels%2) == 0) &&
// 				((src_channels/2) >= dst_channels) ) {
// 			cvt->filters[cvt->filter_index++] =
// 						 SDL_ConvertMono;
// 			src_channels /= 2;
// 			cvt->len_ratio /= 2;
// 		}
// 		if ( src_channels != dst_channels ) {
// 			/* Uh oh.. */;
// 		}
// 	}

// 	/* Do rate conversion */
// 	cvt->rate_incr = 0.0;
// 	if ( src_rate != dst_rate ) {
// 		Uint32 hi_rate, lo_rate;
// 		int len_mult;
// 		double len_ratio;
// 		void (*rate_cvt)(SDL_AudioCVT *cvt, Uint16 format);

// 		if ( src_rate > dst_rate ) {
// 			hi_rate = src_rate;
// 			lo_rate = dst_rate;
// 			rate_cvt = SDL_RateDIV2;
// 			len_mult = 1;
// 			len_ratio = 0.5;
// 		} else {
// 			hi_rate = dst_rate;
// 			lo_rate = src_rate;
// 			rate_cvt = SDL_RateMUL2;
// 			len_mult = 2;
// 			len_ratio = 2.0;
// 		}
// 		/* If hi_rate = lo_rate*2^x then conversion is easy */
// 		while ( (lo_rate*2) <= hi_rate ) {
// 			cvt->filters[cvt->filter_index++] = rate_cvt;
// 			cvt->len_mult *= len_mult;
// 			lo_rate *= 2;
// 			cvt->len_ratio *= len_ratio;
// 		}
// 		/* We may need a slow conversion here to finish up */
// 		if ( lo_rate != hi_rate ) {
// 			if ( src_rate < dst_rate ) {
// 				cvt->rate_incr = (double)lo_rate/hi_rate;
// 				cvt->len_mult *= 2;
// 				cvt->len_ratio /= cvt->rate_incr;
// 			} else {
// 				cvt->rate_incr = (double)hi_rate/lo_rate;
// 				cvt->len_ratio *= cvt->rate_incr;
// 			}
// 			cvt->filters[cvt->filter_index++] = SDL_RateSLOW;
// 		}
// 	}

// 	/* Set up the filter information */
// 	if ( cvt->filter_index != 0 ) {
// 		cvt->needed = 1;
// 		cvt->src_format = src_format;
// 		cvt->dst_format = dst_format;
// 		cvt->len = 0;
// 		cvt->buf = NULL;
// 		cvt->filters[cvt->filter_index] = NULL;
// 	}
// 	return(cvt->needed);
// }

#endif // SDL_SOUNDLIB_H
