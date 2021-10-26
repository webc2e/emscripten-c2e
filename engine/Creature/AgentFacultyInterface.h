#ifndef AgentFacultyInterface_H
#define AgentFacultyInterface_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"


class Classifier;
class CAOSVar;
class Voice;
class GenomeStore;
class AgentHandle;
class Vector2D;


class AgentFacultyInterface
{
public:
	virtual void SetWhetherHighlighted(bool f, int r=255, int g=255, int b=255) = 0;

	virtual int ExecuteScriptForClassifier(const Classifier& c, const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2) = 0;
	virtual int ExecuteScriptForEvent(int event, const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2) = 0;
	virtual const Classifier& GetClassifier() const = 0;
	virtual int GetOverrideCategory() = 0;
	virtual uint32 GetGenus() = 0;
	virtual Voice& GetVoice() = 0;
	virtual AgentHandle GetCarrier() = 0;
	virtual void SoundEffect(DWORD fsp, int delay=0) = 0;
	virtual void StopAllScripts() = 0;
	virtual bool VmIsRunning() = 0;
	virtual GenomeStore& GetGenomeStore() = 0;
	virtual int GetMovementStatus() = 0;
	virtual bool IsStopped() = 0;
	virtual Vector2D GetPosition() = 0;
	virtual uint32 GetFamily() = 0;
	virtual float GetVisualRange() const = 0;
	virtual Vector2D GetCentre() = 0;
	virtual float GetWidth() = 0;
	virtual float GetHeight() = 0;
	virtual bool GetRoomID(int &roomID) = 0;
	virtual bool TryToBeDropped(AgentHandle fromAgent, CAOSVar& p1, CAOSVar& p2, bool forceDrop) = 0;
	virtual int TestAttributes(int bits) = 0;
	virtual float GetCAIncrease() const = 0;

 	virtual Vector2D WhereShouldCreaturesPressMe() = 0;
	virtual AgentHandle GetAsAgentHandle() = 0;
	virtual bool CanSee(const AgentHandle& other) = 0;
	virtual AgentHandle GetCarried() = 0;
	virtual bool ShowPose(int pose,int part=0) = 0;
};
#endif //AgentFacultyInterface_H

