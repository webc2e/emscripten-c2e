// Filename:    SoundHandlers.h
// Class:       SoundHandlers
// Purpose:     Routines to implement sound commands/values in CAOS.
// Description:
//
// Usage:
//
// History: 31Mar99 Alima Created
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SoundHandlers.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../Sound/Soundlib.h"
#include "../App.h"
#include "../World.h"
#include "../Map/Map.h"
#include "../FilePath.h"
#include "../Sound/MusicGlobals.h"
#include "../Sound/MusicManager.h"

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "../../common/FileFuncs.h"


// Macro to make four-character constants
// Assumes little-endian by default.
#ifdef C2E_BIG_ENDIAN		// eg Mac
	#define Tok(a,b,c,d) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))
#else
	#define Tok(a,b,c,d) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a))
#endif

CDRomDevice SoundHandlers::ourCDRomDevice;


void SoundHandlers::Command_RMSC( CAOSMachine& vm )
{
	std::string str;
	int x,y;
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();
	
	vm.FetchStringRV( str );

	int roomID;
	if(theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}



void SoundHandlers::Command_MMSC( CAOSMachine& vm )
{
	std::string str;
	int x,y;

	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	vm.FetchStringRV( str );

	int roomID;
	if(theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetMetaRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void SoundHandlers::StringRV_RMSC( CAOSMachine& vm, std::string &str)
{
	int x,y;

	str="";
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().GetRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}



void SoundHandlers::StringRV_MMSC( CAOSMachine& vm, std::string &str)
{
	int x,y;

	str.empty();
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().GetMetaRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void SoundHandlers::Command_MIDI( CAOSMachine& vm )
{
	std::string str;

	vm.FetchStringRV( str );

	if (theSoundManager) {
		if(str.empty())
		{
		theSoundManager->StopMidiPlayer();
		}
		else
		{
		CheckSoundFileExists(vm, str + ".mid");
		theSoundManager->PlayMidiFile(str);	
		}
	}
}


void SoundHandlers::Command_RCLR( CAOSMachine& vm )
{
	int x,y;

	std::string str("none");
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}



void SoundHandlers::Command_MCLR( CAOSMachine& vm )
{
	int x,y;

	std::string str("none");
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetMetaRoomTrack(roomID,str);
	}
	else
	{
		
	vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void SoundHandlers::CheckSoundFileExists( CAOSMachine& vm, const std::string& file )
{
	FilePath filepath( file, SOUNDS_DIR );
	if (FileExists(filepath.GetFullPath().c_str()) == false)
		vm.ThrowRunError( CAOSMachine::sidSoundFileNotFound, file.c_str());
}

void SoundHandlers::Command_SNDE( CAOSMachine& vm )
{ 
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	CheckSoundFileExists(vm, str + ".wav");

	vm.GetTarg().GetAgentReference().SoundEffect(Tok( str[0], str[1], str[2], str[3] ));
	
}

void SoundHandlers::Command_SNDQ( CAOSMachine& vm )
{
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	int delay = vm.FetchIntegerRV();
	CheckSoundFileExists(vm, str + ".wav");

	vm.GetTarg().GetAgentReference().SoundEffect(Tok( str[0], str[1], str[2], str[3] ),delay);
}



void SoundHandlers::Command_SNDC( CAOSMachine& vm )
{ 
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	CheckSoundFileExists(vm, str + ".wav");
	vm.GetTarg().GetAgentReference().ControlledSound(Tok( str[0], str[1], str[2], str[3] ));
}

void SoundHandlers::Command_SNDL( CAOSMachine& vm )
{
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	CheckSoundFileExists(vm, str + ".wav");
	vm.GetTarg().GetAgentReference().ControlledSound(Tok( str[0], str[1], str[2], str[3] ),true);
}

void SoundHandlers::Command_STPC( CAOSMachine& vm )
{ 
	vm.ValidateTarg();

	vm.GetTarg().GetAgentReference().StopControlledSound();
}
	

void SoundHandlers::Command_FADE( CAOSMachine& vm ) 
{
		vm.ValidateTarg();

	vm.GetTarg().GetAgentReference().StopControlledSound(TRUE);
}


void SoundHandlers::Command_VOLM(CAOSMachine& vm )
{	
	int type = vm.FetchIntegerRV();
	int volume = vm.FetchIntegerRV();
	if (type == 0 && theSoundManager)
		theSoundManager->SetVolume(volume);
	if (type == 1 && theSoundManager)
		theSoundManager->SetVolumeOnMidiPlayer((volume + 10000) / 100); // range 0 to 100
	if (type == 2 && theMusicManager)
	{
		// Convert to a float 0 to 1
		float volumeFloat = (float)(volume + 10000) / 10000.0f;
		// std::cout << "Music volume goes to float " << volumeFloat << std::endl;
		theMusicManager->SetVolume(volumeFloat);
	}
}

int SoundHandlers::IntegerRV_VOLM(CAOSMachine& vm )
{	
	int type = vm.FetchIntegerRV();
	if (type == 0 && theSoundManager)
		return theSoundManager->GetVolume();
	if (type == 1 && theSoundManager)
	{
		int vol = theSoundManager->GetVolumeOnMidiPlayer();
		return vol * 100 - 10000;
	}
	if (type == 2 && theMusicManager)
	{
		// Convert to an integer from -10000 to 0
		float volume = theMusicManager->GetVolume();
		int volumeInt = (int)(volume * 10000.0f - 10000.0f);
		if (volumeInt > 0)
			volumeInt = 0;
		if (volumeInt < -10000)
			volumeInt = -10000;

		// CAOS assumes exact values, and we have a rounding error somewhere
		// on Intel Linux with GCC; this hack fixes it for Docking Station.
		if (volumeInt == -3999)
			volumeInt = -4000;
		return volumeInt;
	}

	return 0;
}

void SoundHandlers::Command_CD( CAOSMachine& vm )
{
	static CommandHandler HandlerTable[] =
	{
		SubCommand_CD_INIT,
		SubCommand_CD_PLAY,
		SubCommand_CD_STOP,
		SubCommand_CD_PAWS,
		SubCommand_CD_EJCT,
		SubCommand_CD_SHUT
	};

	int subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );

}

int SoundHandlers::IntegerRV_CD( CAOSMachine& vm )
{
	static IntegerRVHandler HandlerTable[] =
	{
		SubIntegerRV_CD_FRQL,
		SubIntegerRV_CD_FRQM,
		SubIntegerRV_CD_FRQH
	};

	int subcmd;
	subcmd = vm.FetchOp();
	return (HandlerTable[ subcmd ])( vm );

}

void SoundHandlers::SubCommand_CD_PLAY(CAOSMachine& vm )
{
	
	// in case the init command was forgotten
	if(!ourCDRomDevice.IsInitialised())
	{
		if(theSoundManager)
			theSoundManager->ReleaseAccessTotheSoundCard();
	
		if(theMusicSoundManager)
			theMusicSoundManager->ReleaseAccessTotheSoundCard();

		ourCDRomDevice.Initialise();
	}

	int firstTrack = vm.FetchIntegerRV();
	int endTrack = vm.FetchIntegerRV();

	ourCDRomDevice.PlayCD(	ourCDRomDevice.GetTrackFrame(firstTrack));
}


void SoundHandlers::SubCommand_CD_EJCT(CAOSMachine& vm )
{
	ourCDRomDevice.EjectCD();
}

void SoundHandlers::SubCommand_CD_STOP(CAOSMachine& vm )
{
	ourCDRomDevice.StopCD();
}

void SoundHandlers::SubCommand_CD_PAWS(CAOSMachine& vm )
{
	ourCDRomDevice.PauseCD();
}

int SoundHandlers::SubCommand_CD_TRKS(CAOSMachine& vm )
{
	return 	ourCDRomDevice.GetTrackCount();
}

int SoundHandlers::SubCommand_CD_TRAK(CAOSMachine& vm )
{
	int track =0;
	int frames =0;
	ourCDRomDevice.GetStatus(track,frames);
	return track;
}


int SoundHandlers::SubCommand_CD_POSN(CAOSMachine& vm )
{
	int track =0;
	int frames =0;

	ourCDRomDevice.GetStatus(track,frames);

	return frames;
}

int SoundHandlers::SubCommand_CD_TLEN(CAOSMachine& vm )
{
	int track = vm.FetchIntegerRV();

	if (track == -1)
	{
		int frames = 0;
	
		ourCDRomDevice.GetStatus(track,frames);
	}
	return 	ourCDRomDevice.GetTrackFrame(track);

}

void SoundHandlers::SubCommand_CD_INIT(CAOSMachine& vm)
{
	if(theSoundManager)
		theSoundManager->ReleaseAccessTotheSoundCard();
	
	if(theMusicSoundManager)
		theMusicSoundManager->ReleaseAccessTotheSoundCard();

	ourCDRomDevice.Initialise();
}

void SoundHandlers::SubCommand_CD_SHUT(CAOSMachine& vm)
{
	if(theSoundManager)
		theSoundManager->RestoreAccessTotheSoundCard();
	
	if(theMusicSoundManager)
		theMusicSoundManager->RestoreAccessTotheSoundCard();

	ourCDRomDevice.ShutDown();
}

int SoundHandlers::SubIntegerRV_CD_FRQL(CAOSMachine& vm )
{
	int low,med,high =0;

	ourCDRomDevice.GetCurrentCDFreqencies(low,med,high);
	return low;
}

int SoundHandlers::SubIntegerRV_CD_FRQM(CAOSMachine& vm )
{
	int low,med,high =0;

	ourCDRomDevice.GetCurrentCDFreqencies(low,med,high);
	return med;
}

int SoundHandlers::SubIntegerRV_CD_FRQH(CAOSMachine& vm )
{
	int low,med,high =0;

	ourCDRomDevice.GetCurrentCDFreqencies(low,med,high);
	return high;
}
	

