// -------------------------------------------------------------------------
// Filename:    AgentHandlers.h
// Class:       AgentHandlers
// Purpose:     Routines to implement agent-related commands/values in CAOS.
// Description:
//
// Usage:
//
// History:
// 15Dec98  BenC	Initial version
// -------------------------------------------------------------------------

#ifndef AGENTHANDLERS_H
#define AGENTHANDLERS_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../Agents/AgentHandle.h"
#include <string>
#include "../FilePath.h"

class CAOSMachine;
class CAOSVar;

class AgentHandlers {

public:
  static void Command_DCOR(CAOSMachine &vm);
  static int IntegerRV_TCOR(CAOSMachine &vm);
  static void Command_CORE(CAOSMachine &vm);

  static void Command_VARC(CAOSMachine &vm);
  static float FloatRV_VARC(CAOSMachine &vm);
  static void Command_ROTN(CAOSMachine &vm);

  static float FloatRV_AVEL(CAOSMachine &vm);
  static void Command_AVEL(CAOSMachine &vm);
  static float FloatRV_FVEL(CAOSMachine &vm);
  static void Command_FVEL(CAOSMachine &vm);
  static float FloatRV_SVEL(CAOSMachine &vm);
  static void Command_SVEL(CAOSMachine &vm);
  static float FloatRV_SPIN(CAOSMachine &vm);
  static void Command_SPIN(CAOSMachine &vm);
  static float FloatRV_ANGL(CAOSMachine &vm);

  static float FloatRV_VECX(CAOSMachine &vm);
  static float FloatRV_VECY(CAOSMachine &vm);
  static float FloatRV_ADMP(CAOSMachine &vm);
  static void Command_ADMP(CAOSMachine &vm);
  static float FloatRV_FDMP(CAOSMachine &vm);
  static void Command_FDMP(CAOSMachine &vm);
  static float FloatRV_SDMP(CAOSMachine &vm);
  static void Command_SDMP(CAOSMachine &vm);

  // Commands
  static void Command_CALL(CAOSMachine &vm);
  static void Command_NEW(CAOSMachine &vm);
  static void Command_KILL(CAOSMachine &vm);
  static void Command_BASE(CAOSMachine &vm);
  static void Command_POSE(CAOSMachine &vm);
  static void Command_ANIM(CAOSMachine &vm);
  static void Command_OVER(CAOSMachine &vm);
  static void Command_MVTO(CAOSMachine &vm);
  static void Command_MVSF(CAOSMachine &vm);
  static void Command_MVBY(CAOSMachine &vm);
  static void Command_TICK(CAOSMachine &vm);
  static void Command_MESG(CAOSMachine &vm);
  static void Command_PART(CAOSMachine &vm);
  static void Command_GPAS(CAOSMachine &vm);
  static void Command_DPAS(CAOSMachine &vm);
  static void Command_CABN(CAOSMachine &vm);
  static void Command_CLAC(CAOSMachine &vm);
  static void Command_FRIC(CAOSMachine &vm);
  static void Command_PERM(CAOSMachine &vm);
  static void Command_ATTR(CAOSMachine &vm);
  static void Command_IMSK(CAOSMachine &vm);
  static void Command_ACCG(CAOSMachine &vm);
  static void Command_ELAS(CAOSMachine &vm);
  static void Command_AERO(CAOSMachine &vm);
  static void Command_RTAR(CAOSMachine &vm);
  static void Command_STAR(CAOSMachine &vm);
  static void Command_TTAR(CAOSMachine &vm);
  static void Command_PLNE(CAOSMachine &vm);
  static void Command_RNGE(CAOSMachine &vm);
  static void Command_VELO(CAOSMachine &vm);
  static void Command_SPAS(CAOSMachine &vm);
  static void Command_RPAS(CAOSMachine &vm);
  static void Command_PUPT(CAOSMachine &vm);
  static void Command_PUHL(CAOSMachine &vm);
  static void Command_EPAS(CAOSMachine &vm);
  static void Command_CABP(CAOSMachine &vm);
  static void Command_GALL(CAOSMachine &vm);
  static void Command_BHVR(CAOSMachine &vm);
  static void Command_FLTO(CAOSMachine &vm);
  static void Command_CLIK(CAOSMachine &vm);
  static void Command_MOUS(CAOSMachine &vm);
  static void Command_PURE(CAOSMachine &vm);
  static void Command_PAUS(CAOSMachine &vm);
  static void Command_STPT(CAOSMachine &vm);
  static void Command_ANMS(CAOSMachine &vm);
  static void Command_FRAT(CAOSMachine &vm);
  static void Command_FREL(CAOSMachine &vm);
  static void Command_SHOW(CAOSMachine &vm);
  static void Command_VOIS(CAOSMachine &vm);
  static void Command_VOIC(CAOSMachine &vm);
  static void Command_SEZZ(CAOSMachine &vm);
  static void Command_NEWC(CAOSMachine &vm);
  static void Command_CABW(CAOSMachine &vm);
  static void Command_MIRA(CAOSMachine &vm);
  static void Command_FLIP(CAOSMachine &vm);
  static void Command_ROTA(CAOSMachine &vm);
  static void Command_CABV(CAOSMachine &vm);
  static void Command_HAND(CAOSMachine &vm);
  static void Command_DROP(CAOSMachine &vm);
  static void Command_OUTL(CAOSMachine &vm);
  static void Command_UCLN(CAOSMachine &vm);

