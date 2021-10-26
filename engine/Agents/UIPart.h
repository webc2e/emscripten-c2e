#ifndef UIPART_H
#define UIPART_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompoundPart.h"
#include "../InputManager.h"
#include "../Display/Sprite.h"

class Message;

class Agent;

class ClonedEntityImage;

class UIPart : public CompoundPart
{
public:
	UIPart();
	UIPart(FilePath const& gallery, int baseimage, int numImages, Vector2D& relPos, int relplane );

	virtual void HandleUI( Message *msg );

protected:
	bool GetHotSpot( AgentHandle& agent, Vector2D& hotspot );
};

class UIButton : public UIPart
{
	CREATURES_DECLARE_SERIAL( UIButton )
public:
	UIButton();
	UIButton( FilePath const& gallery, int baseImage, int numImages, Vector2D& relPos, int relplane,
		std::vector<unsigned char> const &hoverAnim,
		int messageID, int option );

	enum State {
		Normal,
		Hover,
		Push
	};
/*
	virtual int GetInputMask() const
		{ return InputEvent::eventMouseDown; }
*/
	virtual void HandleUI( Message *msg );
	virtual bool SetAnim(const uint8* anim, int length);
	virtual void Tick();
	bool IsOver( const Vector2D& hotspot );

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

protected:

	int myMessageID;
	bool myTransparentTest;
	std::vector<unsigned char> myNormalAnim;
	std::vector<unsigned char> myHoverAnim;
	enum State myState;
};

class UIPartWithClonedImage : public UIPart
{
public:
	typedef UIPart base;

	UIPartWithClonedImage() : myClonedEntity(0) {}
	UIPartWithClonedImage(FilePath const& gallery, int baseimage, int numImages, Vector2D& relPos, int relplane );
	~UIPartWithClonedImage();

	virtual void SussPosition( const Vector2D& position );

	// ---------------------------------------------------------------------
	// Method:		SussPlane
	// Arguments:	mainplane - plot plane of the owning CompoundAgent
	// Returns:		None
	// Description:	Moves the part to the correct plane, with respect
	//				to the owning CompoundAgent.
	// ---------------------------------------------------------------------
	virtual void SussPlane( int mainplane );

	virtual void ChangeCameraShyStatus(bool shy);

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

	virtual void Tint(const uint16* tintTable, bool oneImage = false);

	virtual void DrawAlphaBlended(bool drawAlpha, int intensity);

protected:
	ClonedEntityImage *myClonedEntity;
};

struct TextAttributes
{
	TextAttributes( int leftMargin = 8, int topMargin = 8,
		int rightMargin = 8, int bottomMargin = 8,
		int lineSpacing = 0, int characterSpacing = 0, int justification = 0) :
		myLeftMargin(leftMargin),
		myTopMargin(topMargin),
		myRightMargin(rightMargin),
		myBottomMargin(bottomMargin),
		myLineSpacing(lineSpacing),
		myCharacterSpacing(characterSpacing),
		myJustification(justification)
		{}

	int myLeftMargin;
	int myTopMargin;
	int myRightMargin;
	int myBottomMargin;
	int myLineSpacing;
	int myCharacterSpacing;
	int myJustification;
};

CreaturesArchive &operator<<( CreaturesArchive &archive, TextAttributes const &attributes );
CreaturesArchive &operator>>( CreaturesArchive &archive, TextAttributes &attributes );

class UITextPart : public UIPartWithClonedImage
{
public:
	UITextPart() { myType = (partPlain | partUI | partText); }
	UITextPart(FilePath const& gallery, int baseimage, int numImages, Vector2D& relPos, int relplane, std::string fontName )
		: 	UIPartWithClonedImage(gallery, baseimage, numImages, relPos, relplane ),
	  myFontName( fontName ), myCurrentPage( 0 )
	{ myType = (partPlain | partUI | partText); }


// constructor for the new display lib
#ifdef C2D_DIRECT_DISPLAY_LIB
	UITextPart(FilePath const& gallery,
				int baseimage,
				int numImages, 
				Vector2D& relPos,
				int relplane,
				std::string fontName,
				int fontWidth,
				int fontHeight,
				uint8 red,
				uint8 blue,
				uint8 green,
				bool bold);

#endif

	virtual void SetText( std::string text ){}
	virtual std::string GetText() const {return myText;}

	void SetAttributes( const TextAttributes &attributes ) {myAttributes = attributes;}

	int GetPageCount();
	int GetCurrentPage();
	void SetCurrentPage( int page );

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);
	struct Line {
		Line( int first = 0, int last = 0, int y = 0 ) : firstChar( first ), lastChar( last ), yTop( y ) {}
		int firstChar;
		int lastChar;
		int yTop;
	};
	friend CreaturesArchive &operator<<( CreaturesArchive &ar, UITextPart::Line const &line );
	friend CreaturesArchive &operator>>( CreaturesArchive &ar, UITextPart::Line &line );
	struct Page {
		Page( int first = 0, int last = 0 ) : firstLine( first ), lastLine( last ) {}
		int firstLine;
		int lastLine;
	};
	friend CreaturesArchive &operator<<( CreaturesArchive &ar, UITextPart::Page const &page );
	friend CreaturesArchive &operator>>( CreaturesArchive &ar, UITextPart::Page &page );

#ifdef C2D_DIRECT_DISPLAY_LIB
	virtual void CreateAnimStrip(int blockwidth, int blockheight);
#endif


protected:
	void ParseFormattingCodes(std::string text);

	std::string myText;
	std::string myFontName;
	std::vector<Sprite::CharStyle> myCharStyles;
	std::string myTextWithFormatting;

	TextAttributes myAttributes;
	std::vector< Line > myLines;
	std::vector< Page > myPages;
	int myCurrentPage;

