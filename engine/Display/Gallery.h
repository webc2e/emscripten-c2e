// --------------------------------------------------------------------------
// Filename:	Gallery.cpp
// Class:		Gallery
// Purpose:		This class holds a series of bitmaps for an entity or a
//				background.
//
//
//
// Description:In the case of entities the bitmaps are a series
//				of animations.  They can either be compressed or
//uncompressed. 				By default they share pixel information with other galleries of
//				the same name.  Memeory mapped files are used for
//these galleries. 				Cloned galleries need to hold all their bitmap data locally
//in 				an uncompressed state so that they can alter pixel data. 				These do not use
//memory mapped files.  Also we must be provided 				with the base image and the
//number of images that belong to this 				particular gallery since there may be
//many sets of related sprites 				in a sprite file.
//
//				In the case of backgrounds each bitmap is a
//tile.
//
// History:
// -------
// 23Feb2000	Alima			Created
// --------------------------------------------------------------------------
#ifndef GALLERY_H
#define GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../../common/BasicException.h"
#include "../PersistentObject.h"
#include "MemoryMappedFile.h"
#include <string>

const int SpriteFileHeaderSize = 8;
////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class Bitmap;
class CompressedBitmap;
class Sprite;
class EntityImage;
class ClonedGallery;

class Gallery : public PersistentObject {

public:
  // This is the threshold number of entities above which we will do a complete
  // screen redraw
  enum { MAX_DIRTY_RECTS = 500 };
  enum FileFormat { IAmUncompressed = 0, IAmCompressed, IAmTiled };

  ////////////////////////////////////////////////////////////////////////////
  // Constructors and destructor
  ////////////////////////////////////////////////////////////////////////////
  Gallery();
  Gallery(std::string const &name, uint32 baseimage = 0, uint32 numImages = 0,
          bool clone = false, uint32 defaultBitmapWidth = 10,
          uint32 defaultBitmapHeight = 10, bool writeAccess = false);

  Gallery(ClonedGallery &gallery);

  Gallery(std::string const &name, MemoryMappedFile::FileMapping memoryFile,
          uint32 highOffset, uint32 lowOffset, uint32 numBytesToMap);

  void SetUpGalleryShellForCloning(std::string path, uint32 baseImage,
                                   uint32 pixelFormat, uint32 numBitmaps);

  virtual ~Gallery();

  void Trash();

  ////////////////////////////////////////////////////////////////////////////
  // Loading
  ////////////////////////////////////////////////////////////////////////////
private:
  bool InitialiseBitmaps(bool writeAccess = false);
  bool InitialiseClonedBitmaps();
  bool InitialiseCompressedBitmaps();
  bool InitialiseTiledBitmaps(bool writeAccess = false);

public:
  bool LoadFromC16(std::string const &fileName);

  bool ReloadFromC16(uint32 range_start, uint32 range_end);

  //	bool LoadFromS16(std::string fileName);

  ////////////////////////////////////////////////////////////////////////////
  // Background Tiles support
  ////////////////////////////////////////////////////////////////////////////
public:
  void StartTileCount() { ClearTileArray(); }
  void EndTileCount() { ClearTileArray(); }

  void ClearTileArray() { myDrawnTilesMap.clear(); }

  Bitmap *GetTile(uint32 index);

  uint32 GetTileHeight() { return myTileHeight; }

  uint32 GetTileWidth() { return myTileWidth; }

  ////////////////////////////////////////////////////////////////////////////
  // Altering the images in some way
  ////////////////////////////////////////////////////////////////////////////
  bool ConvertTo(uint32 format);

  bool DecompressC16(File &file);

  void Recolour(const uint16 tintTable[65536]);

  void Recolour(const uint16 tintTable[65536], uint32 range_start,
                uint32 range_end);

  void ScaleGallery(float scaleby);

  ////////////////////////////////////////////////////////////////////////////
  // Querying
  ////////////////////////////////////////////////////////////////////////////

  int32 ClonedAbsoluteBase() const;

  void DecrementReferenceCount() { myReferenceCount--; }

  Bitmap *GetBitmap(uint32 index);

  int32 GetBitmapWidth(uint32 index);

  int32 GetBitmapHeight(uint32 index);

  uint32 GetCount() { return myNumberOfBitmaps; }

  std::string GetName() { return myName; }
  void SetName(std::string const &newName) { myName = newName; }

  uint32 GetPixelFormat() { return myPixelFormat; }

  FileFormat GetFileFormat() { return myFileFormat; }

  void IncrementReferenceCount() { myReferenceCount++; }

  bool IsPixelTransparent(uint32 x, uint32 y, int32 imageIndex);

  bool IsUsed() { return myReferenceCount != 0 ? true : false; }

  bool IsValid();

  bool ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight);

  void ResetReferenceCount() { myReferenceCount = 0; }

  void SetFileSpec(uint32 uniqueID) { myFileSpec = uniqueID; }
  uint32 GetFileSpec() { return myFileSpec; }

  bool AreYouCloned() { return myAmIClonedFlag; }

  ////////////////////////////////////////////////////////////////////////////
  // Saving
  ////////////////////////////////////////////////////////////////////////////
  uint32 Save(MemoryMappedFile &file);
  uint32 Save(File &file);
  uint32 SaveCompressedFile(MemoryMappedFile &file);
  uint32 SaveCompressedFile(File &file);
  uint32 SaveCompressedFileInALocalWorldFolder(std::string &filename);
  uint32 SaveCompressedFile();
  //
  uint32 SaveUncompressedFile(MemoryMappedFile &file);
  uint32 SaveUncompressedFile(File &file);
  uint32 SaveUncompressedFileInALocalWorldFolder(std::string &filename);
  uint32 SaveUncompressedFile();
  //
  uint32 SaveTiledFile(MemoryMappedFile &file);

  virtual bool Read(CreaturesArchive &archive);
  //	virtual bool Write(CreaturesArchive &archive) const;

  //////////////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////////////
  class GalleryException : public BasicException {
  public:
    GalleryException(std::string what, uint32 line)
        : BasicException(what.c_str()), lineNumber(line) {
      ;
    }

    uint32 LineNumber() { return lineNumber; }

  private:
    uint32 lineNumber;
  };

private:
  // Copy constructor and assignment operator
  // Declared but not implemented
  Gallery(const Gallery &);
  Gallery &operator=(const Gallery &);

  CompressedBitmap *myCompressedBitmaps;

  MemoryMappedFile myMemoryMappedFile;

  bool myAmIClonedFlag;

  uint32 myReferenceCount;

  uint32 myTileWidth;
  uint32 myTileHeight;

  // no need to serialise
  std::map<uint32, bool> myDrawnTilesMap;

  FileFormat myFileFormat;

  std::string myName;

  uint32 myFileSpec;

protected:
  // for backwards compatibility with cloned gallery
  // make this protected
  uint32 myBaseImage;
  uint32 myPixelFormat;
  uint32 myNumberOfBitmaps;
  Bitmap *myBitmaps;
};

#endif // GALLERY_H
