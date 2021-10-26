#include "StringFuncs.h"
#include <ctype.h>

int CaseInsensitiveCompare( const char *l, const char *r )
{
	if( l == r ) return 0;
	if( !l ) return -1;
	if( !r ) return 1;
	while( *l )
	{
		// The cast to unsigned char is because tolower (and all
		// other similar C standard library functions) takes an
		// int as a parameter.  Since char is signed by default
		// in VC++, this causes problems with characters over 127.
		int diff = tolower( (unsigned char)*l ) - tolower( (unsigned char)*r );
		if( diff < 0 ) return -1;
		if( diff > 0 ) return 1;
		++l, ++r;
	}
	if (*r != 0)
		return -1;
	return 0;
}

void LowerCase(std::string& str)
{
	for (int i = 0; i < str.size(); ++i)
	{
		str[i] = tolower((unsigned char)(str[i]));
	}
}

void UpperCase(std::string& str)
{
	for (int i = 0; i < str.size(); ++i)
	{
		str[i] = toupper((unsigned char)(str[i]));
	}
}

