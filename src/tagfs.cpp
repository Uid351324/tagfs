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
#include <iostream>
int main(int argc, char  *argv[])
{
	std::string home;
	std::string file;
	std::string dbdir = "";
	bool create = false;

	if(getenv("XDG_CONFIG_HOME") != NULL)
	{
		// If XDG_CONFIG_HOME is set explicitly, we'll use that instead of $HOME/.config
		home = std::string( getenv("XDG_CONFIG_HOME")) + "/tagfs";
	}
	else
	{
		home = std::string( getenv("HOME")) + "/.config/tagfs";
	}
	if(argc >=3 and strcmp(argv[argc-2],"--db")==0)
	{
		printf("use db: %s\n", argv[argc-1]);
		dbdir = "/";
		dbdir += argv[argc-1];
		struct stat buf;
		if(stat(std::string(home+dbdir).c_str(),&buf) == 0)
		{
			printf("dir exist");
		}
		else
		{
			printf("dir not exist %s", std::string(home+dbdir).c_str());
			if(mkdir(std::string(home+dbdir).c_str(),0755) == -1) 
			{
				int errsv = errno;
				printf("somecall() failed %d\n", errsv);
			}
			create = true;
		}
		argc -=2;
	}
	char *zErrMsg = 0;
	int rc;
	file = home + dbdir +"/tagfs.db";
	fuse_operations ops;
	memset(&ops, 0, sizeof(struct fuse_operations));
	try{
		tagDB db(file);

		if(create)
		{
			db.createDB();
		}
		init(&db, &ops);
	}
	catch(std::exception const & e) 
	{
		std::cerr << "An error occurred: " << e.what() << std::endl;
		return 1;
	}
	return fuse_main(argc, argv, &ops,NULL);;
}
