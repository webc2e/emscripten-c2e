
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "MapScanner.h"

#include <algorithm>
#include <fstream>
#include <iostream>


#include <sstream>

#include <imagehlp.h>
#include <windows.h>


MapScanner::MapScanner(const std::vector<std::string> &mapFiles,
                       const unsigned int *addressStart,
                       unsigned int addressCount,
                       std::vector<MapScanner::FunctionData> &functionDatum) {
  functionDatum.clear();
  functionDatum.resize(addressCount);

  for (int iMapFile = 0; iMapFile < mapFiles.size(); ++iMapFile) {
    std::string mapFile = mapFiles[iMapFile];

    // Check the mapfile is here, by trying to open it
    nofile = false;
    std::ifstream infile(mapFile.c_str());
    if (!infile.good()) {
      nofile = true;
      return;
    }

    // Scanning for Location in mapfile...
    std::string token;
    while (token != "Address" && infile.good()) {
      std::string aline;
      std::getline(infile, aline);
      std::istringstream instring(aline);
      instring >> token;
    }

    // Okay then, now we have had the address line, what's the next one?
    // Blank line
    std::string aline;
    std::getline(infile, aline);

    FunctionData lastFunctionData;
    while (infile.good()) {
      std::getline(infile, aline);
      std::istringstream instring(aline);
      instring >> token;

      // stop if we reach the fixups
      if (token == "FIXUPS:")
        return;

      // It is a code line I think.
      FunctionData functionData;
      std::string call;
      instring >> call;
      instring >> std::hex >> functionData.functionStart;
      // find end token on line as obj, dll or other label
      std::string lastToken = token;
      while (token != "") {
        lastToken = token;
        instring >> token;
      }
      functionData.objectFile = lastToken;
      functionData.mangledName = call;
      functionData.functionName = Unmangle(call);

      // Search for whether we've hit something since
      // the last line
      bool finished = true;
      for (int i = 0; i < addressCount; ++i) {
        unsigned int address = *(addressStart + i);

        if (!functionDatum[i].found) {
          finished = false;
          if (address >= lastFunctionData.functionStart &&
              address < functionData.functionStart) {
            functionDatum[i] = lastFunctionData;
            functionDatum[i].found = true;
          }
        }
      }
      if (finished)
        return;

      lastFunctionData = functionData;
    }
  }
}

// static
std::string MapScanner::Unmangle(std::string mangled) {
  if (mangled.empty())
    return mangled;

  char buffer[16384];
  int status =
      ::UnDecorateSymbolName(mangled.c_str(), buffer, 1024, UNDNAME_COMPLETE);
  if (status == 0)
    return mangled;
  else
    return std::string(buffer);
}

#endif
