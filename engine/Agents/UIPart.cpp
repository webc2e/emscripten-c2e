#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "UIPart.h"
#include "../App.h"
#include "../World.h"
#include "../Camera/MainCamera.h"	// for theMainView

#ifdef C2D_DIRECT_DISPLAY_LIB

#else
#include "../Display/DisplayEngine.h"
#endif // C2D_DIRECT_DISPLAY_LIB

#include "../AgentDisplay/EntityImageClone.h"
#include "../Message.h"
#include "../AgentManager.h"
#include "PointerAgent.h"
#include "CompoundAgent.h"
#include <sstream>

#include "QuoteFactory.h" // For a nice "Oeuf de Paques"
#include "../../common/ProfanityFilter.h"

#ifndef _WIN32
#include "../unix/KeyScan.h"
#endif

UIPart::UIPart() { myType = (partPlain | partUI); }

UIPart::UIPart(FilePath const &gallery, int baseimage, int numImages,
               Vector2D &relPos, int relplane)
    : CompoundPart(gallery, baseimage, relPos, relplane, numImages) {
  myType = (partPlain | partUI);
}

void UIPart::HandleUI(Message *msg) {}

bool UIPart::GetHotSpot(AgentHandle &agent, Vector2D &hotspot) {
  if (agent.IsPointerAgent()) {
    PointerAgent &pointerAgent = agent.GetPointerAgentReference();
    hotspot = pointerAgent.GetHotSpot();
    return true;
  } else {
    hotspot = agent.GetAgentReference().GetPosition();
    return false;
  }
}

CREATURES_IMPLEMENT_SERIAL(UIButton)

UIButton::UIButton()
    : UIPart(), myMessageID(0), myState(Normal), myTransparentTest(false) {
  myType = partPlain | partUI | partButton;
}

UIButton::UIButton(FilePath const &gallery, int baseImage, int numImages,
                   Vector2D &relPos, int relplane,
                   std::vector<unsigned char> const &hoverAnim, int messageID,
                   int option)
    : UIPart(gallery, baseImage, numImages, relPos, relplane),
      myHoverAnim(hoverAnim), myMessageID(messageID), myState(Normal),
      myTransparentTest(option != 0) {
  myPixelPerfectHitTestFlag = (option != 0);
  myType = partPlain | partUI | partButton;
}

bool UIButton::IsOver(const Vector2D &hotspot) {
  int x = Map::FastFloatToInteger(hotspot.x);
  int y = Map::FastFloatToInteger(hotspot.y);

#ifndef C2D_DIRECT_DISPLAY_LIB
  if (myTransparentTest)
    return (!myEntity->IsPixelTransparent(x, y));
  else
#endif
    return (x >= myEntity->GetWorldX() &&
            x <= myEntity->GetWorldX() + myEntity->GetWidth() &&
            y >= myEntity->GetWorldY() &&
            y <= myEntity->GetWorldY() + myEntity->GetHeight());
}

void UIButton::Tick() {
  UIPart::Tick();
  Vector2D hotspot = thePointer.GetPointerAgentReference().GetHotSpot();
  if (IsOver(hotspot) && myHoverAnim.size() &&
      thePointer.GetPointerAgentReference().Find() == *GetParent()) {
    if (myState != Hover) { // SPARKY CHANGES
                            // myEntity->SetAnim( (myHoverAnim.begin()),
                            // myHoverAnim.size() );
    }
    myState = Hover;
  } else {
    if (myState == Hover) { // SPARKY CHANGES
                            // myEntity->SetAnim( myNormalAnim.begin(),
                            // myNormalAnim.size() );
    }
    myState = Normal;
  }
}

void UIButton::HandleUI(Message *msg) {
  Vector2D hotspot;
  CAOSVar fromVar = msg->GetFrom();
  if (fromVar.GetType() != CAOSVar::typeAgent)
    return;
  AgentHandle fromAgent = fromVar.GetAgent();
  if (fromAgent.IsInvalid())
    return;
  GetHotSpot(fromAgent, hotspot);

  if (msg->GetMsg() == SCRIPTUIMOUSEDOWN &&
      (msg->GetP1().GetType() == CAOSVar::typeInteger &&
       msg->GetP1().GetInteger() == 0)) {
    if (IsOver(hotspot)) {
      // Change by Daniel to make P1 of the message be the part index of the
      // button.
      CAOSVar p1, p2;
      p1.SetInteger(GetParent()->GetIndex(this));
      p2.SetInteger(0);
      theApp.GetWorld().WriteMessage(fromAgent, msg->GetTo(), myMessageID, p1,
                                     p2, 0);
    }
  }
  /*
          else if( msg->GetMsg() == SCRIPTUIMOUSEMOVE )
          {
                  if( IsOver( hotspot ) && myHoverAnim.size() )
                  {
                          if( myState != State::Hover )
                                  myEntity->SetAnim( myHoverAnim.begin(),
     myHoverAnim.size() ); myState = State::Hover;
                  }
                  else
                  {
                          if( myState == State::Hover )
                                  myEntity->SetAnim( myNormalAnim.begin(),
     myNormalAnim.size() ); myState = State::Normal;
                  }
          }
          else if( msg->GetMsg() == SCRIPTUIMOUSELEAVE )
          {
                  if( myState == State::Hover )
                          myEntity->SetAnim( myNormalAnim.begin(),
     myNormalAnim.size() ); myState = State::Normal;
          }
  */
}

