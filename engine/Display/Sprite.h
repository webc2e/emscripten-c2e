// --------------------------------------------------------------------------
// Filename:	Sprite.h
// Class:		Sprite
// Purpose:		Each sprite has an undelying bitmap for its associated object.
//				A sprite keeps track of its Screen position and knows how to
//				draw itself taking into account transparent pixels.
//
// Description: I went for the has-a bitmap relationship rather is-a because
//				I could use the drawable object as a base class for all things
//				to be displayed over the background.
//			
//				
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Added comments.
//							Restructed adding drawable object base class
// --------------------------------------------------------------------------
#ifndef		ENGINESPRITE_H
#define		ENGINESPRITE_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"DrawableObject.h"
#include	"Gallery.h" // for file formats


class Bitmap;
class CompressedBitmap;
class Gallery;

// the possible number of bitmaps that could be drawn
// one on top of the other
// bitmap zero is the default sprite
const int NUM_BITMAPS = 4;

class Sprite : public DrawableObject
{
public:
	enum WhichGallery {PRIMARY_GALLERY =0,
		OVERLAY_GALLERY =1};


public:
	
	Sprite();

	void CreateGalleryForCloning(std::string path,
								uint32 baseImage,
								uint32 pixelFormat,
								uint32 numBitmaps);

	void CopyGalleryData(ClonedGallery& gallery);

	virtual ~Sprite();

	virtual Bitmap* GetBitmap(uint32 index) const ;
	virtual Bitmap* GetBitmap();

	uint32 GetBitmapCount() const;

	uint32 ClonedAbsoluteBase() const;

	bool SetBitmap(uint32 index, int32 layer,WhichGallery choice);

	void HideBitmap(int32 layer);
	virtual void ShowBitmap(int32 layer);




	virtual int32 GetWidth(void);

	virtual int32 GetHeight(void);

	virtual void SetScreenPosition(Position pos);


	virtual uint16* GetImageData() const;

	virtual void Draw();

	virtual void SetClippedDimensions(int32 w, int32 h);

// ----------------------------------------------------------------------
// Method:      Visible
// Arguments:   test - rectangle inside which to test whether I am visible.
// Returns:     true if i am visible within that rectangle false otherwise
// Description: Tells caller whether the entity exists inside the bounds
//              of the given rectangle
//
//              dummy function returns true until I have sussed 
//              cameras out
//			
// ----------------------------------------------------------------------
    virtual bool Visible( RECT& test);

	// this is so that the dirty rects can look after themselves
	virtual void SetCurrentBound(RECT* rect= NULL);

	virtual void DrawMirrored();

	bool IsMirrored(){return myDrawMirroredFlag;}

	void DrawMirrored(bool mirror);
	void DrawMirrored(uint32 layer, bool yesOrNo);

	bool SetGallery(std::string const& galleryName);
	void SetOverlayGallery(std::string const& galleryName);

	bool SetGallery(std::string const& galleryName,
								   uint32 baseImage,
								   uint32 numImages/*= 0*/);

	void DestroyPrimaryGallery();

	bool IsPixelTransparent(uint32 x,
							uint32 y,
							int32 imageIndex);

	std::string GetGalleryName() const;
	std::string GetOverlayGalleryName() const;

	Gallery* GetGallery() const {return myGallery;}

	bool HasOverlayGallery() const {return myOverlayGallery != NULL? true : false;}

	void RecolourPrimaryGallery(const uint16* tintTable);

	void YouAreCloned(bool flag ){myIAmClonedFlag = flag;}

	// cloned sprite
		void DrawOnMe(Position& whereToDraw,
					Bitmap* const bitmapToDraw);

	int SelectFont( const std::string &fontName, int r = 128, int g = 128, int b = 128, int rot = 128, int swap = 128);

	struct CharStyle
	{
		int myCharStart; // where character style begins in myText
		int myFontIndex;
	};
	friend CreaturesArchive &operator<<( CreaturesArchive &ar, Sprite::CharStyle const &style );
	friend CreaturesArchive &operator>>( CreaturesArchive &ar, Sprite::CharStyle &style );
	void DrawString( int32 x, int32 y, const std::string& text, int defaultFontIndex, std::vector<CharStyle> charStyles, int stylesOffset );

	void MeasureString( const std::string &text, int fontIndex, int32 &width, int32 &height );

	void DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed = 0,
							 uint8 lineColourGreen= 0,
							 uint8 lineColourBlue = 0);

	void DrawSpriteToBitmap( Bitmap* destination,
								 Position position,
								 Bitmap* const source );
	void Clear() ;

	void DrawAlphaBlended(bool drawAlpha,int intensity);
	void DrawAlpha();

protected:
	// a gallery of sprites
	Gallery*	myGallery;
	Gallery*	myOverlayGallery;

	Gallery::FileFormat			myFileFormat;

	Bitmap* myBitmaps[NUM_BITMAPS];
	CompressedBitmap* myCompressedBitmaps[NUM_BITMAPS];

	//we can hide some layers if we want
	bool myBitmapsToDraw[NUM_BITMAPS];
	bool myBitmapsToDrawMirrored[NUM_BITMAPS];
	bool myDrawMirroredFlag;
	bool myIAmClonedFlag;

	struct CachedFont
	{
		std::string myName;
		Gallery* myGallery;
		int r, g, b, rot, swap;

		CachedFont(std::string name, Gallery* gallery, int lr, int lg, int lb, int lrot, int lswap)
			: myName(name), myGallery(gallery), 
				r(lr), g(lg), b(lb), rot(lrot), swap(lswap) {}
	};
	typedef std::vector< CachedFont > FontCache;
	static FontCache myFontGalleries;

	bool myDrawAlphaFlag;
	int myAlphaIntensity;

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	Sprite (const Sprite&);
	Sprite& operator= (const Sprite&);

};


#endif		// ENGINESPRITE_H

