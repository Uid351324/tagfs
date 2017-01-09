

#ifndef TAGFSSQL
#include <sqlite/connection.hpp>
#include <sqlite/execute.hpp>
#include <sqlite/query.hpp>
#include <sqlite3.h>
#include <stdlib.h>     /* getenv */
#include <xxhash.h>
#include <stdio.h>
#include <string>

#define TAGFSSQL value
class tagDB
{
public:
	tagDB(const std::string &filename);
	void createDB();
	int nullcallback(void *data, int argc, char **argv, char **azColName);
	void init();
	sqlite::query * setUp(const std::string &statment);

	 sqlite::query *selectTagByName;
	 sqlite::query *selectTagByNameWithParent;
	 sqlite::query *selectFileByName;
	 sqlite::query *selectFileByNameWithParent;
	 sqlite::query *selectTaggedFileByNameWithParent;
	 sqlite::query *selectTagsOfFile;
	 sqlite::query *selectTagsByParent;
	 sqlite::query *selectTagsByParentNULL;
	 sqlite::query *selectFileByTag;
	 sqlite::query *selectFileByID;
	 sqlite::query *selectFileWithTags;
	 sqlite::query *selectTags;
	 sqlite::query *insertTags;
	 sqlite::query *deleteTags;
	 sqlite::query *updateTags;
	 sqlite::query *updateTagsParent;
	 sqlite::query *insertFile;
	 sqlite::query *insertFileTaged;
	 sqlite::query *deleteFileTaged;
	 sqlite::connection db;
};
#endif