bool UIButton::SetAnim(const uint8 *anim, int length) {
  if (!UIPart::SetAnim(anim, length))
    return false;

  myNormalAnim = std::vector<unsigned char>(anim, anim + length);
  return true;
}

bool UIButton::Write(CreaturesArchive &archive) const {
  UIPart::Write(archive);
  archive << myMessageID << myTransparentTest;

  int i, size;

  size = myNormalAnim.size();
  archive << size;
  for (i = 0; i < size; ++i)
    archive << myNormalAnim[i];

  size = myHoverAnim.size();
  archive << size;
  for (i = 0; i < size; ++i)
    archive << myHoverAnim[i];

  return true;
}

bool UIButton::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {

    if (!UIPart::Read(archive))
      return false;

    archive >> myMessageID >> myTransparentTest;

    int i, size;

    archive >> size;
    myNormalAnim.resize(size);
    for (i = 0; i < size; ++i)
      archive >> myNormalAnim[i];

    archive >> size;
    myHoverAnim.resize(size);
    for (i = 0; i < size; ++i)
      archive >> myHoverAnim[i];
  } else {
    _ASSERT(false);
    return false;
  }
  return true;
}

UIPartWithClonedImage::UIPartWithClonedImage(FilePath const &gallery,
                                             int baseimage, int numImages,
                                             Vector2D &relPos, int relplane)
    : UIPart(gallery, baseimage, numImages, relPos, relplane) {
  myClonedEntity =
      new ClonedEntityImage(gallery, relplane, 0, 0, baseimage, 1, 0);
}

UIPartWithClonedImage::~UIPartWithClonedImage() { delete myClonedEntity; }

void UIPartWithClonedImage::SussPosition(const Vector2D &position) {
  UIPart::SussPosition(position);
  if (myClonedEntity)
    myClonedEntity->SetPosition(
        Map::FastFloatToInteger(position.x + myRelativePosition.x),
        Map::FastFloatToInteger(position.y + myRelativePosition.y));
}

void UIPartWithClonedImage::SussPlane(int mainplane) {
  UIPart::SussPlane(mainplane);
  if (myClonedEntity)
    myClonedEntity->SetPlane(mainplane + myRelPlane);
}

void UIText::SussPlane(int mainplane) {
  UITextPart::SussPlane(mainplane);
  if (myCursor)
    myCursor->SetPlane(myClonedEntity->GetPlane() + 1);
}

void UIPartWithClonedImage::ChangeCameraShyStatus(bool shy) {
  UIPart::ChangeCameraShyStatus(shy);
  if (myClonedEntity)
    myClonedEntity->YouAreCameraShy(shy);
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool UIPartWithClonedImage::Write(CreaturesArchive &archive) const {
  UIPart::Write(archive);
  archive << myClonedEntity;

  return true;
}

bool UIPartWithClonedImage::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {
    if (!UIPart::Read(archive))
      return false;

    archive >> myClonedEntity;
  } else {
    _ASSERT(false);
    return false;
  }
  return true;
}

CreaturesArchive &operator<<(CreaturesArchive &archive,
                             TextAttributes const &a) {
  archive << a.myLeftMargin << a.myTopMargin << a.myRightMargin
          << a.myBottomMargin << a.myLineSpacing << a.myCharacterSpacing
          << a.myJustification;
  return archive;
}

CreaturesArchive &operator>>(CreaturesArchive &archive, TextAttributes &a) {
  archive >> a.myLeftMargin >> a.myTopMargin >> a.myRightMargin >>
      a.myBottomMargin >> a.myLineSpacing >> a.myCharacterSpacing >>
      a.myJustification;
  return archive;
}

bool UITextPartHelperCanBreak(const std::string &text, int index) {
  if (text[index] != ' ')
    return false;
  if (text.length() <= index + 1)
    return true;
  char next = text[index + 1];
  if (next == '!' || next == '?' || next == ':')
    return false;
  return true;
}

