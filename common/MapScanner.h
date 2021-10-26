#ifndef MAPSCANNER_H
#define MAPSCANNER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include <vector>

// With the linker option /MAPINFO:LINES it would be possible to
// extend this class to give the exact line number and file which
// the error happened at

class MapScanner
{
public:
	struct FunctionData
	{
		unsigned int functionStart;
		std::string functionName;
		std::string mangledName;
		std::string objectFile;
		bool found;

		bool operator<(const FunctionData& other)
		{
			return functionStart < other.functionStart;
		}
		FunctionData::FunctionData()
		{
			found = false;
		}
	};

	MapScanner(const std::vector<std::string>& mapFiles, const unsigned int* addressStart, unsigned int addressCount, std::vector<FunctionData>& functionDatum);
	bool nofile;

private:
	static std::string Unmangle(std::string mangled);
};

#endif

