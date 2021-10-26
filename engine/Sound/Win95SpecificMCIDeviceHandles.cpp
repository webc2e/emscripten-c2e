#include "Win95SpecificMCIDeviceHandles.h"
#include "..\display\ErrorMessageHandler.h"

HANDLE Win95SpecificMCIDeviceHandles::ourBlockReadySemaphore;
signed short* Win95SpecificMCIDeviceHandles::ourLastBlock;

// ----------------------------------------------------------------------
// Method:		Constructor
// Description:	Opens the CD device ready for subsequent commands
// ----------------------------------------------------------------------
Win95SpecificMCIDeviceHandles::Win95SpecificMCIDeviceHandles()
{
	myCDTrackFrame = NULL;
	mySampleSize = 256;
	myWaveInHandle = NULL;
	myMixerHandle = NULL;
	myCDdevice = NULL;

}


Win95SpecificMCIDeviceHandles::~Win95SpecificMCIDeviceHandles()
{
	StopCD();
	CloseSoundRecordingBuffer();
	CloseCD();
	  
}

// ----------------------------------------------------------------------
// Method:		SoundCallback
//				see waveInProc for parameter details.
// Description:	Callback procedure for waveinopen.  Takes a sample of the
//				soundbuffer during recording.
// ----------------------------------------------------------------------
void CALLBACK Win95SpecificMCIDeviceHandles::SoundCallback(HWAVEIN lhwi, 
							UINT uMsg, 
							DWORD dwInstance, 
							DWORD dwParam1, 
							DWORD dwParam2)
{
  MMRESULT result;

  if(uMsg == WIM_DATA)
  {	      

	result = waveInUnprepareHeader(lhwi,(LPWAVEHDR)dwParam1,sizeof(WAVEHDR));
    if(result != MMSYSERR_NOERROR)
    {
//	ErrorMessageHandler::Show("cd_device_error", 
//								12, 
//								"Win95SpecificMCIDeviceHandles::SoundCallback");
      return;
    }

	ourLastBlock = ( signed short*) ((LPWAVEHDR)dwParam1)->lpData;

    ReleaseSemaphore(ourBlockReadySemaphore,1,NULL);
  }
}

