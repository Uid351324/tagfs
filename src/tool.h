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
class tool
{
public:
	tool(sqlite::connection &db, int argc, char  **argv);
	// ~tool();
private:
	int contain(const char *file);
	int getTagsOfFid(int64 fid);
	sqlite::query *select;
	sqlite::connection &db;
	int argc;
	char  **argv;
};
#endif