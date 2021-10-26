// --------------------------------------------------------------------------
// Filename:	CDRomDevice.h
// Class:		CDRomDevice
// Purpose:		This class is a wrapper to the system's CDROM device.
//				and mixer.  It is possible to read the waveform buffer
//				and convert it to a frequency spectrum.
//
//				Sample sizes and frequencies are set here.
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

#ifdef C2E_SDL
	#include "stub/stub_CDRomDevice.h"
#endif

#ifndef C2E_CDROM_DEVICE_H
#define C2E_CDROM_DEVICE_H

// get rid of annoying compiler warnings
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif
#include <windows.h>
#include <string>


const int SampleFrequency = 22050;
const int SampleSize = 256;

const double  DDC_PI  = 3.14159265358979323846;





class Win95SpecificMCIDeviceHandles;

class CDRomDevice
{
public :
	CDRomDevice();
	virtual ~CDRomDevice();

//	enum SoundSource { SourceLine, SourceCD, SourcePipe };
	void Initialise();

	bool OpenCD(char *cdromName);

	void CloseCD(void);

	bool GetStatus(int &track, int &frames);

	bool PlayCD(int trackFrame, int endFrame=-1); 

	bool StopCD(void);

	bool PauseCD(void);

	bool ResumeCD(void);

	bool EjectCD(void);

	bool CloseCDTray(void);

	int GetTrackCount(void);

	int GetTrackFrame(int track);

	bool GetTrackInfo(void);

	void SetVolume(double loudness);

	int GetNextFragment(void);

	void GetCurrentCDFreqencies(int& low, int& med, int& high);

	void ShutDown();

	bool IsInitialised(){return myInitialisedProperlyFlag;}

private:

	void OnGeneralCDError(std::string& str);
	unsigned int WinMSFtoSynMSF(unsigned int WinMSF);
	unsigned int SynMSFtoWinMSF(unsigned int SynMSF);

	void FastFourierTransfer (
    unsigned  NumSamples,
    int       InverseTransform,
    float    *RealIn,
    float    *ImagIn,
    float    *RealOut,
    float    *ImagOut );

	double Index_to_frequency ( unsigned NumSamples, unsigned Index );
	unsigned ReverseBits ( unsigned index, unsigned NumBits );
	unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo );
	int IsPowerOfTwo ( unsigned x );

	// Copy constructor and assignment operator
	// Declared but not implemented
	CDRomDevice (const CDRomDevice&);
	CDRomDevice& operator= (const CDRomDevice&);

	Win95SpecificMCIDeviceHandles* mySpecificMCIDeviceHandles;

	bool myCDOpenFlag;
	
	bool myInitialisedProperlyFlag;
};

#endif // C2E_CDROM_DEVICE_H



