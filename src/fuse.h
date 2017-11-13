#ifndef TAGFSFUSE
#define FUSE_USE_VERSION 30
#define TAGFSFUSE value

#include <sys/types.h>
#include <sys/stat.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <string>
#include <sstream>
#include <xxhash.h>
#include "sql.h"
#include <stdio.h>

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include "common.h"
typedef  boost::int64_t int64;
class Ftype
{
public:
	bool isfile;
	int64 id;
};
class Ffile : public Ftype
{
public:
	std::string file;
	std::string filename;
	int64 fsize;
	int64 fhash;
};

class Ftag : public Ftype
{
public:
	std::string tname;
	int64 tparent;
};

Ftype* createQuery(std::vector<std::string> &paths, bool noparents = false );
Ftype* fileOrTagByName(std::vector<std::string> &paths, bool noparents = false);

int filldir(void *buf, fuse_fill_dir_t filler, bool withParent = false, int64 parentHash = 0);
int fillinks(void *buf, fuse_fill_dir_t filler,  int64 parentHash);
int fillinksplus(void *buf, fuse_fill_dir_t filler,  int64 parentHash);
int fillinksQuery(std::vector<std::string> &paths,void *buf, fuse_fill_dir_t filler);

 int taggetattr(const char *path, struct stat *stbuf);
 int tagreaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
 int tagopen(const char *path, struct fuse_file_info *fi);
 int tagopendir(const char *path, struct fuse_file_info *fi);
 int tagread(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
 int tagmkdir(const char *path, mode_t mode);
int tagrmdir (const char *path);
int tagrename (const char *oldpath, const char *newpath);
	int tagsymlink (const char *linkname, const char *path);
	int tagreadlink (const char * path, char *buffer, size_t size);
	int tagunlink (const char *path);


void init(tagDB *dbp, struct fuse_operations *tagoper);
static struct fuse_operations tagoper ;
static tagDB *db;
/*= {
   .getattr   = taggetattr,
   .readdir   = tagreaddir,
   .open     = tagopen,
   .read     = tagread,
};*/

#endif