void UITextPart::FindLineStarts() {
  if (myClonedEntity) {
    myLines.clear();
    myPages.clear();
    int32 spaceWidth, spaceHeight;

    int font = -1;
#ifdef C2D_DIRECT_DISPLAY_LIB

    myClonedEntity->SelectFontType(myFontName);
#else
    FilePath path(myFontName + ".s16", IMAGES_DIR);
    path.LowerCase();
    font = myClonedEntity->SelectFontType(path.GetFullPath());
#endif

    myClonedEntity->MeasureString(" ", font, spaceWidth, spaceHeight);

    int right = myClonedEntity->GetWidth() - myAttributes.myRightMargin;
    int bottom =
        myClonedEntity->GetHeight() - myAttributes.myBottomMargin - spaceHeight;
    int x = myAttributes.myLeftMargin;
    int y = myAttributes.myTopMargin;

    int index = 0;
    int length = myText.size();
    int lineStart = 0;
    int lineLength = 0;
    int pageStart = 0;
    int maxLength = myClonedEntity->GetWidth() - myAttributes.myLeftMargin -
                    myAttributes.myRightMargin;

    while (index < length) {
      if (myText[index] == '\n') {
        myLines.push_back(Line(lineStart, index, y));
        y += spaceHeight + myAttributes.myLineSpacing;
        lineStart = index + 1;
        lineLength = 0;
      } else {
        long charWidth, charHeight;
        myClonedEntity->MeasureString(myText.substr(index, 1), font, charWidth,
                                      charHeight);
        lineLength += charWidth;
        if (lineLength > maxLength) // need to wrap
        {
          int lineBreak = index;
          while (lineBreak > lineStart &&
                 !UITextPartHelperCanBreak(myText, lineBreak))
            --lineBreak;
          if (lineBreak == lineStart)
            lineBreak = index;
          myLines.push_back(Line(lineStart, lineBreak, y));
          index = lineBreak;
          while (myText[index] == ' ')
            ++index;
          lineStart = index;
          y += spaceHeight + myAttributes.myLineSpacing;
          myClonedEntity->MeasureString(myText.substr(index, 1), font,
                                        charWidth, charHeight);
          lineLength = charWidth;
        }
      }
      if (y >= bottom) {
        myPages.push_back(Page(pageStart, myLines.size()));
        pageStart = myLines.size();
        y = myAttributes.myTopMargin;
      }
      ++index;
    }
    myLines.push_back(Line(lineStart, index, y));
    myPages.push_back(Page(pageStart, myLines.size()));

    if (myAttributes.myJustification & 16)
      InverseLines(bottom, spaceHeight);
  }
}

void UITextPart::InverseLines(int bottom, int spaceHeight) {
  // Remaps the pages, so they start at the bottom of the last page.
  // Useful for a scrolling style effect.  Changes line heights
  // to do this

  if (myLines.size() <= 0)
    return;

  myPages.clear();

  int y = bottom;
  int pageEnd = myLines.size();

  for (int lineIndex = myLines.size() - 1; lineIndex >= 0; --lineIndex) {
    Line &line = myLines[lineIndex];
    line.yTop = y;
    y -= spaceHeight + myAttributes.myLineSpacing;
    if (y < myAttributes.myTopMargin) {
      y = bottom;
      myPages.insert(myPages.begin(), Page(lineIndex, pageEnd));
      pageEnd = lineIndex;
    }
  }
  if (pageEnd > 0)
    myPages.insert(myPages.begin(), Page(0, pageEnd));
}

int UITextPart::GetPageCount() {
  FindLineStarts();
  return myPages.size();
}

int UITextPart::GetCurrentPage() { return myCurrentPage; }

void UITextPart::SetCurrentPage(int page) {
  FindLineStarts();
  if (page >= 0 && page < myPages.size()) {
    myCurrentPage = page;
    Draw();
  }
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool UITextPart::Write(CreaturesArchive &archive) const {
  UIPartWithClonedImage::Write(archive);
  archive << myAttributes << myText << myFontName << myLines << myPages
          << myCurrentPage;

#ifdef C2D_DIRECT_DISPLAY_LIB
  archive << myFontWidth << myFontHeight << myFontColour << myFontType;
#endif

  archive << myTextWithFormatting;

  return true;
}

bool UITextPart::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {
    if (!UIPartWithClonedImage::Read(archive))
      return false;

    archive >> myAttributes >> myText >> myFontName >> myLines >> myPages >>
        myCurrentPage;

#ifdef C2D_DIRECT_DISPLAY_LIB
    archive >> myFontWidth >> myFontHeight >> myFontColour >> myFontType;
#endif

    if (archive.GetFileVersion() >= 34) {
      std::string text = myText;
      archive >> myTextWithFormatting;
      ParseFormattingCodes(myTextWithFormatting);
      myText = text;
    }
  } else {
    _ASSERT(false);
    return false;
  }
#ifdef C2D_DIRECT_DISPLAY_LIB
  Draw();
#endif
  return true;
}

CREATURES_IMPLEMENT_SERIAL(UIText)

UIText::UIText() : myMessageID(0), myCursor(0), myCursorPosition(0) {
  myType = (partPlain | partUI | partText | partEdit);
}

#ifdef C2D_DIRECT_DISPLAY_LIB

UIText::UIText(FilePath const &gallery, int baseimage, int numImages,
               Vector2D &relPos, int relplane, int messageID,
               std::string fontName, int fontWidth, int fontHeight, uint8 red,
               uint8 green, uint8 blue, bool bold)
    : UITextPart(gallery, baseimage, numImages, relPos, relplane, fontName,
                 fontWidth, fontHeight, red, green, blue, bold),
      myMessageID(messageID), myCursor(0) {
  myType = (partPlain | partUI | partText | partEdit);
  myText = "";
  Draw();
}

