// $Id: QuickFsp.h,v 1.2 2001/02/13 14:53:35 firving Exp $

// Speed up searching for Creature body parts.  

// Fsp is legacy terminology "File SPecification" or some such, from Creatures 1.
// It is where the mutable variables (age, breed, sex, genus) and part are
// encoded in a four character filename, which is also encoded as a 32 bit integer.

#include "../common/C2eTypes.h"

#include <string>
#include <map>
#include <vector>

namespace QuickFsp
{
	void ClearCache();

	DWORD ValidFsp(int part, int genus, int sex, int age, int breed,
		std::string extension, int dir, bool overlay);

	class MutableVariables
	{
	public:
		MutableVariables(int age, int breed, int sex, int genus);
		MutableVariables();
		int Distance(const MutableVariables& other) const;
		DWORD GetDword(int part) const;

	private:
		int myAge, myBreed, mySex, myGenus;
	};

	class ImmutableVariables
	{
	public:
		int myPart;
		std::string myExtension;
		int myDir;
		bool myOverlay;

		bool operator<(const ImmutableVariables& other) const;
	};

	class FspCache
	{
	public:
		FspCache();
		FspCache(const ImmutableVariables& immut);
		bool FindNearestMatch(const MutableVariables& from, MutableVariables& nearest) const;

	private:
		ImmutableVariables myImmut;
		std::vector<MutableVariables> myMutes;
	};

	static std::map<ImmutableVariables, FspCache > ourImmutables;
};


