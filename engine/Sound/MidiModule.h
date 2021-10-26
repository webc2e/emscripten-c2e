// --------------------------------------------------------------------------
// Filename:	MidiModule.h
// Class:		MidiModule
// Purpose:		This class  streams data from a disk MIDI file to a
//	           MIDI stream buffer for playback.
//				
//				
// History:
// ------- 
// 24Mar98	Alima		Created. 
//							
//
// --------------------------------------------------------------------------

#ifndef MIDI_MODULE_H
#define MIDI_MODULE_H


#ifdef C2E_SDL

#include "stub/stub_MidiModule.h"

#else



#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <dmusici.h>
#include <string>


#define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP, \
        MB_PRECOMPOSED, y, -1, x, _MAX_PATH );


class MidiModule
{
public :
	MidiModule();
	MidiModule(LPDIRECTSOUND directSoundObject,
						 HWND window);

	~MidiModule();


	bool PlayMidiFile(std::string& file);

	void StopPlaying();
	void SetVolume(long volume);
	long GetVolume();

	IDirectMusicSegment* LoadMidiSegment(WCHAR midiFileName[]);
	HRESULT FreeDirectMusic();
	void Mute(bool mute);
	
private:
	bool StartUp(LPDIRECTSOUND directSoundObject,
						 HWND window);
	IDirectMusicLoader* CreateLoader();
	IDirectMusicPerformance* CreatePerformance();
		
	IDirectMusicPerformance*	myPerformance;
	IDirectMusic*				myDirectMusic;
	IDirectMusicLoader*			myLoader;
	IDirectMusicSegment*		mySegment;
	std::string myCurrentFile;
	long myVolume;
	bool myMuteFlag;

};

#endif	// end of non-win32 redirection

#endif // MIDI_MODULE_H