// ----------------------------------------------------------------------
// Method:		OpenSoundBufferForRecording
// Arguments:	soundsoure - enumeration value specifying where the sound 
//							is coming from - e.g. CD
//				inFrequency - Sample rate in samples per second thar each
//							channel should be recorded.
//				sampleSize - size of the sample buffer taken on each request.
//				dspName,mixerName  - unused,  to specify specific devices
//								in the future if required.
//				
// Description:	Callback procedure for waveInopen.  Takes a sample of the
//				soundbuffer during recording.
// ----------------------------------------------------------------------
void Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording(SoundSource source, 
											  int inFrequency, 
											  int sampleSize,
											  char *dspName,
												char *mixerName) 
{
	mySampleSize = sampleSize;
  mySoundSource = source;
  myInFrequency = inFrequency;
  myMixer = mixerName;

  WAVEFORMATEX format;

  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = 2;
  format.nSamplesPerSec = inFrequency;
  format.nAvgBytesPerSec = inFrequency*4;
  format.nBlockAlign = 4;
  format.wBitsPerSample = 16;
  format.cbSize = 0;

/* use this code later to find a given device!!
  int numdevs =  waveInGetNumDevs();
  WAVEINCAPS wic;
  int j=0;
  for(j=0;j<numdevs;j++)
  {
    MMRESULT result = waveInGetDevCaps(j,&wic,sizeof(wic));
	if(result != MMSYSERR_NOERROR)
	{
	  error("Error querying device capabilities");
	}
  }
*/

  MMRESULT result = waveInOpen(&myWaveInHandle, 
								WAVE_MAPPER, 
								&format, 
								(DWORD) &Win95SpecificMCIDeviceHandles::SoundCallback, 
								NULL, 
								CALLBACK_FUNCTION);

  if(result != MMSYSERR_NOERROR)
  {
	if(result == MMSYSERR_ALLOCATED)
     ErrorMessageHandler::Show("cd_device_error", sidFailedToUnprepareRecordingSoundBuffer, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");
	if(result == MMSYSERR_BADDEVICEID)
      ErrorMessageHandler::Show("cd_device_error", sidWaveDeviceForRecordingInUse, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");
	if(result == MMSYSERR_NODRIVER)
        ErrorMessageHandler::Show("cd_device_error", sidBadIDForWaveDeviceForRecording, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");
	if(result == MMSYSERR_NOMEM)
       ErrorMessageHandler::Show("cd_device_error", sidNoDriverForWaveDeviceForRecording, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");
	if(result == WAVERR_BADFORMAT)
      ErrorMessageHandler::Show("cd_device_error", sidOutOfMemoryForWaveDeviceForRecording, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");
    return;
  }

  int i;
  for(i=0;i<NUMBUFFERS;i++)
  {
	unsigned short* tmpdata = new unsigned short[mySampleSize*2];
	memset(tmpdata,0,mySampleSize*2*2);
	(myBuffer[i]).lpData = (LPSTR) tmpdata;
    (myBuffer[i]).dwBufferLength = mySampleSize*2*2;	// number of BYTES
    (myBuffer[i]).dwFlags = 0;
  }

  for(i=0;i<NUMBUFFERS-1;i++)
  {
    result = waveInPrepareHeader(myWaveInHandle,&(myBuffer[i]),sizeof(WAVEHDR));
    if(result != MMSYSERR_NOERROR)
    {
       ErrorMessageHandler::Show("cd_device_error", sidBadWaveFormatForWaveDeviceForRecording, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");
      return;
    }

    result = waveInAddBuffer(myWaveInHandle,&(myBuffer[i]),sizeof(WAVEHDR));
    if(result != MMSYSERR_NOERROR)
    {
       ErrorMessageHandler::Show("cd_device_error", sidBadWaveFormatForWaveDeviceForRecording, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");

      return;
    }
  }

  myData = (WaveFormBufferType*) myBuffer[0].lpData;
  ourLastBlock = myData;
  myBufferCount=NUMBUFFERS-1;

  ourBlockReadySemaphore = CreateSemaphore(NULL,0,NUMBUFFERS,NULL);
  if (!ourBlockReadySemaphore)
     ErrorMessageHandler::Show("cd_device_error", sidFailedToCreateSemaphore , "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");


  result = waveInStart(myWaveInHandle);
  if(result != MMSYSERR_NOERROR)
  {
  	 ErrorMessageHandler::Show("cd_device_error", sidErrorPreparingBufferForRecording, "Win95SpecificMCIDeviceHandles::OpenSoundBufferForRecording");

  }
}

void Win95SpecificMCIDeviceHandles::CloseSoundRecordingBuffer()
{

	// stop streaming the music data
  waveInStop(myWaveInHandle);
  
  // release all buffers
  waveInReset(myWaveInHandle);

  CloseHandle(ourBlockReadySemaphore);

  // unlock and free all buffers
	for(int i=0;i<NUMBUFFERS;i++)
	{
		waveInUnprepareHeader(myWaveInHandle,(LPWAVEHDR)&myBuffer[i],sizeof(WAVEHDR));
		delete (myBuffer[i]).lpData;
		myBuffer[i].lpData = NULL;
	}

	waveInClose(myWaveInHandle);
	CloseHandle(myWaveInHandle);

	 myData = NULL;
}

// ----------------------------------------------------------------------
// Method:		GetNextFragment
// Arguments:	None
// Description:	Gets a sample of the current recording buffer.
//			
// ----------------------------------------------------------------------
int Win95SpecificMCIDeviceHandles::GetNextFragment(void)
{

  WaitForSingleObject(ourBlockReadySemaphore, INFINITE);

  myData = ourLastBlock;

  int result, i;

  i = 0;
 
    myBuffer[myBufferCount].dwBufferLength = mySampleSize*2*2;
    myBuffer[myBufferCount].dwFlags = 0;

    result = waveInPrepareHeader(myWaveInHandle,&(myBuffer[myBufferCount]),sizeof(WAVEHDR));
    if(result != MMSYSERR_NOERROR)
    {
       ErrorMessageHandler::Show("cd_device_error", sidFailedToStartRecording, "Win95SpecificMCIDeviceHandles::GetNextFragment");

    }

    result = waveInAddBuffer(myWaveInHandle,&(myBuffer[myBufferCount]),sizeof(WAVEHDR));
    if(result != MMSYSERR_NOERROR)
    {
      ErrorMessageHandler::Show("cd_device_error", sidFailedToPrepareBuffer, "Win95SpecificMCIDeviceHandles::GetNextFragment");
    }

    myBufferCount++;
    if(myBufferCount == NUMBUFFERS)
    {
      myBufferCount = 0;
    }

  
  return 0;
}

// ----------------------------------------------------------------------
// Method:		SetVolume
// Arguments:	None
// Description:	Gets a sample of the current recording buffer.
//			
// ----------------------------------------------------------------------
void Win95SpecificMCIDeviceHandles::SetVolume(double loudness) 
{
	//if (mixerOpen(&hmixer, 0, 0, 0, MIXER_OBJECTF_MIXER)
  //    != MMSYSERR_NOERROR) {
  //  warning("opening mixer");
  //  return;
  //}

  MIXERLINE ml;
  ml.cbStruct = sizeof(ml);
  ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
  if (mixerGetLineInfo(0, &ml,
       MIXER_OBJECTF_MIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)!= MMSYSERR_NOERROR)
			ErrorMessageHandler::Show("cd_device_error", sidFailedToAddBuffer, "Win95SpecificMCIDeviceHandles::SetVolume");

  MIXERLINECONTROLS mlc;
  MIXERCONTROL mc;
  mlc.cbStruct = sizeof(mlc);
  mlc.dwLineID = ml.dwLineID;
  mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
  mc.cbStruct = sizeof(mc);
  mlc.cControls = 1;
  mlc.cbmxctrl = sizeof(mc);
  mlc.pamxctrl = &mc;

  if (mixerGetLineControls(0, &mlc,
    MIXER_OBJECTF_MIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE)!= MMSYSERR_NOERROR)
   ErrorMessageHandler::Show("cd_device_error", sidFailedToAddBuffer, "Win95SpecificMCIDeviceHandles::SetVolume");

  MIXERCONTROLDETAILS d;
  d.cbStruct = sizeof(d);
  d.dwControlID = mc.dwControlID; 
  d.cChannels = 1;
  d.cMultipleItems = 0;
  d.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
  MIXERCONTROLDETAILS_UNSIGNED value;
  value.dwValue = mc.Bounds.dwMinimum +
            (DWORD)(loudness * (mc.Bounds.dwMaximum-mc.Bounds.dwMinimum));
  d.paDetails = &value;

  if (mixerSetControlDetails(0, &d, MIXER_OBJECTF_MIXER)!= MMSYSERR_NOERROR)
    ErrorMessageHandler::Show("cd_device_error", sidFailedToAddBuffer, "Win95SpecificMCIDeviceHandles::SetVolume");  

}

void Win95SpecificMCIDeviceHandles::OnGeneralCDError(std::string& str)
{
  //May want to print the message, but for now
  delete myCDTrackFrame;
  myCDTrackFrame = 0;
  myCDTrackCount = 0;

}


unsigned int Win95SpecificMCIDeviceHandles::WinMSFtoSynMSF(unsigned int WinMSF)
{
	return ((MCI_MSF_MINUTE(WinMSF) * 60 * CD_FRAMES) +
		   (MCI_MSF_SECOND(WinMSF) * CD_FRAMES) +
		   MCI_MSF_FRAME(WinMSF));
}


unsigned int Win95SpecificMCIDeviceHandles::SynMSFtoWinMSF(unsigned int SynMSF)
{
	return (SynMSF / (60*CD_FRAMES))
		+  ((SynMSF / CD_FRAMES % 60) << 8)
		+ ((SynMSF % CD_FRAMES) << 16);
}

// ----------------------------------------------------------------------
// Method:		OpenCD
// Arguments:	cdromName - unused intended to specify a particular cd rom
//							device to open.
// Description:	Open the default CDRom device.
//			
// ----------------------------------------------------------------------
bool Win95SpecificMCIDeviceHandles::OpenCD(char* cdromName)
{
// tbd: get correct CDrom names in config screen, specify the correct one in
// cdromName, use that one in devicetype.

  MCIERROR retval;
  MCI_OPEN_PARMS openstruct;

  myBugFixedPauseFlag = false;
  
  openstruct.dwCallback = NULL;
  openstruct.wDeviceID = NULL;
  openstruct.lpstrDeviceType = (LPSTR) MCI_DEVTYPE_CD_AUDIO;
  openstruct.lpstrElementName = NULL;
  openstruct.lpstrAlias = NULL;

  retval = mciSendCommand(myCDdevice,MCI_OPEN,MCI_WAIT|MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_SHAREABLE,(DWORD)&openstruct);
  if(retval)
  {
        ErrorMessageHandler::Show("cd_device_error", sidPleaseShutDownActiveCDPlayers, "Win95SpecificMCIDeviceHandles::SetVolume");  
        return false;
  }

  myCDdevice = openstruct.wDeviceID;


  MCI_SET_PARMS mciSetParms;
  mciSetParms.dwTimeFormat = MCI_FORMAT_MSF;
  
  retval = mciSendCommand(myCDdevice,MCI_SET,MCI_WAIT|MCI_SET_TIME_FORMAT,(DWORD)(LPVOID) &mciSetParms);
  if(retval)
  {
	 ErrorMessageHandler::Show("cd_device_error", sidPleaseShutDownActiveCDPlayers, "Win95SpecificMCIDeviceHandles::OpenCD");  
        return false;
  }


  return true;

}

// ----------------------------------------------------------------------
// Method:		CloseCD
// Arguments:	None
// Description:	Properly closes CDRom device.  You must release the CD rom
//				device when you have finished with it.
//			
// ----------------------------------------------------------------------
void Win95SpecificMCIDeviceHandles::CloseCD(void)
{
	
  MCIERROR retval;
  MCI_GENERIC_PARMS closestruct;

  myBugFixedPauseFlag = false;

  retval = mciSendCommand(myCDdevice,MCI_CLOSE,MCI_WAIT,(DWORD)&closestruct);
  if(retval)
  {
     ErrorMessageHandler::Show("cd_device_error", sidFailedToStopCDPlayer, "Win95SpecificMCIDeviceHandles::CloseCD");  
        return;
  }

  delete[] myCDTrackFrame;
  
}

bool Win95SpecificMCIDeviceHandles::GetTrackInfo(void)
{
	MCIERROR retval;
	MCI_STATUS_PARMS parms;

	delete myCDTrackFrame;
	myCDTrackFrame = 0;
	myCDTrackCount  = 0;

	parms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
                return false;
	}

	myCDTrackCount = parms.dwReturn;

	int i;
	myCDTrackFrame = new int[myCDTrackCount+1];

	DWORD startpos;
	DWORD length;

	parms.dwItem = MCI_STATUS_POSITION;
	parms.dwTrack = myCDTrackCount;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_TRACK|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
        return false;
	}
	startpos = WinMSFtoSynMSF(parms.dwReturn);

	parms.dwItem = MCI_STATUS_LENGTH;
	parms.dwTrack = myCDTrackCount;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_TRACK|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
                return false;
	}
	length = WinMSFtoSynMSF(parms.dwReturn);

	myCDTrackFrame[myCDTrackCount] = startpos+length-1;

	for(i=myCDTrackCount-1;i>=0;i--)
	{
		// determine if track is a data or audio track
		parms.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
		parms.dwTrack = i+1;
		retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_STATUS_ITEM|MCI_TRACK,(DWORD)&parms);
		
		if(retval)
		{
             return false;
		}

		DWORD tracktype = parms.dwReturn;

		if(tracktype != MCI_CDA_TRACK_AUDIO)
		{
			myCDTrackFrame[i] = myCDTrackFrame[i+1];
		}
		else
		{
			// get start position
			parms.dwItem = MCI_STATUS_POSITION;
			parms.dwTrack = i+1;
			retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_TRACK|MCI_STATUS_ITEM,(DWORD)&parms);
			if(retval)
			{
                return false;
			}
			startpos = WinMSFtoSynMSF(parms.dwReturn);
			myCDTrackFrame[i] = startpos;
		}
	}
	return true;
}

// ----------------------------------------------------------------------
// Method:		GetTrackCount
// Arguments:	None
// Description:	The number of tracks on the current CD.
//			
// ----------------------------------------------------------------------
int Win95SpecificMCIDeviceHandles::GetTrackCount(void)
{
  return myCDTrackCount;
}

// ----------------------------------------------------------------------
// Method:		GetTrackFrame
// Arguments:	track - cd track of which to find the playing position
//						 in milliseconds
// Description:	find the playing position in milliseconds of the specified
//				track.
//			
// ----------------------------------------------------------------------
int Win95SpecificMCIDeviceHandles::GetTrackFrame(int track)
{
  if (!myCDTrackFrame)
    return 0;
  else if (track <= 1 || track > myCDTrackCount+1)
    return myCDTrackFrame[0];
  else
    return myCDTrackFrame[track-1];
}

// ----------------------------------------------------------------------
// Method:		PlayCD
// Arguments:	frame - position in milliseconds of where to start playing.
//				endframe - position to stop playing, if this is set to -1
//				then the CD will play to its end.
// Description:	Play a CD track from the given position - see GetTrackFrame()
//				for obtaining positions.
//			
// ----------------------------------------------------------------------
bool Win95SpecificMCIDeviceHandles::PlayCD(int frame, int endFrame)
{
  MCIERROR retval;
  MCI_PLAY_PARMS parms;

  myBugFixedPauseFlag = false;

  if (frame < 1) frame = 1;
  if (endFrame < 1) 
	if (myCDTrackCount > 0)
	  endFrame = myCDTrackFrame[myCDTrackCount];
    else
	  endFrame = 1;

  myLastEndFrame = endFrame;

  StopCD();

  parms.dwCallback = NULL;
  parms.dwFrom = SynMSFtoWinMSF(frame);
  parms.dwTo = SynMSFtoWinMSF(endFrame);
  retval = mciSendCommand(myCDdevice,MCI_PLAY,MCI_NOTIFY|MCI_FROM|MCI_TO,(DWORD)&parms);
  if(retval)
  {
	  char buf[1000];
	  mciGetErrorString(
		   retval,        
		  buf,  
		  1000      
		);

        return false;
  }
  return true;
}

// ----------------------------------------------------------------------
// Method:		GetStatus
// Arguments:	track - number of the current track playing.
//				frames - current position in milliseconds.
//				state - what state the CD Player is in e.g. playing or eject.
// Description:	Get information about the current state of the CD player.
//			
//			
// ----------------------------------------------------------------------
bool Win95SpecificMCIDeviceHandles::GetStatus(int &track, int &frames, SymbolID &state)
{
	MCI_STATUS_PARMS parms;

	MCIERROR retval;  
	
	parms.dwItem = MCI_STATUS_MODE;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
        return false;
	}

	DWORD cdstate = parms.dwReturn;

	SymbolID oldState = state;

	switch(cdstate)
	{
		case MCI_MODE_PAUSE:
		  state = Pause;
		  break;
		case MCI_MODE_PLAY:
		  state = Play;
		  break;
		case MCI_MODE_STOP:
		  if(myBugFixedPauseFlag == true)
		  {
			  state = Pause;
		  }
		  else
		  {
			  state = Stop;
		  }
		  break;
		case MCI_MODE_OPEN:
		  state = Open;
		  return true;
		  break;
		case MCI_MODE_NOT_READY:
		case MCI_MODE_RECORD:
		case MCI_MODE_SEEK:
		default:
		  state = NoCD;
		  return false;
		  break;
	}

	parms.dwItem = MCI_STATUS_CURRENT_TRACK;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
       return false;
	}
	track = parms.dwReturn;

	parms.dwItem = MCI_STATUS_POSITION;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
        return false;
	}
	frames = WinMSFtoSynMSF(parms.dwReturn);
	if(myCDTrackFrame)
		frames-=myCDTrackFrame[track-1];		// track relative frames!!!

	if ((oldState == NoCD || oldState == Open) &&
	  (state == Pause || state == Play || state == Stop))
	{
		GetTrackInfo();
	}

	if (state != Play && state != Pause)
	{
		track = 1;
		frames = 0;
	}

	return true;
}