#else

UIText::UIText(FilePath const &gallery, int baseimage, int numImages,
               Vector2D &relPos, int relplane, int messageID,
               std::string fontName)
    : UITextPart(gallery, baseimage, numImages, relPos, relplane, fontName),
      myMessageID(messageID), myCursor(0), myCursorPosition(0) {
  myType = (partPlain | partUI | partText | partEdit);
  myText = "";
  Draw();
}

#endif

UIText::~UIText() { delete myCursor; }

void UIText::SussPosition(const Vector2D &position) {
  UITextPart::SussPosition(position);
  FindLineStarts();
  MoveCursor();
}

void UIText::ChangeCameraShyStatus(bool shy) {
  UITextPart::ChangeCameraShyStatus(shy);
  if (myCursor)
    myCursor->YouAreCameraShy(shy);
}

bool UIText::AllowScroll() { return myText.empty(); }

bool UIText::RawKey(int keyCode) {
  bool handled = false;

  if (keyCode == VK_LEFT && myCursorPosition > 0) {
    myCursorPosition--;
    handled = true;
  } else if (keyCode == VK_RIGHT && myCursorPosition < myText.size()) {
    myCursorPosition++;
    handled = true;
  } else if (keyCode == VK_UP) {
    int lineIndex = FindLineCursorIn();
    handled = true;
    if (lineIndex > 0)
      myCursorPosition = myLines[lineIndex - 1].firstChar;
    else
      myCursorPosition = 0;
  } else if (keyCode == VK_DOWN) {
    int lineIndex = FindLineCursorIn();
    handled = true;
    if (lineIndex < myLines.size() - 1)
      myCursorPosition = myLines[lineIndex + 1].firstChar;
    else
      myCursorPosition = myText.size();
  } else if (keyCode == VK_DELETE && myText.size()) {
    // Japanese - this deleting would need making multibyte aware
    if (myCursorPosition < myText.size() && !myText.empty()) {
      myText.erase(myText.begin() + myCursorPosition);
      handled = true;
    }
  } else if (keyCode == 'C' && theApp.GetInputManager().IsKeyDown(VK_CONTROL)) {
    // Ctrl+C - copy

    // default to copying from text part which is active
    std::string text = myText;

    // but search for a text part under the cursor, if we can
    if (thePointer.IsValid()) {
      // Look for an agent under the mouse
      AgentHandle a = thePointer.GetPointerAgentReference().IsTouching(0, 0);
      if (a.IsValid()) {
        if (a.IsCompoundAgent()) {
          // Find the part underneath the hot spot of the pointer
          int part = a.GetCompoundAgentReference().GetPartOver(
              thePointer.GetPointerAgentReference().GetHotSpot());
          CompoundPart *aPart = a.GetCompoundAgentReference().GetPart(part);
          UITextPart *uiPart = dynamic_cast<UITextPart *>(aPart);
          if (uiPart) {
            text = uiPart->GetText();
          }
        }
      }
    }

    theApp.GetInputManager().SendTextToClipboard(text);
  } else if (keyCode == 'V' && theApp.GetInputManager().IsKeyDown(VK_CONTROL)) {
    // Ctrl+V - paste
    std::string cliptext = theApp.GetInputManager().FetchTextFromClipboard();
    for (int i = 0; i < cliptext.size(); ++i) {
      unsigned char c = cliptext[i];
      if (c >= 32 || c == 10)
        if (!InsertSingleChar(c))
          break;
      if (c == 9) {
        InsertSingleChar(' ');
        InsertSingleChar(' ');
        InsertSingleChar(' ');
        InsertSingleChar(' ');
      }
    }
    handled = true;
  }

  if (handled) {
    Draw();
    MoveCursor();
  }

  return handled;
}

bool UIText::InsertSingleChar(unsigned char c) {
  if (c < 32 && c != 10)
    return false;
  char signC = c;

  int font = -1;
#ifdef C2D_DIRECT_DISPLAY_LIB
  myClonedEntity->SelectFontType(myFontName);
#else
  FilePath path(myFontName + ".s16", IMAGES_DIR);
  path.LowerCase();
  font = myClonedEntity->SelectFontType(path.GetFullPath());
#endif

  int32 spaceWidth, spaceHeight;
  std::string newChar(&signC, 1);
  myClonedEntity->MeasureString(newChar, font, spaceWidth, spaceHeight);
  // Characters less than 5 pixels in height are 'dummies'
  if (spaceHeight > 4 || c == 10) {
    // See whether new character will fit in the background (with space margin
    // on each side)
    std::string tempText = myText;
    if (myCursorPosition >= myText.size())
      myText += c;
    else
      myText.insert(myText.begin() + myCursorPosition, c);
    FindLineStarts();
    if (myPages.size() > 1)
      myText = tempText;
    else {
      myCursorPosition++;
      return true;
      ;
    }
  }

  return false;
}

