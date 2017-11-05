#ifndef TAGFSTOOL
#define TAGFSTOOL

#include <sqlite/connection.hpp>
#include <sqlite/execute.hpp>
#include <sqlite/query.hpp>
#include <sqlite3.h>
#include <stdlib.h>     /* getenv */
#include <xxhash.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <stdlib.h>
#include <linux/limits.h>

#include "common.h"

typedef  boost::int64_t int64;
enum class PrintType{All,AllTaged, Found, NotFound};
enum class  Contain{ Path, Hash, Both, Either};
class tool
{
public:
	tool(sqlite::connection &db, int argc, char  **argv);
	// ~tool();
private:

	boost::shared_ptr<sqlite::result> containPath(const char *file);
	boost::shared_ptr<sqlite::result> containHash(const char *file);
	boost::shared_ptr<sqlite::result> containBoth(const char *file);
	
	
	int getTagsOfFid(int64 fid);
	sqlite::query *select;
	sqlite::connection &db;
	int argc;
	char  **argv;
	
	void printResult(const char *file, const boost::shared_ptr<sqlite::result> &result, const PrintType& pt);

	char  ** howCont(int &numFiles,char  **files, Contain & ct);
	char  ** howPrint(int &numFiles,char  **files, PrintType & pt);
};
#endif