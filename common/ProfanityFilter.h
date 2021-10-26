// $Id: ProfanityFilter.h,v 1.2 2001/04/24 15:00:58 firving Exp $

// Removes swear words from text.

#ifndef PROFANITY_FILTER_H
#define PROFANITY_FILTER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include <vector>

#define theProfanityFilter ProfanityFilter::GetProfanityFilter()

class ProfanityFilter
{
public:
	ProfanityFilter();
	void Filter(std::string& text);

	static ProfanityFilter& GetProfanityFilter();

private:
	bool SingleFilter(std::string& text, const std::string& lower, const std::string& swearWord);

	void AddWord(const std::string& word);
	std::string GetReplacement(const std::string& word);

	std::vector<std::string> mySwearWords;

	static ProfanityFilter ourProfanityFilter;
};

#endif // PROFANITY_FILTER_H