bool UIText::SendChar(int keyCode) {
  bool handled = false;

  if ((keyCode >= 32) && (keyCode <= 255)) {
    if (InsertSingleChar(keyCode))
      handled = true;
  } else if (keyCode == 8 && myText.size()) {
    // Japanese - this backspacing would need making multibyte aware
    if (myCursorPosition > 0 && !myText.empty()) {
      myText.erase(myText.begin() + myCursorPosition - 1);
      myCursorPosition--;
      handled = true;
    }
  } else if (keyCode == 13 && myMessageID > 0) {
    theApp.GetWorld().WriteMessage(AgentHandle((Agent *)GetParent()),
                                   AgentHandle((Agent *)GetParent()),
                                   myMessageID, INTEGERZERO, INTEGERZERO, 0);
    handled = true;
  } else if (keyCode == 13 && myMessageID == -1) {
    if (InsertSingleChar(10))
      handled = true;
  }

  if (handled) {
    Draw();
    MoveCursor();
  }

  return handled;
}

void UIText::HandleUI(Message *msg) {
  Vector2D hotspot;
  if (msg->GetMsg() == SCRIPTUIMOUSEDOWN &&
      (msg->GetP1().GetType() == CAOSVar::typeInteger &&
       msg->GetP1().GetInteger() == 0)) {
    CAOSVar fromVar = msg->GetFrom();
    if (fromVar.GetType() != CAOSVar::typeAgent)
      return;
    AgentHandle fromAgent = fromVar.GetAgent();
    if (fromAgent.IsInvalid())
      return;
    GetHotSpot(fromAgent, hotspot);
    int xEnt = Map::FastFloatToInteger(hotspot.x) - myEntity->GetWorldX();
    int yEnt = Map::FastFloatToInteger(hotspot.y) - myEntity->GetWorldY();
    if (xEnt >= 0 && xEnt < myEntity->GetWidth() && yEnt >= 0 &&
        yEnt < myEntity->GetHeight()) {
      InputManager &inputManager = theApp.GetInputManager();
      inputManager.SetTranslatedCharTarget(this);
    }
  }
}

void UIText::MakeCursor() {
  uint8 anim[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 255};
  myCursor = new EntityImage(
      FilePath("cursor.s16", IMAGES_DIR), 0, myClonedEntity->GetPlane() + 1,
      myEntity->GetWorldX(), myEntity->GetWorldY(), 0, 0);
  myCursor->SetAnim(anim, sizeof(anim) / sizeof(uint8));
  if (myEntity)
    myCursor->YouAreCameraShy(myEntity->AreYouCameraShy());
  MoveCursor();
}

int UIText::FindLineCursorIn() {
  int lineIndex;
  for (lineIndex = myPages[0].firstLine; lineIndex < myPages[0].lastLine;
       ++lineIndex) {
    Line &line = myLines[lineIndex];
    if (myCursorPosition >= line.firstChar && myCursorPosition <= line.lastChar)
      break;
  }
  if (lineIndex >= myPages[0].lastLine)
    --lineIndex;

  return lineIndex;
}

void UIText::MoveCursor() {
  if (myCursor) {
    int font = -1;
#ifdef C2D_DIRECT_DISPLAY_LIB
    myClonedEntity->SelectFontType(myFontName);
#else
    FilePath path(myFontName + ".s16", IMAGES_DIR);
    path.LowerCase();
    font = myClonedEntity->SelectFontType(path.GetFullPath());
#endif

    // Find line that cursor is in
    int lineIndex = FindLineCursorIn();

    int32 width, height;
    Line &line = myLines[lineIndex];
    myClonedEntity->MeasureString(
        myText.substr(line.firstChar, myCursorPosition - line.firstChar), font,
        width, height);

    int off = line.yTop;

    int x = myAttributes.myLeftMargin;
    if ((myAttributes.myJustification & 3) == 1) // Right
      x = myClonedEntity->GetWidth() - width - myAttributes.myRightMargin;
    else if ((myAttributes.myJustification & 3) == 2) // Centre
      x = (myAttributes.myLeftMargin + myClonedEntity->GetWidth() -
           myAttributes.myRightMargin) /
              2 -
          width / 2;

    if (myCursorPosition <= line.lastChar)
      x -= 3;

    myCursor->SetPosition(myClonedEntity->GetWorldX() + width + x + 1,
                          myClonedEntity->GetWorldY() + off);
  }
}