bool Win95SpecificMCIDeviceHandles::StopCD(void)
{

	MCI_GENERIC_PARMS parms;

  myBugFixedPauseFlag = false;
  
  MCIERROR retval = mciSendCommand(myCDdevice,MCI_STOP,MCI_WAIT,(DWORD)&parms);
  if(retval)
  {
        return false;
  }

  	return true;
}

bool Win95SpecificMCIDeviceHandles::PauseCD()
{

/*	turns out Windows standard CD-Rom drivers (MCICDA device) don't actually support pause 
    and resume. Sigh. I'll just fake a pause by doing a stop and remembering the current 
	position.

	MCI_GENERIC_PARMS parms;
	MCIERROR retval = mciSendCommand(cd_device,MCI_PAUSE,MCI_WAIT,(DWORD)&parms);
	if(retval)
	{
		char errorstr[256];
		mciGetErrorString(retval,errorstr,256);
		strcat(errorstr," (cdPause)");
                cdError(errorstr);
                return;
	}
*/

	MCI_STATUS_PARMS parms;
	MCIERROR retval;

	parms.dwItem = MCI_STATUS_POSITION;
	retval = mciSendCommand(myCDdevice,MCI_STATUS,MCI_WAIT|MCI_STATUS_ITEM,(DWORD)&parms);
	if(retval)
	{
       return false;
	}
	myPausedPosition = WinMSFtoSynMSF(parms.dwReturn);

	if(!StopCD())
		return false;

	myBugFixedPauseFlag = true;
	return true;
}


