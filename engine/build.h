#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#define MAJOR_VER 2
#define MINOR_VER 296


#include <sstream>

inline std::string GetEngineVersion()
{
	std::ostringstream out;
	out << MAJOR_VER << "." << MINOR_VER;
	return out.str();
}

inline int GetMinorEngineVersion()
{
	return MINOR_VER;
}

inline int GetMajorEngineVersion()
{
	return MAJOR_VER;
}

