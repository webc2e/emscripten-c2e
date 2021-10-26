// $Id: UniqueIdentifier.cpp,v 1.4 2000/05/22 14:22:41 firving Exp $

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "UniqueIdentifier.h"
#include "md5.h"
#include <iomanip>
#include <sstream>


#ifdef _WIN32
#include <windows.h>
#endif

#ifndef MD5_NO_GAME_ENTROPY
#include "TimeFuncs.h"                   // GetTimeStamp
#include "Display/ErrorMessageHandler.h" // FormatMessageFooter
#include "AgentManager.h"                // Get coutns of agents
#include "App.h"                         // theApp
#include "Maths.h"                       // Rnd
#include "World.h"                       // World tick
#else
#include <stdio.h>
#endif

// This is not a cryptographically secure routine - only useful
// to provide a good chance of uniqueness.
std::string GenerateUniqueIdentifier(const std::string &extraFood1,
                                     const std::string &extraFood2) {
  // Set up MD5
  md5_state_t state;
  md5_init(&state);

  // Feed MD5 various data to be munged, to guarantee universal uniqueness ...

  // ... extra strings passed in (parent genomes, for monikers)
  md5_append(&state, (const md5_byte_t *)extraFood1.data(), extraFood1.size());
  md5_append(&state, (const md5_byte_t *)extraFood2.data(), extraFood2.size());

#ifndef MD5_NO_GAME_ENTROPY
  // ... time stamp
  int timeStamp = GetTimeStamp();
  md5_append(&state, (const md5_byte_t *)&timeStamp, sizeof(timeStamp));

  // ... error message header has date and user name in it
  std::string errorHeader = ErrorMessageHandler::ErrorMessageFooter();
  md5_append(&state, (const md5_byte_t *)errorHeader.data(),
             errorHeader.size());

  // ... number of agents in the world
  AgentList aList;
  theAgentManager.FindByFGS(aList, Classifier(0, 0, 0));
  int agentCount = aList.size();
  md5_append(&state, (const md5_byte_t *)&agentCount, sizeof(agentCount));

  // ... system and world ticks
  uint32 systemTick = theApp.GetSystemTick();
  md5_append(&state, (const md5_byte_t *)&systemTick, sizeof(systemTick));
  uint32 worldTick = theApp.GetWorld().GetWorldTick();
  md5_append(&state, (const md5_byte_t *)&worldTick, sizeof(worldTick));

  // ... mouse position and velocity
  int mopX = theApp.GetInputManager().GetMouseX();
  md5_append(&state, (const md5_byte_t *)&mopX, sizeof(mopX));
  int mopY = theApp.GetInputManager().GetMouseY();
  md5_append(&state, (const md5_byte_t *)&mopY, sizeof(mopY));
  float movX = theApp.GetInputManager().GetMouseVX();
  md5_append(&state, (const md5_byte_t *)&movX, sizeof(movX));
  float movY = theApp.GetInputManager().GetMouseVY();
  md5_append(&state, (const md5_byte_t *)&movY, sizeof(movY));

  // ... current pace
  float tickFactor = theApp.GetTickRateFactor();
  md5_append(&state, (const md5_byte_t *)&tickFactor, sizeof(tickFactor));

  // ... some stuff from our random number generator
  for (int i = 0; i < Rnd(200, 300); ++i) {
    md5_byte_t randomByte = Rnd(0, 255);
    md5_append(&state, &randomByte, sizeof(randomByte));
  }

  // ... high performance time stamp (later than time stamp, so more likely to
  // be different)
  int64 highTimeStamp = GetHighPerformanceTimeStamp();
  md5_append(&state, (const md5_byte_t *)&highTimeStamp, sizeof(highTimeStamp));

#endif

  // ... platform dependent information (hardware profiles etc.)

  // Get entropy from the kernel - same amount as digest size, 16 bytes
  const int entropyBytes = 16;
  char kernelEntropy[entropyBytes];

  // Use urandom as we don't need to be cryptographically secure
  FILE *f = fopen("/dev/urandom", "r");
  if (f != NULL) {
    fread(kernelEntropy, 1, entropyBytes, f);
    md5_append(&state, (const md5_byte_t *)kernelEntropy, entropyBytes);
    fclose(f);
  } else {
    std::cout << "Couldn't find /dev/urandom" << std::endl;
  }

  // Retrieve our digest
  md5_byte_t digest[16];
  md5_finish(&state, digest);

  unsigned int i1 =
      digest[0] + (digest[1] << 8) + (digest[2] << 16) + (digest[3] << 24);
  unsigned int i2 =
      digest[4] + (digest[5] << 8) + (digest[6] << 16) + (digest[7] << 24);
  unsigned int i3 =
      digest[8] + (digest[9] << 8) + (digest[10] << 16) + (digest[11] << 24);
  unsigned int i4 =
      digest[12] + (digest[13] << 8) + (digest[14] << 16) + (digest[15] << 24);

  std::string uniqueId;

  // We need some way to extract 5 entries from this dictionary...

  std::string dictionary =
      "abcdefghjklmnpqrstuvwxyz23456789"; // Length is 32 :)

  int loopy;
  for (loopy = 0; loopy < 5; loopy++) {
    uniqueId += dictionary.at(i1 % 31);
    i1 >>= 5;
  }
  uniqueId += "-";
  for (loopy = 0; loopy < 5; loopy++) {
    uniqueId += dictionary.at(i2 % 31);
    i2 >>= 5;
  }
  uniqueId += "-";
  for (loopy = 0; loopy < 5; loopy++) {
    uniqueId += dictionary.at(i3 % 31);
    i3 >>= 5;
  }
  uniqueId += "-";
  for (loopy = 0; loopy < 5; loopy++) {
    uniqueId += dictionary.at(i4 % 31);
    i4 >>= 5;
  }

  return uniqueId;
}