void UITextPart::ParseFormattingCodes(std::string text) {
  myTextWithFormatting = text;
  myCharStyles.clear();

  std::string rawText;

  bool formattingCode = false;
  std::string code;
  for (int i = 0; i < text.size(); ++i) {
    char c = text[i];

    if (c == '>') {
      if (formattingCode) {
        // Correct closing tag
        formattingCode = false;
        // parse actual code
        std::istringstream in(code);
        std::string tokenName;
        in >> tokenName;
        if (tokenName == "tint") {
          int r = 128, g = 128, b = 128, rot = 128, swap = 128;
          in >> r >> g >> b >> rot >> swap;

          int font = -1;
#ifdef C2D_DIRECT_DISPLAY_LIB
          myClonedEntity->SelectFontType(myFontName);
#else
          FilePath path(myFontName + ".s16", IMAGES_DIR);
          path.LowerCase();
          font = myClonedEntity->SelectFontType(path.GetFullPath(), r, g, b,
                                                rot, swap);
#endif

          Sprite::CharStyle style;
          style.myCharStart = rawText.size();
          style.myFontIndex = font;
          myCharStyles.push_back(style);
        } else {
          rawText += "<" + code + ">";
        }
        code = "";
      } else {
        // Silly closing tag
        rawText += ">";
        code = "";
      }
    } else if (c == '<') {
      if (formattingCode) {
        // Previous open tag was duff, as this is another open tag without close
        rawText += "<" + code;
        formattingCode = true;
        code = "";
      } else {
        // Correct open tag
        formattingCode = true;
        code = "";
      }
    } else {
      if (formattingCode)
        code += c;
      else
        rawText += c;
    }
  }
  if (formattingCode)
    rawText += "<" + code;
  myText = rawText;
}

void UIText::SetText(std::string text) {
  // Remove swear words
  theProfanityFilter.Filter(text);
  ParseFormattingCodes(text);
  myCurrentPage = 0;
  myCursorPosition = myText.size();
  Draw();
  MoveCursor();
}

std::string UIText::GetText() const { return myText; }

bool UIText::LoseFocus() {
  Draw();
  delete myCursor;
  myCursor = 0;
  return true;
}

void UIText::GainFocus() {
  Draw();
  delete myCursor;
  myCursor = 0;
  MakeCursor();
}

// ---------------------------------------------------------------------
// Method:		Tick
// Arguments:	None
// Returns:		None
// Description:	Virtual function to update part. Default behaviour is
//				to just call the EntityImage Animate() function
// to 				update the parts animation (if any).
// ---------------------------------------------------------------------
void UIText::Tick() {
  if (myCursor)
    myCursor->Animate();
  UIPart::Tick();
}

bool UIText::Write(CreaturesArchive &archive) const {
  UITextPart::Write(archive);
  archive << myMessageID << myCursor;
  archive << myCursorPosition;
  SaveFocusState(archive);
  return true;
}

bool UIText::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {

    if (!UITextPart::Read(archive))
      return false;

    archive >> myMessageID >> myCursor;
    if (archive.GetFileVersion() >= 33)
      archive >> myCursorPosition;
    RestoreFocusState(archive);
  } else {
    _ASSERT(false);
    return false;
  }
  return true;
}

CREATURES_IMPLEMENT_SERIAL(UIFixedText)

UIFixedText::UIFixedText() {}

#ifdef C2D_DIRECT_DISPLAY_LIB
UIFixedText::UIFixedText(FilePath const &gallery, int baseimage, int numImages,
                         Vector2D &relPos, int relplane, std::string fontName,
                         int fontWidth, int fontHeight, uint8 red, uint8 green,
                         uint8 blue, bool bold)
    : UITextPart(gallery, baseimage, numImages, relPos, relplane, fontName,
                 fontWidth, fontHeight, red, green, blue, bold) {
  myText = "";
  Draw();
}

#else

UIFixedText::UIFixedText(FilePath const &gallery, int baseimage, int numImages,
                         Vector2D &relPos, int relplane, std::string fontName)
    : UITextPart(gallery, baseimage, numImages, relPos, relplane, fontName) {
  myText = "";
  Draw();
}

#endif

UIFixedText::~UIFixedText() {}

void UIFixedText::SetText(std::string text) {
  // Remove swear words
  theProfanityFilter.Filter(text);
  ConvertDullText(text);
  ParseFormattingCodes(text);
  myCurrentPage = 0;
  Draw();
}

std::string UIFixedText::GetText() const { return myText; }

#ifdef C2D_DIRECT_DISPLAY_LIB

UITextPart::UITextPart(FilePath const &gallery, int baseimage, int numImages,
                       Vector2D &relPos, int relplane, std::string fontName,
                       int fontWidth, int fontHeight, uint8 red, uint8 green,
                       uint8 blue, bool bold)
    : UIPartWithClonedImage(gallery, baseimage, numImages, relPos, relplane),
      myFontName(fontName), myCurrentPage(0), myFontWidth(fontWidth),
      myFontHeight(fontHeight) {
  myType = (partPlain | partUI | partText);

  myFontColour = (((blue)&0xff) << 16) | (((green)&0xff) << 8) | (((red)&0xff));

  if (bold)
    myFontType = 700;
  else
    myFontType = 400;

  if (myClonedEntity) {
    myClonedEntity->SetFontAttributes(myFontWidth, myFontHeight, myFontColour,
                                      myFontType);
  }
}

