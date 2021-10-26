// Class to provide general self-documentation of things
// in the engine.  Anything in the engine can have a documentation
// table, which statically calls RegisterTable.  The table
// will then appear in the CAOS documentation which the
// engine streams out.

#ifndef AUTO_DOCUMENTATION_TABLE_H
#define AUTO_DOCUMENTATION_TABLE_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "TableSpec.h"

class AutoDocumentationTable
{
public:
	static int RegisterTable(TableSpec *table, int sizeInBytes);

	static void StreamTitleLinksAsHTML(std::ostream& out);
	static void StreamAllTablesAsHTML(std::ostream& out);

protected:
	static bool SortComparison(std::pair<TableSpec *, int> a, std::pair<TableSpec *, int> b);
	static void SortTablesByTitle();

	static void StreamTableAsHTML(std::ostream& out, TableSpec *table, int size);

	typedef std::vector< std::pair<TableSpec *, int> > TableSpecTable;
	static TableSpecTable& GetOurTables();
};


#endif // AUTO_DOCUMENTATION_TABLE_H

