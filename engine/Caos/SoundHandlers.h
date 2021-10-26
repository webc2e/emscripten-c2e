// Filename:    SoundHandlers.h
// Class:       SoundHandlers
// Purpose:     Routines to implement sound commands/values in CAOS.
// Description:
//
// Usage:
//
// History: 31Mar99 Alima Created
// -------------------------------------------------------------------------


#ifndef SOUNDHANDLERS_H
#define SOUNDHANDLERS_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include <string>
#include "../Sound/CDRomDevice.h"


class CAOSMachine;

class SoundHandlers
{
public:
	// Commands
	static void Command_RMSC( CAOSMachine& vm );
	static void Command_MMSC( CAOSMachine& vm );
	static void Command_MIDI( CAOSMachine& vm );
	static void Command_RCLR( CAOSMachine& vm );
	static void Command_MCLR( CAOSMachine& vm );
	static void Command_SNDE( CAOSMachine& vm );
	static void Command_SNDQ( CAOSMachine& vm );
	static void Command_SNDC( CAOSMachine& vm );
	static void Command_SNDL( CAOSMachine& vm );
	static void Command_STPC( CAOSMachine& vm );
	static void Command_FADE( CAOSMachine& vm );
	static void Command_VOLM(CAOSMachine& vm );

	static void Command_CD( CAOSMachine& vm );

	// RValues
	static int IntegerRV_CD( CAOSMachine& vm );
	static int IntegerRV_VOLM(CAOSMachine& vm );

	// String r-values
	static void StringRV_MMSC( CAOSMachine& vm, std::string& str );
	static void StringRV_RMSC( CAOSMachine& vm, std::string& str );

private:
	// Helpers
	static void CheckSoundFileExists( CAOSMachine& vm, const std::string& file );
	static void SubCommand_CD_PLAY( CAOSMachine& vm );
	static void	SubCommand_CD_STOP( CAOSMachine& vm );
	static void	SubCommand_CD_PAWS( CAOSMachine& vm );
	static void	SubCommand_CD_EJCT( CAOSMachine& vm );

	static int SubCommand_CD_TRKS(CAOSMachine& vm );
	static int SubCommand_CD_TRAK(CAOSMachine& vm );
	static int SubCommand_CD_TLEN(CAOSMachine& vm );
	static int SubCommand_CD_POSN(CAOSMachine& vm );
	static void SubCommand_CD_INIT(CAOSMachine& vm );
	static void SubCommand_CD_SHUT(CAOSMachine& vm );

	static int	SubIntegerRV_CD_FRQL( CAOSMachine& vm );
	static int	SubIntegerRV_CD_FRQM( CAOSMachine& vm );
	static int	SubIntegerRV_CD_FRQH( CAOSMachine& vm );

	static CDRomDevice ourCDRomDevice;
};



#endif	// GENERALHANDLERS_H