void UITextPart::CreateAnimStrip(int blockwidth, int blockheight) {
  if (myClonedEntity) {
    myClonedEntity->CreateAnimStrip(blockwidth, blockheight);
  }
}
#endif

void UITextPart::Draw() {
  if (myClonedEntity) {
    myClonedEntity->Clear();
    FindLineStarts();
    int i;
    int font = -1;
#ifdef C2D_DIRECT_DISPLAY_LIB
    int r = 128, g = 128, b = 128, rot = 128, swap = 128;
    myClonedEntity->SelectFontType(myFontName, r, g, b, rot, swap);
#else
    FilePath path(myFontName + ".s16", IMAGES_DIR);
    path.LowerCase();
    font = myClonedEntity->SelectFontType(path.GetFullPath());
#endif
    int topOffset = 0;
    if (myAttributes.myJustification & 12) {
      long width, lineHeight;
      int availableHeight = myClonedEntity->GetHeight() -
                            myAttributes.myTopMargin -
                            myAttributes.myBottomMargin;
      myClonedEntity->MeasureString(" ", font, width, lineHeight);
      int textHeight =
          (myPages[myCurrentPage].lastLine - myPages[myCurrentPage].firstLine) *
          lineHeight;
      if ((myAttributes.myJustification & 12) == 4) // Bottom
        topOffset = availableHeight - textHeight;
      if ((myAttributes.myJustification & 12) == 8) // Middle
        topOffset = (availableHeight - textHeight) / 2;
    }
    for (i = myPages[myCurrentPage].firstLine;
         i != myPages[myCurrentPage].lastLine; ++i) {
      long width, height;
      myClonedEntity->MeasureString(
          myText.substr(myLines[i].firstChar,
                        myLines[i].lastChar - myLines[i].firstChar),
          font, width, height);
      int x = myAttributes.myLeftMargin;
      if ((myAttributes.myJustification & 3) == 1) // Right
        x = myClonedEntity->GetWidth() - width - myAttributes.myRightMargin - 1;
      else if ((myAttributes.myJustification & 3) == 2) // Centre
        x = (myAttributes.myLeftMargin + myClonedEntity->GetWidth() -
             myAttributes.myRightMargin) /
                2 -
            width / 2;
      myClonedEntity->DrawString(
          x, myLines[i].yTop + topOffset,
          myText.substr(myLines[i].firstChar,
                        myLines[i].lastChar - myLines[i].firstChar),
          font, myCharStyles, myLines[i].firstChar);
    }
  }
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool UIFixedText::Write(CreaturesArchive &archive) const {
  UITextPart::Write(archive);

  return true;
}

bool UIFixedText::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {

    if (!UITextPart::Read(archive))
      return false;
  } else {
    _ASSERT(false);
    return false;
  }
  return true;
}

CREATURES_IMPLEMENT_SERIAL(UIGraph)

UIGraph::UIGraph() { myType = (partPlain | partUI | partGraph); }

UIGraph::UIGraph(FilePath const &gallery, int baseimage, int numImages,
                 Vector2D &relPos, int relplane, int numValues)
    : UIPartWithClonedImage(gallery, baseimage, numImages, relPos, relplane),
      myNumValues(numValues) {
  myType = (partPlain | partUI | partGraph);
}

int UIGraph::AddLine(int r, int g, int b, float minY, float maxY) {
  myLines.push_back(Line(r, g, b, minY, maxY, myNumValues));
  return myLines.size() - 1;
}

void UIGraph::AddValue(int lineIndex, float value) {
  if (lineIndex < 0 || lineIndex >= myLines.size()) {
    return;
  }

  // Line &line = myLines.at(lineIndex);
  Line &line = myLines[lineIndex];

  line.myValues[line.myNext++] = value;
  if (line.myNext == myNumValues) {
    line.myNext = 0;
    line.myWrapped = true;
  }

  if (lineIndex == myLines.size() - 1)
    Draw();
}

bool UIGraph::Write(CreaturesArchive &archive) const {
  UIPartWithClonedImage::Write(archive);
  archive << myLines << myNumValues;
  return true;
}

bool UIGraph::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {

    if (!UIPartWithClonedImage::Read(archive))
      return false;

    archive >> myLines >> myNumValues;
  } else {
    _ASSERT(false);
    return false;
  }
  return true;
}

void UIGraph::Draw() {
  if (myClonedEntity) {
    myClonedEntity->Clear();
    std::vector<Line>::const_iterator line;
    for (line = myLines.begin(); line != myLines.end(); ++line) {
      int size = line->myNext;
      int start = 0;
      if (line->myWrapped) {
        size = myNumValues;
        start = line->myNext;
      }
      if (size > 1) {
        int width = myClonedEntity->GetWidth();
        int height = myClonedEntity->GetHeight();
        int x = (int)(((myNumValues - size) * width) / myNumValues);
        int y =
            (int)(height - ((line->myValues[start] - line->myMin) * height) /
                               (line->myMax - line->myMin));
        while (--size) {
          ++start;
          if (start == myNumValues)
            start = 0;
          int x1 = ((myNumValues - size) * width) / myNumValues;
          int y1 =
              (int)(height - ((line->myValues[start] - line->myMin) * height) /
                                 (line->myMax - line->myMin));
          myClonedEntity->DrawLine(x, y, x1, y1, line->myRed, line->myGreen,
                                   line->myBlue);
          x = x1, y = y1;
        }
      }
    }
  }
}

