#ifndef REMOTE_CAMERA_H
#define REMOTE_CAMERA_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Camera.h"
// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif



class RemoteCamera: public Camera 
{
		CREATURES_DECLARE_SERIAL( RemoteCamera )
public:
	
	RemoteCamera();

	RemoteCamera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground,
			uint32 topLeftXofBackground, // top left world co-ordinates 
			uint32 topLeftYofBackground, // of background as a meta room
			int32 plane,
			RECT& bound); 

	virtual ~RemoteCamera();

	void PhysicallyMoveToWorldPosition(int32 x, int32 y);

	virtual void ZoomBy(int32 pixels,int32 x, int32 y );

	bool SetUpDrawingSurface();

	virtual bool TrackObject();

	virtual bool IsRemote(){return true;}

	void SetPlane(uint32 plane);
	

	// new serialization stuff
	virtual bool Write( CreaturesArchive &ar ) const;
	virtual bool Read( CreaturesArchive &ar );

	virtual void Refresh();
	virtual void Track(AgentHandle& agent, int xpercent, int ypercent,
		int style, int transition);

	virtual void Render();

private:
		
	// Copy constructor and assignment operator
	// Declared but not implemented
	RemoteCamera (const RemoteCamera&);
	RemoteCamera& operator= (const RemoteCamera&);

	virtual int GetMetaRoom();

	int32						myCameraWidth;
	int32						myCameraHeight;

	float myOriginalViewToPhysicalWidthRatio ;
	float myOriginalViewToPhysicalHeightRatio;
};
#endif // REMOTE_CAMERA_H

