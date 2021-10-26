// --------------------------------------------------------------------------
// Filename:	EntityImageClone.cpp
// Class:		EntityImageClone
// Purpose:		This class is the interface to the rest of C2e.  This class 
//				is closely based on the C2 entity and will evolve as required.
//				The difference between a cloned entityImage and a normal one
//				is that you can draw other sprites to the bitmaps canvas 
//
//				Clones hold their own pixel data in heap memory so that we can
//				write to the sprites.
//				
//				
// History:
// ------- 
// 05Feb98	Alima		Created. 
//							
//
// --------------------------------------------------------------------------
#ifndef CLONED_ENTITY_IMAGE_H
#define CLONED_ENTITY_IMAGE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "EntityImage.h"
#include "../Display/Sprite.h"

typedef std::vector<C2DAniStrip*>::iterator C2DANISTRIPVECTOR_ITERATOR;
class ClonedEntityImage : public EntityImage
{
	CREATURES_DECLARE_SERIAL( ClonedEntityImage )
public:
	ClonedEntityImage();
		
	ClonedEntityImage(FilePath const& galleryName, 
		int32 plane,//plane
		 int32 worldx, // world x
		 int32 worldy, // world y
		 uint32 baseimage,// base image
		  uint32 numImages,
		 uint32 currentimage );	//			// current image

	ClonedEntityImage(
		int32 plane,//plane
		 int32 worldx, // world x
		 int32 worldy, // world y
		 int32 width,
		 int32 height
		 );

	virtual ~ClonedEntityImage();

#ifdef C2D_DIRECT_DISPLAY_LIB

	virtual void CreateAnimStrip(int blockWidth, int blockHeight);
	
	void SetFontAttributes(int width , 
						  int height, 
						  uint32 colour, 
						  uint32 type );

	static void RefreshAllAnimStrips();

	virtual void DeleteAnimStrip();

#endif

// ----------------------------------------------------------------------
// Method:      SetGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseimage - the first image in the gallery relating to this
//				entity.
// Returns:     None
//
// Description: Create a gallery for this entity 	
//						
// ----------------------------------------------------------------------
	virtual bool SetGallery(FilePath const& galleryName,
							uint32 baseimage,
							uint32 numImages = 0,
							bool reset = true);

	int SelectFontType( std::string const &fontName, int r = 128, int g = 128, int b = 128, int rot = 128, int swap = 128);

	void DrawString( int32 x, int32 y,
					 const std::string& text,
					 int defaultFontIndex,
					 std::vector<Sprite::CharStyle> charStyles, int stylesOffset );

	void MeasureString( const std::string &text,
						int fontIndex,
						int32 &width, int32 &height );

	void DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed = 0,
							 uint8 lineColourGreen= 0,
							 uint8 lineColourBlue = 0);

	void DrawSprite(Position& whereToDraw,
							Bitmap* const bitmapToDraw);

	void DrawSprite(Position& whereToDraw,
					FilePath const& galleryName,
					int32 spriteIndex,
					int32 baseimage,
					int32 numimages);
	void Clear();
	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);

	virtual int32 GetAbsoluteBaseImage();



private:

	// Copy constructor and assignment operator
	// Declared but not implemented
	ClonedEntityImage (const ClonedEntityImage&);
	ClonedEntityImage& operator= (const ClonedEntityImage&);
	// This stops implicit casts from floats
	ClonedEntityImage
		(float plane,float x,float y,float width,float height);

#ifdef C2D_DIRECT_DISPLAY_LIB
	int myFontWidth;
	int myFontHeight;
	uint32 myFontColour;
	uint32	myFontType; // bold 700 or normal 400

	// when the direct display engine loses it's surfaces
	// we need to refresh each and every animstrip 
	// so keep a list of them here for speed
	static std::vector<C2DAniStrip*> ourClonedAnimationStrips;
#endif
};

#endif //CLONED_ENTITY_IMAGE_H

