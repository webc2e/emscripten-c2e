// $Id: UniqueIdentifier.h,v 1.3 2001/04/24 15:01:04 firving Exp $

#ifndef UNIQUE_IDENTIFIER_H
#define UNIQUE_IDENTIFIER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
std::string GenerateUniqueIdentifier(const std::string& extraFood1, const std::string& extraFood2);
std::string TryGenerateUniqueMoniker(const std::string& monikerSeed1, const std::string& monikerSeed2, int genus);

#endif