CREATURES_IMPLEMENT_SERIAL(UIGenerative)

UIGenerative::UIGenerative()
    : myGenerationNumber(0), myStartString(""), myReplaceString(""), myDepth(0),
      myBranchLength(0), myPerturbBranchLength(0), myDelta(0.0),
      myPerturbDelta(0.0) {
  myType = (partPlain | partGenerative);
}

UIGenerative::UIGenerative(FilePath const &gallery, int baseimage,
                           int numImages, Vector2D &relPos, int relplane,
                           int numValues)
    : UIPartWithClonedImage(gallery, baseimage, numImages, relPos, relplane),
      myGenerationNumber(0), myStartString(""), myReplaceString(""), myDepth(0),
      myBranchLength(0), myPerturbBranchLength(0), myDelta(0.0),
      myPerturbDelta(0.0) {
  myType = (partPlain | partGenerative);
}

void UIGenerative::SetStartString(std::string startStr) {
  myStartString = startStr;
}

std::string UIGenerative::GetStartString() { return myStartString; }

void UIGenerative::SetReplaceString(std::string replaceStr) {
  myReplaceString = replaceStr;
}

std::string UIGenerative::GetReplaceString() { return myReplaceString; }

int UIGenerative::GetGenerationNumber() { return myGenerationNumber; }

void UIGenerative::SetBranchLength(int length) { myBranchLength = length; }

int UIGenerative::GetBranchLength() { return myBranchLength; }

void UIGenerative::SetPerturbBranchLength(int perturb) {
  myPerturbBranchLength = perturb;
}

int UIGenerative::GetPerturbBranchLength() { return myPerturbBranchLength; }

void UIGenerative::SetPerturbDelta(float perturb) { myPerturbDelta = perturb; }

float UIGenerative::GetPerturbDelta() { return myPerturbDelta; }

void UIGenerative::SetDelta(float delta) { myDelta = delta; }

float UIGenerative::GetDelta() { return myDelta; }

void UIGenerative::Iterate(int numIterations) {}

bool UIGenerative::Write(CreaturesArchive &archive) const {
  UIPartWithClonedImage::Write(archive);
  return true;
}

bool UIGenerative::Read(CreaturesArchive &archive) {
  int32 version = archive.GetFileVersion();
  if (version >= 3) {

    if (!UIPartWithClonedImage::Read(archive))
      return false;
  } else {
    _ASSERT(false);
    return false;
  }
  return true;
}

void UIGenerative::Draw() {
  if (myClonedEntity) {
  }
}

/*
CreaturesArchive &operator<<( CreaturesArchive &ar)
{
        return ar;
}

CreaturesArchive &operator>>( CreaturesArchive &ar)
{
        return ar;
}
*/

CreaturesArchive &operator<<(CreaturesArchive &ar, UIGraph::Line const &line) {
  ar << line.myRed << line.myGreen << line.myBlue << line.myMin << line.myMax
     << line.myNext << line.myWrapped << line.myValues;
  return ar;
}

CreaturesArchive &operator>>(CreaturesArchive &ar, UIGraph::Line &line) {
  ar >> line.myRed >> line.myGreen >> line.myBlue >> line.myMin >> line.myMax >>
      line.myNext >> line.myWrapped >> line.myValues;
  return ar;
}

CreaturesArchive &operator<<(CreaturesArchive &ar,
                             UITextPart::Line const &line) {
  ar << line.firstChar << line.lastChar << line.yTop;
  return ar;
}

CreaturesArchive &operator>>(CreaturesArchive &ar, UITextPart::Line &line) {
  ar >> line.firstChar >> line.lastChar >> line.yTop;
  return ar;
}

CreaturesArchive &operator<<(CreaturesArchive &ar,
                             UITextPart::Page const &page) {
  ar << page.firstLine << page.lastLine;
  return ar;
}

CreaturesArchive &operator>>(CreaturesArchive &ar, UITextPart::Page &page) {
  ar >> page.firstLine >> page.lastLine;
  return ar;
}

void UIPartWithClonedImage::Tint(const uint16 *tintTable, bool oneImage) {
  myClonedEntity->RecolourGallery(tintTable);
  base::Tint(tintTable);
}

void UIPartWithClonedImage::DrawAlphaBlended(bool drawAlpha, int intensity) {
  if (myClonedEntity) {
    myClonedEntity->DrawAlphaBlended(intensity, drawAlpha);
  }
  base::DrawAlphaBlended(drawAlpha, intensity);
}
