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

#ifndef C2E_CDROM_DEVICE_H
#define C2E_CDROM_DEVICE_H

// get rid of annoying compiler warnings
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif
#include <string>

//typedef signed short WaveFormBufferType;

const int SampleFrequency = 22050;
const int SampleSize = 256;

//const int CD_FRAMES = 75;	/* wild guess, but it seems to work...?? */
const double  DDC_PI  = 3.14159265358979323846;


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

	bool IsInitialised(){return false;}

	void ShutDown();

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

private:

};

#endif // C2E_CDROM_DEVICE_H