// Statistically unique across all computers anywhere
std::string TryGenerateUniqueMoniker(const std::string &monikerSeed1,
                                     const std::string &monikerSeed2,
                                     int genus) {
#ifndef MD5_NO_GAME_ENTROPY
  std::ostringstream friendlyTagSream;
  friendlyTagSream << "Moniker Friendly Names " << genus + 1;
  std::string friendlyTag = friendlyTagSream.str();

  if (!theCatalogue.TagPresent(friendlyTag))
    friendlyTag = "Moniker Friendly Names";

  int friendlyMax = theCatalogue.GetArrayCountForTag(friendlyTag);
  int friendly = Rnd(0, friendlyMax - 1);
  std::string friendlyPart = theCatalogue.Get(friendlyTag, friendly);
#else
  std::string friendlyPart = "test";
#endif

  // ensure _exactly_ four characters
  friendlyPart =
      friendlyPart + friendlyPart + friendlyPart + friendlyPart + "xxxx";
  friendlyPart.assign(friendlyPart, 0, 4);

  // Next we need a generation number :)
  int generationOne = 0, generationTwo = 0;
  {
    std::istringstream monikerStream1(monikerSeed1);
    monikerStream1 >> generationOne;
    std::istringstream monikerStream2(monikerSeed2);
    monikerStream2 >> generationTwo;
  }
  // Generation number defined as Max+1 (Because Helen Birchmore (Yes Blame
  // her!) said so :)
  int ownGeneration =
      (generationOne < generationTwo) ? generationTwo + 1 : generationOne + 1;
  // special case for cloning to preserve generation
  if (!monikerSeed1.empty() && monikerSeed2.empty())
    ownGeneration = generationOne;

  std::string moniker = GenerateUniqueIdentifier(monikerSeed1, monikerSeed2);
  std::ostringstream out;
  out << std::setfill('0') << std::setw(3) << ownGeneration;
  return out.str() + "-" + friendlyPart + "-" + moniker;
}
