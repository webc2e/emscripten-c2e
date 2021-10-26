// $Id: stub_CDRomDevice.cpp,v 1.2 2000/09/12 14:07:45 cbuilder Exp $

#include "stub_CDRomDevice.h"

CDRomDevice::CDRomDevice()
{

}

CDRomDevice::~CDRomDevice()
{

}

void CDRomDevice::Initialise()
{
}

bool CDRomDevice::OpenCD(char *cdromName)
{
	return false;
}

void CDRomDevice::CloseCD(void)
{
}

bool CDRomDevice::GetStatus(int &track, int &frames)
{
	track = 0;
	frames = 0;
	return false;
}

bool CDRomDevice::PlayCD(int trackFrame, int endFrame)
{
	return false;
}

bool CDRomDevice::StopCD(void)
{
	return false;
}

bool CDRomDevice::PauseCD(void)
{
	return false;
}

bool CDRomDevice::ResumeCD(void)
{
	return false;
}

bool CDRomDevice::EjectCD(void)
{
	return false;
}

bool CDRomDevice::CloseCDTray(void)
{
	return false;
}

int CDRomDevice::GetTrackCount(void)
{
	return 0;
}

int CDRomDevice::GetTrackFrame(int track)
{
	return 0;
}

bool CDRomDevice::GetTrackInfo(void)
{
	return false;
}

void CDRomDevice::SetVolume(double loudness)
{
	return;
}

int CDRomDevice::GetNextFragment(void)
{
	return 0;
}

void CDRomDevice::GetCurrentCDFreqencies(int& low, int& med, int& high)
{
	low = 0;
	med = 0;
	high = 0;
}

void CDRomDevice::ShutDown()
{
}

