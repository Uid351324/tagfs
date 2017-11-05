#define FUSE_USE_VERSION 30

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include <sqlite3.h>
#include <stdlib.h>     /* getenv */
#include <xxhash.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sql.h"
#include "fuse.h"
#include "tool.h"
#include <iostream>
#include "common.h"
bool testcreatedir(const std::string &dir)
{
	struct stat buf;
		if(stat(dir.c_str(),&buf) == 0)
		{
			printf("dir exist\n");
		}
		else
		{
			printf("dir not exist %s\n", dir.c_str());
			if(mkdir(dir.c_str(),0755) == -1) 
			{
				int errsv = errno;
				printf("somecall() failed %d\n", errsv);
			}
			return true;
		}
		return false;
}
bool testcreatedb(const std::string &dir)
{
	struct stat buf;
		if(stat(dir.c_str(),&buf) == 0)
		{
			printf("db exist  %s\n", dir.c_str());
		}
		else
		{
			printf("db not exist %s\n", dir.c_str());
			return true;
		}
		return false;
}
int main(int argc, char  *argv[])
{
	std::string home;
	std::string file;
	std::string dbname = "/tagfs";
	bool create = true;
	bool toolMode = false;
	if(getenv("XDG_CONFIG_HOME") != NULL)
	{
		// If XDG_CONFIG_HOME is set explicitly, we'll use that instead of $HOME/.config
		home = std::string( getenv("XDG_CONFIG_HOME")) + "/tagfs";
	}
	else
	{
		home = std::string( getenv("HOME")) + "/.config/tagfs";
	}
	create = testcreatedir(home);
	int nxtArg = 1;// [0] is exe name so [1] is first actual argument
	if(strcmp(argv[nxtArg],"--")==0)
	{
		printf("use toolMode %s\n", argv[1]);
		toolMode = true;
		nxtArg++;
	}
	if(strcmp(argv[nxtArg],"--db")==0)
	{
		printf("use db: %s\n", argv[++nxtArg]);
		dbname = "/";
		dbname += argv[nxtArg];
		nxtArg++;
//		argc -=2;
	}
	char *zErrMsg = 0;
	int rc;
	file = home + dbname +".db";
	create = testcreatedb(file);
	printf("create= %d \n", create);

	fuse_operations ops;
	memset(&ops, 0, sizeof(struct fuse_operations));
	tagDB db(file);
	try{

		printf("create= %d \n", create);
		if(create)
		{

			printf("create dba\n");
			db.createDB();
			printf("create dbb\n");
		}
		db.init();
		init(&db, &ops);
	}
	catch(std::exception const & e) 
	{
		std::cerr << "An error occurred: " << e.what() << std::endl;
		return 1;
	}
	nxtArg--;//shift back to 'exec name' first argument because that what exepts fuse
	if(toolMode)
	{	
		tool(db.db, argc-nxtArg, argv+nxtArg);
		return 0;
	}
	else
		return fuse_main(argc-nxtArg, argv+nxtArg, &ops,NULL);;
}
