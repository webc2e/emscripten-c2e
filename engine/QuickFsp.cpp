// $Id: QuickFsp.cpp,v 1.4 2001/04/24 15:01:04 firving Exp $

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "QuickFsp.h"
#include "General.h" // GetFilesInDirectory
#include "../common/FileFuncs.h"

// std::map<QuickFsp::ImmutableVariables, QuickFsp::FspCache > QuickFsp::ourImmutables;
// ****************************************************************************
void QuickFsp::ClearCache()
{
	ourImmutables.clear();
}
// ****************************************************************************
bool QuickFsp::ImmutableVariables::operator<(const ImmutableVariables& other) const
{
	if (myPart != other.myPart)
		return myPart < other.myPart;
	if (myExtension != other.myExtension)
		return myExtension < other.myExtension;
	if (myDir != other.myDir)
		return myDir < other.myDir;
	if (myOverlay != other.myOverlay)
		return myOverlay < other.myOverlay;

	// all equal
	return false;
}
// ****************************************************************************
DWORD QuickFsp::ValidFsp(int part, int genus, int sex, int age, int breed,
			std::string extension, int dir, bool overlay)
{
	sex -= 1; // we use zero indexed gender internally (as in fsp filenames!)
	if (part < 0)
		part = 0;
	part = part % 26;

	MutableVariables mut(age, breed, sex, genus);

	ImmutableVariables immut;
	immut.myPart = part;
	immut.myExtension = extension;
	immut.myDir = dir;
	immut.myOverlay = overlay;

	if (ourImmutables.find(immut) == ourImmutables.end())
		ourImmutables[immut] = FspCache(immut);

	MutableVariables result;
	bool ok = ourImmutables[immut].FindNearestMatch(mut, result);
	if (ok)
	{
#ifdef _DEBUG
		// Backcheck: The file should be genuinely there
		std::string file = BuildFsp(result.GetDword(part),extension.c_str(),dir,overlay);
		ASSERT(FileExists(file.c_str()));
#endif
		return result.GetDword(part);
	}
	else
		return 0;
}
// ****************************************************************************
QuickFsp::MutableVariables::MutableVariables()
{
}
// ****************************************************************************
QuickFsp::MutableVariables::MutableVariables(int age, int breed, int sex, int genus)
{
	// Validate range
	if (genus < 0)
		genus = 0;
	genus = genus % 4;
	if (sex < 0)
		sex = 0;
	if (sex > 1)
		sex = 1;
	if (age < 0)
		age = 0;
	age = age % 10;
	if (breed < 0)
		breed = 0;
	breed = breed % 26;

	myAge = age;
	myBreed = breed;
	mySex = sex;
	myGenus = genus;
}
// ****************************************************************************
// How good a match to us is other?
// The larger the return value, the worse a match
int QuickFsp::MutableVariables::Distance(const MutableVariables& other) const
{
	int distance = 0;

	// Genus: look down then up
	if (other.myGenus <= myGenus)
		distance += myGenus - other.myGenus;
	else
		distance += other.myGenus;
	
	// Allow space 256 for breed (maximum that can fit in an fsp,
	// in case we decide to have more than 26 later, by making five
	// letter filenames)
	distance *= 256;
	// Breed: lower ones better, then higher ones (as before this code)
	if (other.myBreed <= myBreed)
		distance += myBreed - other.myBreed;
	else
		distance += other.myBreed;

	// Allow space 2 for sex
	distance *= 2;
	// Sex: Our own sex is nearer than the other
	if (other.mySex != mySex)
		distance += 1;

	// Allow space 10 for age
	distance *= 10;
	// Age: decreasingly lower ones are better, then higher
	if (other.myAge <= myAge)
		distance += myAge - other.myAge;
	else
		distance += other.myAge;

	return distance;
}
// ****************************************************************************
DWORD QuickFsp::MutableVariables::GetDword(int part) const
{
	int genusSex = myGenus + mySex * 4;
	DWORD fsp = (part+'a') | 
				((genusSex+'0')<<8) | 
				((myAge+'0')<<16) | 
				((myBreed+'a')<<24);
	return fsp;
}
// ****************************************************************************
QuickFsp::FspCache::FspCache()
{
}
// ****************************************************************************
QuickFsp::FspCache::FspCache(const ImmutableVariables& immut)
{
	myImmut = immut;

	// Find wildcard to simplify search for files matching the immutable variables
	std::string wildcard;
	wildcard = (myImmut.myPart+'a');
	wildcard += "???";
	std::string overlayPrefix = "over_";
	if (myImmut.myOverlay)
		wildcard = overlayPrefix + wildcard;
	wildcard += "." + myImmut.myExtension;

	// Get all the files
	std::vector<std::string> files;
	{
		for (int i = 0; i < theDirectoryManager.GetAuxDirCount(myImmut.myDir); ++i)
		{
			GetFilesInDirectory(theDirectoryManager.GetAuxDir(myImmut.myDir, i ), files, wildcard);
		}

		GetFilesInDirectory(theDirectoryManager.GetDirectory(IMAGES_DIR,true), files, wildcard);
		GetFilesInDirectory(theDirectoryManager.GetDirectory(GENETICS_DIR,true), files, wildcard);
		GetFilesInDirectory(theDirectoryManager.GetDirectory(BODY_DATA_DIR,true), files, wildcard);
		GetFilesInDirectory(theDirectoryManager.GetDirectory(SOUNDS_DIR,true), files, wildcard);
		  

	}

	// Make a mutable variable for each one, adding it to our array
	for (int i = 0; i < files.size(); ++i)
	{
		std::string filename = files[i];

		// Strip it down to basic four character part of filename
		if (myImmut.myOverlay)
			filename = filename.substr(overlayPrefix.size(), filename.size() - overlayPrefix.size());
		filename = filename.substr(0, filename.size() - myImmut.myExtension.size() - 1);
		ASSERT(filename.size() == 4);

		// Convert to age, breed, sex, genus
		int age, breed, sex, genus;
		int genusSex = filename[1] - '0';
		if (genusSex >= 0 && genusSex < 8)
		{
			sex = genusSex / 4;
			genus = genusSex % 4;
			ASSERT(genusSex == genus + sex * 4);

			age = filename[2] - '0';
			if (age >= 0 && age <= 9)
			{
				breed = filename[3] - 'a';
				if (breed >= 0 && breed < 26)
				{
					// Add to our list of mutable variables
					MutableVariables mute(age, breed, sex, genus);
					myMutes.push_back(mute);
				}
				else
					ASSERT(true); // debug test (should be non-creature four letter files, put a breakpoint here to check)
			}
			else
				ASSERT(true); // debug test (should be non-creature four letter files, put a breakpoint here to check)
		}
		else
			ASSERT(true); // debug test (should be non-creature four letter files, put a breakpoint here to check)
	}
}
// ****************************************************************************
bool QuickFsp::FspCache::FindNearestMatch(const MutableVariables& from, MutableVariables& nearest) const
{
	int leastDistance = -1;
	int leastIndex = -1;
#ifdef _DEBUG
	std::string leastS;
#endif
	for (int i = 0; i < myMutes.size(); ++i)
	{
		const MutableVariables& other = myMutes[i];
		int distance = from.Distance(other);
#ifdef _DEBUG
		std::string fromS = BuildFsp(from.GetDword(myImmut.myPart),myImmut.myExtension.c_str(),myImmut.myDir,myImmut.myOverlay);
		std::string otherS = BuildFsp(other.GetDword(myImmut.myPart),myImmut.myExtension.c_str(),myImmut.myDir,myImmut.myOverlay);
#endif

		if (leastDistance == -1 || distance < leastDistance)
		{
			leastDistance = distance;
			leastIndex = i;
#ifdef _DEBUG
			leastS = otherS;
#endif
		}
	}

	if (leastDistance == -1 || leastIndex == -1)
	{
		ASSERT(leastDistance == leastIndex);
		return false;
	}

	nearest = myMutes[leastIndex];
	return true;
}
// ****************************************************************************

