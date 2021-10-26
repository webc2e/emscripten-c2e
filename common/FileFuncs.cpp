// BenC 30Dec99

#include "FileFuncs.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32) || defined(WINELIB)

#include <windows.h>

// We need FileExists for Win32 - the
// others are standard Windows functions
bool FileExists(const char *filename) {
  int attributes = GetFileAttributes(filename);
  return attributes == -1 ? false : true;
}

#else

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>


// win32 replacement
bool DeleteFile(const char *filename) {
  int i = unlink(filename);
  return i == 0 ? true : false;
}

// win32 replacement
// hmmm... thought there would be a better way...
bool CopyFile(const char *src, const char *dest, bool overwrite) {
  int infd = -1;
  int outfd = -1;
  void *p = NULL;
  struct stat statbuf;
  bool success = false; // positive attitude.
  int flags;

  // open input file and map it into memory
  infd = open(src, O_RDONLY);
  if (infd == -1)
    goto cleanup;

  if (fstat(infd, &statbuf) != 0)
    goto cleanup;

  p = mmap(0, statbuf.st_size, PROT_READ, MAP_PRIVATE, infd, 0);
  if (p == MAP_FAILED)
    goto cleanup;

  // create output file
  flags = O_CREAT | O_WRONLY;
  if (overwrite)
    flags |= O_TRUNC;
  else
    flags |= O_EXCL; // fail if file exists

  outfd = open(dest, flags, S_IRWXU); // make new file writeable by user
  if (outfd == -1)
    goto cleanup;

  // blam.
  if (write(outfd, p, statbuf.st_size) == statbuf.st_size)
    success = true;

cleanup:
  if (outfd != -1)
    close(outfd);
  if (p)
    munmap(p, statbuf.st_size);
  if (infd != -1)
    close(infd);
  if (!success)
    perror("CopyFile failed");

  return success;
}

bool CreateDirectory(const char *name, void *ignored) {
  int i = mkdir(name, 0xffff);
  return i == 0 ? true : false;
}

// Note: If the destination file is already there, this
// could fail (it definitely does in Win32's built in version)
// So, DeleteFile first.
bool MoveFile(const char *src, const char *dest) {
  int i = rename(src, dest);
  return i == 0 ? true : false;
}

bool FileExists(const char *filename) {
  struct stat buf;
  int i = stat(filename, &buf); // SPARKY was stat
  return i == 0 ? true : false;
}

#endif // _WIN32

int FileSize(const char *filename) {
  struct stat buf;
  int i = stat(filename, &buf);
  if (i == -1)
    return -1;
  return buf.st_size;
}

#ifdef _WIN32
char pathSeparator[] = "\\";
#else
char pathSeparator[] = "/";
#endif

char *PathSeparator() { return pathSeparator; }

void AddTrailingPathSeparator(std::string &path) {
  if (!path.empty()) {
    if (path[path.size() - 1] != '\\' && path[path.size() - 1] != '/')
      path.append(PathSeparator());
  }
}

void GetLineCompatibleMunch(std::istream &in, char *buf, int len) {
  // Read a line (up to line feed \n), stripping spaces from
  // the start
  bool afterInitialSpace = false;
  int offset = 0;
  char ch;
  do {
    ch = in.get();
    if (ch >= 0 && ch != '\n' &&
        (afterInitialSpace ||
         (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r'))) {
      afterInitialSpace = true;
      buf[offset] = ch;
      offset++;
    }
  } while ((in.good()) && (offset < len) &&
           ((ch != '\n') || !afterInitialSpace));
  buf[offset] = 0;

  // Remove trailing carriage return \r
  int actualLen = strlen(buf);
  if (actualLen > 0) {
    if (buf[actualLen - 1] == '\r')
      buf[actualLen - 1] = 0;
  }
}

void GetLineCompatible(std::istream &in, std::string &str) {
  std::getline(in, str);
  if (str.size() > 0)
    if (str[str.size() - 1] == '\r')
      str = str.substr(0, str.size() - 1);
}

std::string FileExtension(const std::string &filename) {
  std::string::size_type posTo = filename.find_last_of(".");
  if (posTo == std::string::npos)
    return "";

  std::string extension = filename.substr(posTo + 1, filename.size());

  return extension;
}

void MakeFilenameSafe(std::string &filename) {
  for (int index = 0; index < filename.length(); index++)
    switch (filename.at(index)) {
    case '\\':
    case '/':
    case '?':
    case '*':
    case '\"':
    case '<':
    case '>':
    case '|':
    case ':':
      filename.at(index) = '_';
    }

  // strip trailing spaces and dots - Windows can't take them
  while (!filename.empty() && (filename[filename.size() - 1] == ' ' ||
                               filename[filename.size() - 1] == '.'))
    filename = filename.substr(0, filename.size() - 1);

  // strip initial spaces and dots - Windows can't take them and they
  // get confused with .. and . directories.
  while (!filename.empty() && (filename[0] == '.' || filename[0] == ' '))
    filename = filename.substr(1, filename.size() - 1);
}