bool Win95SpecificMCIDeviceHandles::ResumeCD()
{
/* resume and pause are not supported by the standard MCICDA devices in Windows.

	MCI_GENERIC_PARMS parms;
	MCIERROR retval = mciSendCommand(cd_device,MCI_RESUME,MCI_WAIT,(DWORD)&parms);
	if(retval)
	{
		char errorstr[256];
		mciGetErrorString(retval,errorstr,256);
		strcat(errorstr," (cdResume)");
                cdError(errorstr);
                return;
	}
*/

	if(myBugFixedPauseFlag)
	{
		PlayCD(myPausedPosition,myLastEndFrame);
		myBugFixedPauseFlag = false;
	}
	else
	{
       return false;
	}

	return true;
}


bool  Win95SpecificMCIDeviceHandles::EjectCD()
{
  MCI_SET_PARMS parms;
  MCIERROR retval = mciSendCommand(myCDdevice,MCI_SET,MCI_WAIT|MCI_SET_DOOR_OPEN,(DWORD)&parms);
  if(retval)
  {
        return false;
  }
 
  myBugFixedPauseFlag = false;
  return true;
}


bool Win95SpecificMCIDeviceHandles::CloseCDTray()
{
  MCI_SET_PARMS parms;
  MCIERROR retval = mciSendCommand(myCDdevice,MCI_SET,MCI_WAIT|MCI_SET_DOOR_CLOSED,(DWORD)&parms);
  if(retval)
  {
  
        return false;
  }
  myBugFixedPauseFlag = false;
  return true;
}

