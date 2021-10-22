#ifndef UNIX_MEMORY_MAPPED_FILE_H
#define UNIX_MEMORY_MAPPED_FILE_H

#include "../../../common/C2eTypes.h"
#include "../../File.h" // for flag defs
#include <string.h>     // for memcpy()

class MemoryMappedFile {
public:
  // constructors
  MemoryMappedFile();

  MemoryMappedFile(const std::string &filename,
                   uint32 desiredAccessFlags = GENERIC_READ | GENERIC_WRITE,
                   uint32 sharemodeFlags = FILE_SHARE_READ | FILE_SHARE_WRITE);

  virtual ~MemoryMappedFile();

  void Open(const std::string &name,
            uint32 desiredAccessFlags = GENERIC_READ | GENERIC_WRITE,
            uint32 sharemodeFlags = FILE_SHARE_READ | FILE_SHARE_WRITE,
            uint32 fileSize = 0);
  void Close();

  inline void ReadUINT8Ptr(uint8_t *&dataBufferPtr);
  inline void ReadUINT32Ptr(uint32 *&dataBufferPtr);
  inline void ReadUINT16Ptr(uint16 *&dataBufferPtr);

  inline uint32 *GetUINT32Ptr();

  inline uint8_t ReadUINT8();
  inline uint16 ReadUINT16();
  inline uint32 ReadUINT32();

  inline void WriteUINT8(uint8_t value);
  inline void WriteUINT16(uint16 value);
  inline void WriteUINT32(uint32 value);

  inline void Blast(uint32 countInBytes, uint8_t *source);
  inline void BlockCopy(uint32 countInBytes, uint8_t *dest);

  //	void Seek(uint32 numBytes,){myPosition += numBytes;}

  void Seek(int32 moveBy, File::FilePosition from);

  unsigned char *GetFileStart();
  //	HANDLE GetFileMapping(){return myMemoryFile;}
  void Reset() { myPosition = 0; }
  uint32 GetPosition() { return myPosition; }
  bool Valid() { return (myFile != NULL); }

  //////////////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////////////
  class MemoryMappedFileException : public BasicException {
  public:
    MemoryMappedFileException(std::string what, uint16 line)
        : BasicException(what.c_str()), lineNumber(line) {
      ;
    }

    uint16 LineNumber() { return lineNumber; }

  private:
    uint16 lineNumber;
  };

  // Copy constructor and assignment operator
  // Declared but not implemented
  MemoryMappedFile(const MemoryMappedFile &);
  MemoryMappedFile &operator=(const MemoryMappedFile &);

private:
  int myFile;
  uint8_t *myBasePtr;
  uint32 myLength;
  uint32 myPosition;
};

inline uint32 *MemoryMappedFile::GetUINT32Ptr() {
  return (uint32 *)(myBasePtr + myPosition);
}

inline void MemoryMappedFile::ReadUINT8Ptr(uint8_t *&dataBufferPtr) {
  dataBufferPtr = (uint8_t *)(myBasePtr + myPosition);
  myPosition += sizeof(uint8_t);
}

inline void MemoryMappedFile::ReadUINT32Ptr(uint32 *&dataBufferPtr) {
  dataBufferPtr = (uint32 *)(myBasePtr + myPosition);
  myPosition += sizeof(uint32);
}

inline void MemoryMappedFile::ReadUINT16Ptr(uint16 *&dataBufferPtr) {
  dataBufferPtr = (uint16 *)(myBasePtr + myPosition);
  myPosition += sizeof(uint16);
}

inline uint8_t MemoryMappedFile::ReadUINT8() {
  uint8_t *p = (uint8_t *)(myBasePtr + myPosition);
  myPosition += sizeof(uint8_t);
  return *p;
}

inline uint16 MemoryMappedFile::ReadUINT16() {
  uint16 *p = (uint16 *)(myBasePtr + myPosition);
  myPosition += sizeof(uint16);
  return *p;
}

inline uint32 MemoryMappedFile::ReadUINT32() {
  uint32 *p = (uint32 *)(myBasePtr + myPosition);
  myPosition += sizeof(uint32);
  return *p;
}

inline void MemoryMappedFile::WriteUINT8(uint8_t value) {
  uint8_t *p = (uint8_t *)(myBasePtr + myPosition);
  *p = value;
  myPosition += sizeof(uint8_t);
}

inline void MemoryMappedFile::WriteUINT16(uint16 value) {
  uint16 *p = (uint16 *)(myBasePtr + myPosition);
  *p = value;
  myPosition += sizeof(uint16);
}

inline void MemoryMappedFile::WriteUINT32(uint32 value) {
  uint32 *p = (uint32 *)(myBasePtr + myPosition);
  *p = value;
  myPosition += sizeof(uint32);
}

inline void MemoryMappedFile::Blast(uint32 countInBytes, uint8_t *source) {
  memcpy((uint8_t *)(myBasePtr + myPosition), source, countInBytes);
  myPosition += countInBytes;
}

inline void MemoryMappedFile::BlockCopy(uint32 countInBytes, uint8_t *dest) {
  memcpy(dest, (uint8_t *)(myBasePtr + myPosition), countInBytes);
  myPosition += countInBytes;
}

#endif // UNIX_MEMORY_MAPPED_FILE
