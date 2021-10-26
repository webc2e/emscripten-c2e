// --------------------------------------------------------------------------
// Filename:	Win95SpecificMCIDeviceHandles.h
// Class:		Win95SpecificMCIDeviceHandles
// Purpose:		This class allows access to the system's CDROM device.
//				and mixer under windows.
//
//				It is possible to read the waveform buffer
//				and convert it to a frequency spectrum.
//				TODO: Initially assumes that you only have one drive.
//					
//		  Windows95/NT sound, CD-ROM and audio mixer routines, 
 //		  (C) 1998 Nils Desle (ndesle@hotmail.com)		
//				
// History:
// ------- 
// 23Jun00	Alima		Created. 
//							
// --------------------------------------------------------------------------


#ifndef _WIN95SPECIFICMCIDEVICEHANDLES_H
#define _WIN95SPECIFICMCIDEVICEHANDLES_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <string>
#endif



typedef signed short WaveFormBufferType;

const int NUMBUFFERS = 5;
const int CD_FRAMES = 75;	/* wild guess, but it seems to work...?? */

	enum SymbolID 
	{
	  Bulb = 0, 
	  Speaker = 1, Play = 2, Pause = 3, Stop = 4, SkipFwd = 5, SkipBack = 6,
	  Handle = 7, Pointer = 8, Open = 9, NoCD = 10, Exit = 11,
	  Zero = 12, One = 13, Two = 14, Three = 15, Four = 16,
	  Five = 17, Six = 18, Seven = 19, Eight = 20, Nine = 21, Ten = 22,
	  Colon = 22, Slider = 23, Selector = 24,
	  SymbolCount = 25,
	};

class Win95SpecificMCIDeviceHandles
{
public:

	Win95SpecificMCIDeviceHandles();
	~Win95SpecificMCIDeviceHandles();

	static void CALLBACK SoundCallback(HWAVEIN lhwi, 
							UINT uMsg, 
							DWORD dwInstance, 
							DWORD dwParam1, 
							DWORD dwParam2);

	enum SIDText
	{
		sidFailedToUnprepareRecordingSoundBuffer=0,
		sidWaveDeviceForRecordingInUse,
		sidBadIDForWaveDeviceForRecording,
		sidNoDriverForWaveDeviceForRecording,
		sidOutOfMemoryForWaveDeviceForRecording,
		sidBadWaveFormatForWaveDeviceForRecording,
		sidErrorPreparingBufferForRecording,
		sidFailedToCreateSemaphore,
		sidFailedToStartRecording,
		sidFailedToPrepareBuffer,
		sidFailedToAddBuffer,
		sidFailedToSetMixerVolume,
		sidPleaseShutDownActiveCDPlayers,
		sidFailedToStopCDPlayer,
	};

public:
	enum SoundSource { SourceLine, SourceCD, SourcePipe };

	bool OpenCD(char *cdromName);

	void CloseCD(void);

	bool GetStatus(int &track, int &frames, SymbolID &state);

	bool PlayCD(int trackFrame, int endFrame=-1); 

	bool StopCD(void);

	bool PauseCD(void);

	bool ResumeCD(void);

	bool EjectCD(void);

	bool CloseCDTray(void);

	int GetTrackCount(void);

	int GetTrackFrame(int track);

	bool GetTrackInfo(void);

	void OpenSoundBufferForRecording(SoundSource sound, 
									int inFrequency,
									int sampleSize,
									char *dspName,
									char *mixerName);

	void CloseSoundRecordingBuffer();

	void SetVolume(double loudness);

	int GetNextFragment(void);


	void OnGeneralCDError(std::string& str);
	unsigned int WinMSFtoSynMSF(unsigned int WinMSF);
	unsigned int SynMSFtoWinMSF(unsigned int SynMSF);

	const WaveFormBufferType* GetData(){return myData;}

private:

	HWAVEIN myWaveInHandle;
	HMIXER myMixerHandle;
	WAVEHDR myBuffer[NUMBUFFERS];
	MCIDEVICEID myCDdevice;

	WaveFormBufferType* myData;
	SoundSource mySoundSource;
	int myInFrequency;
	int myDownFactor;
	int myWindowSize;
	int myPipeIn;
	int myDevice;

		unsigned short *myDataIn;
	char* myMixer;

	int myBufferCount;

	int myCDTrackCount; 
	int *myCDTrackFrame;

	int myLastEndFrame;
	int myPausedPosition;
	bool myBugFixedPauseFlag;

	int mySampleSize;
	static HANDLE ourBlockReadySemaphore;
	static signed short* ourLastBlock;
};


#endif //_WIN95SPECIFICMCIDEVICEHANDLES_H