#ifdef C2D_DIRECT_DISPLAY_LIB
	int myFontWidth;
	int myFontHeight;
	uint32 myFontColour;
	uint32	myFontType; // bold 700 or normal 400
#endif

	void Draw();
	void FindLineStarts();
	void InverseLines(int bottom, int spaceHeight);


};


class UIText : public UITextPart, public TranslatedCharTarget
{
	CREATURES_DECLARE_SERIAL( UIText )
public:
	UIText();

#ifdef C2D_DIRECT_DISPLAY_LIB
	UIText( FilePath const& gallery, int baseimage, int numImages,
		Vector2D& relPos, int relplane, int messageID, std::string fontName,
				int fontWidth,
				int fontHeight,
				uint8 red,
				uint8 blue,
				uint8 green,
				bool bold );
#else
		UIText( FilePath const& gallery, int baseimage, int numImages,
		Vector2D& relPos, int relplane, int messageID, std::string fontName );

#endif
	~UIText();

	virtual void HandleUI( Message *msg );

	virtual bool SendChar( int keyCode );
	virtual bool RawKey( int keyCode );
	virtual bool AllowScroll();
	virtual bool LoseFocus();
	virtual void GainFocus();

	virtual void SetText( std::string text );
	virtual std::string GetText() const;

	void MoveCursor();
	int FindLineCursorIn();
	virtual void SussPosition( const Vector2D& position );
	virtual void SussPlane( int mainplane );
	// ---------------------------------------------------------------------
	// Method:		Tick
	// Arguments:	None
	// Returns:		None
	// Description:	Virtual function to update part. Default behaviour is
	//				to just call the EntityImage Animate() function to
	//				update the parts animation (if any).
	// ---------------------------------------------------------------------
	virtual void Tick();
	virtual void ChangeCameraShyStatus(bool shy);


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
protected:
	bool InsertSingleChar(unsigned char c);

	void MakeCursor();
	EntityImage *myCursor;
	int myCursorPosition;
	int myMessageID;
};

class UIFixedText : public UITextPart
{
	CREATURES_DECLARE_SERIAL( UIFixedText )
public:
	UIFixedText();

#ifdef C2D_DIRECT_DISPLAY_LIB
	UIFixedText( FilePath const& gallery, int baseimage, int numImages,
		Vector2D& relPos, int relplane,  std::string fontName,
				int fontWidth,
				int fontHeight,
				uint8 red,
				uint8 blue,
				uint8 green,
				bool bold );
#else
	UIFixedText( FilePath const& gallery, int baseimage, int numImages,
		Vector2D& relPos, int relplane, std::string fontName );
#endif

	~UIFixedText();


	virtual void SetText( std::string text );
	virtual std::string GetText() const;
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

protected:

};

class UIGraph : public UIPartWithClonedImage
{
	CREATURES_DECLARE_SERIAL( UIGraph )
public:
	UIGraph();
	UIGraph( FilePath const& gallery, int baseimage, int numImages,
		Vector2D& relPos, int relplane, int numValues );

	int AddLine( int r, int g, int b, float minY, float maxY );
	void AddValue( int lineIndex, float value );

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

	class Line {
	public:
		Line() {}
		Line( int r, int g, int b, float minY, float maxY, int numValues )
			: myRed( r ), myGreen( g ), myBlue( b ), myMin( minY ), myMax( maxY ), myWrapped( false ), myNext( 0 )
		{
			myValues.resize( numValues );
		}
		uint8 myRed;
		uint8 myGreen;
		uint8 myBlue;
		float myMin;
		float myMax;
		std::vector< float > myValues;
		int myNext;
		bool myWrapped;
	};
	friend CreaturesArchive &operator<<( CreaturesArchive &ar, UIGraph::Line const &line );
	friend CreaturesArchive &operator>>( CreaturesArchive &ar, UIGraph::Line &line );

protected:


	void Draw();

	std::vector< Line > myLines;
	int myNumValues;
};


#define LSYS_STACK_SIZE		100

class UIGenerative : public UIPartWithClonedImage
{
private:
	CREATURES_DECLARE_SERIAL( UIGenerative )

	typedef struct
	{
		float	angle;
		int		pos_x;
		int		pos_y;
	} lSysState;

	lSysState myStack[LSYS_STACK_SIZE];
	int myDepth, myBranchLength;
	int myPerturbBranchLength;
	float myDelta, myPerturbDelta;
	int myGenerationNumber;
	std::string myStartString, myReplaceString;

	bool PushState(float angle, int x, int y);
	void PopState();

public:
	UIGenerative();
	UIGenerative( FilePath const& gallery, int baseimage, int numImages,
		Vector2D& relPos, int relplane, int numValues );

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

	//Get and set methods for start string, replacement string and generation number
	//Generation number is not settable as this is done in the iterate method.
	void SetStartString(std::string startStr);
	std::string GetStartString();
	void SetReplaceString(std::string replaceStr);
	std::string GetReplaceString();
	int GetGenerationNumber();
	void SetBranchLength(int length);
	int GetBranchLength();
	void SetPerturbBranchLength(int perturb);
	int GetPerturbBranchLength();
	void SetPerturbDelta(float perturb);
	float GetPerturbDelta();
	void SetDelta(float delta);
	float GetDelta();

	//Perform numIterations iteratation on the L-System
	void Iterate(int numIterations);

//	friend CreaturesArchive &operator<<( CreaturesArchive &ar);
//	friend CreaturesArchive &operator>>( CreaturesArchive &ar);

protected:


	void Draw();
};

//CreaturesArchive &operator<<( CreaturesArchive &ar, UIGraph::Line const &line );
//CreaturesArchive &operator>>( CreaturesArchive &ar, UIGraph::Line &line );


#endif // UIPART_H