  static void Command_BLCK(CAOSMachine &vm);

  static void Command_DELN(CAOSMachine &vm);
  static void Command_NAMN(CAOSMachine &vm);

  // Integer RValues
  static int IntegerRV_FALL(CAOSMachine &vm);
  static int IntegerRV_SPCS(CAOSMachine &vm);
  static int IntegerRV_FMLY(CAOSMachine &vm);
  static int IntegerRV_GNUS(CAOSMachine &vm);
  static int IntegerRV_UNID(CAOSMachine &vm);
  static int IntegerRV_TOUC(CAOSMachine &vm);
  static int IntegerRV_SEEE(CAOSMachine &vm);
  static int IntegerRV_TOTL(CAOSMachine &vm);
  static int IntegerRV_TICK(CAOSMachine &vm);
  static int IntegerRV_TMVT(CAOSMachine &vm);
  static int IntegerRV_TMVB(CAOSMachine &vm);
  static int IntegerRV_TMVF(CAOSMachine &vm);
  static int IntegerRV_VISI(CAOSMachine &vm);
  static int IntegerRV_TYPE(CAOSMachine &vm);

  static int IntegerRV_POSE(CAOSMachine &vm);
  static int IntegerRV_MOWS(CAOSMachine &vm);
  static int IntegerRV_MIRA(CAOSMachine &vm);
  static int IntegerRV_FLIP(CAOSMachine &vm);
  static int IntegerRV_ROTA(CAOSMachine &vm);

  static float FloatRV_POSX(CAOSMachine &vm);
  static float FloatRV_POSY(CAOSMachine &vm);
  static float FloatRV_POSL(CAOSMachine &vm);
  static float FloatRV_POSR(CAOSMachine &vm);
  static float FloatRV_POST(CAOSMachine &vm);
  static float FloatRV_POSB(CAOSMachine &vm);
  static float FloatRV_DISQ(CAOSMachine &vm);

  static int IntegerRV_WDTH(CAOSMachine &vm);
  static int IntegerRV_HGHT(CAOSMachine &vm);

  static float FloatRV_OBST(CAOSMachine &vm);
  static int IntegerRV_WALL(CAOSMachine &vm);
  static int IntegerRV_MOVS(CAOSMachine &vm);

  static float FloatRV_RELX(CAOSMachine &vm);
  static float FloatRV_RELY(CAOSMachine &vm);

  static int IntegerRV_ELAS(CAOSMachine &vm);
  static int IntegerRV_AERO(CAOSMachine &vm);
  static int IntegerRV_PERM(CAOSMachine &vm);
  static int IntegerRV_ATTR(CAOSMachine &vm);
  static int IntegerRV_IMSK(CAOSMachine &vm);
  static int IntegerRV_FRIC(CAOSMachine &vm);
  static int IntegerRV_PLNE(CAOSMachine &vm);
  static int IntegerRV_PUPT(CAOSMachine &vm);
  static int IntegerRV_PUHL(CAOSMachine &vm);

