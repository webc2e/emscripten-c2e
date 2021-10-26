// $Id: ProfanityFilter.cpp,v 1.7 2002/03/01 17:20:13 mstamps Exp $

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ProfanityFilter.h"
#include "StringFuncs.h"
#include "C2eDebug.h"

#ifdef LINUX_PORT
#include <ctype.h>
#endif

ProfanityFilter ProfanityFilter::ourProfanityFilter;

ProfanityFilter& ProfanityFilter::GetProfanityFilter()
{
	return ourProfanityFilter;
}
	
ProfanityFilter::ProfanityFilter()
{
	AddWord("fuck");
}

void ProfanityFilter::AddWord(const std::string& word)
{
	ASSERT(word.size() >= 2);
	mySwearWords.push_back(word);
}

std::string ProfanityFilter::GetReplacement(const std::string& word)
{
	if (word == "fuck")
		return "kisspop";

	static const std::string chars = "@@##!";
	std::string exclamified = word;
	for (int i = 0; i < exclamified.size(); ++i)
	{
		// Deterministic way of changing to the swear-characters
		exclamified[i] = chars[exclamified[i] % chars.size()];
	}

	return exclamified;
}

void ProfanityFilter::Filter(std::string& text)
{
	// Filter each word
	for (int i = 0; i < mySwearWords.size(); ++i)
	{
		// Repeatedly until every occurrence has gone
		bool changed = false;
		do
		{
			std::string lower = text;
			LowerCase(lower);
			changed = SingleFilter(text, lower, mySwearWords[i]);
		}
		while (changed);
	}
}


bool ProfanityFilter::SingleFilter(std::string& text, const std::string& lower, const std::string& swearWord)
{
	ASSERT(text.size() == lower.size());

	std::string::size_type start = lower.find(swearWord);
	if (start == std::string::npos)
		return false;

	// We've found the swear word - check its case
	ASSERT(swearWord.size() >= 2);
	bool initialCaps = isupper((unsigned char)(text[start])) ? true : false;
	bool secondCaps = isupper((unsigned char)(text[start + 1])) ? true : false;

	// Make replacement the right case
	// (roughly, as sizes may differ - we assume either an initial
	// capital, no capitals or entirely capitals)
	std::string replacement = GetReplacement(swearWord);
	ASSERT(replacement.size() >= 2);
	if (initialCaps && secondCaps)
		UpperCase(replacement);
	else if (initialCaps)
		replacement[0] = toupper((unsigned char)(replacement[0]));
	
	// Perform the change
	text.replace(start, swearWord.size(), replacement);

	return true;
}