  static int IntegerRV_CABP(CAOSMachine &vm);
  static int IntegerRV_CABL(CAOSMachine &vm);
  static int IntegerRV_CABR(CAOSMachine &vm);
  static int IntegerRV_CABB(CAOSMachine &vm);
  static int IntegerRV_CABT(CAOSMachine &vm);
  static int IntegerRV_BHVR(CAOSMachine &vm);
  static int IntegerRV_CABW(CAOSMachine &vm);
  static int IntegerRV_CABV(CAOSMachine &vm);

  static int IntegerRV_BASE(CAOSMachine &vm);
  static int IntegerRV_PURE(CAOSMachine &vm);
  static int IntegerRV_PAUS(CAOSMachine &vm);

  static int IntegerRV_CLIK(CAOSMachine &vm);
  static int IntegerRV_CLAC(CAOSMachine &vm);

  static int IntegerRV_ABBA(CAOSMachine &vm);
  static int IntegerRV_PNXT(CAOSMachine &vm);
  static int IntegerRV_TINT(CAOSMachine &vm);
  static int IntegerRV_TRAN(CAOSMachine &vm);

  static int IntegerRV_HOTP(CAOSMachine &vm);

  // Float RValues
  static float FloatRV_RNGE(CAOSMachine &vm);
  static float FloatRV_ACCG(CAOSMachine &vm);
  static float FloatRV_FLTX(CAOSMachine &vm);
  static float FloatRV_FLTY(CAOSMachine &vm);

  // String RValues
  static void StringRV_VOIS(CAOSMachine &vm, std::string &str);
  static void StringRV_HAND(CAOSMachine &vm, std::string &str);
  static void StringRV_GALL(CAOSMachine &vm, std::string &str);

  // Agent RValues
  static AgentHandle AgentRV_PNTR(CAOSMachine &vm);
  static AgentHandle AgentRV_NORN(CAOSMachine &vm);
  static AgentHandle AgentRV_NULL(CAOSMachine &vm);
  static AgentHandle AgentRV_HELD(CAOSMachine &vm);
  static AgentHandle AgentRV_CARR(CAOSMachine &vm);
  static AgentHandle AgentRV_AGNT(CAOSMachine &vm);
  static AgentHandle AgentRV_HOTS(CAOSMachine &vm);
  static AgentHandle AgentRV_TWIN(CAOSMachine &vm);
  static AgentHandle AgentRV_NCLS(CAOSMachine &vm);
  static AgentHandle AgentRV_PCLS(CAOSMachine &vm);

  // Variables
  static CAOSVar &Variable_OVnn(CAOSMachine &vm);
  static CAOSVar &Variable_MVnn(CAOSMachine &vm);
  static CAOSVar &Variable_VELX(CAOSMachine &vm);
  static CAOSVar &Variable_VELY(CAOSMachine &vm);
  static CAOSVar &Variable_AVAR(CAOSMachine &vm);
  static CAOSVar &Variable_NAME(CAOSMachine &vm);
  static CAOSVar &Variable_MAME(CAOSMachine &vm);

private:
  static void SubCommand_MESG_WRIT(CAOSMachine &vm);
  static void SubCommand_MESG_WRT(CAOSMachine &vm);

  static void SubCommand_NEW_SIMP(CAOSMachine &vm);
  static void SubCommand_NEW_CREA(CAOSMachine &vm);
  static void SubCommand_NEW_CRAG(CAOSMachine &vm);
  static void SubCommand_NEW_VHCL(CAOSMachine &vm);
  static void SubCommand_NEW_COMP(CAOSMachine &vm);
  static void SubCommand_NEW_GROW(CAOSMachine &vm);

  static void MoveAndFloatHelper(CAOSMachine &vm, bool floater);
};

#endif // AGENTHANDLERS_H
