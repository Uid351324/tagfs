#include "fuse.h"
#include <sys/types.h>
#include <sys/stat.h>

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
		if(item != "")
			elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
void init(tagDB *dbp,struct fuse_operations *tagoper)
{
	tagoper->getattr   = taggetattr;
	tagoper->readdir = tagreaddir;
	tagoper->open = tagopen;
	tagoper->opendir = tagopendir;
	tagoper->read = tagread;
	tagoper->mkdir = tagmkdir;
	tagoper->rmdir = tagrmdir;
	tagoper->rename = tagrename;
	tagoper->symlink = tagsymlink;
	tagoper->readlink = tagreadlink;
	tagoper->unlink = tagunlink;
	db = dbp; 
}
Ftype* createQuery(std::vector<std::string> &paths, bool noparents)
{
	std::cout << "createQuery" << std::endl;
	uint pathsize = paths.size();
	std::string &name = paths[pathsize-1]; 
	std::ostringstream query;
	int params = 0;
	std::string qnot = "";
	if( paths[1].at (0) == '-' )
		qnot = "not";
	if( paths[1].at (0) == '-' or paths[1].at (0) == '+')
		paths[1] = paths[1].substr (1);

	query << "WITH RECURSIVE\
	                childTags(tid) as (\
	                        VALUES(?)\
	                        union\
	                        select tags.tid from tags, childTags where tparent == childTags.tid\
	                )\
	                select distinct fid,file,filename,fsize, fhash from filetag natural join files where tid " << qnot <<" in childTags and `filename` == ? \
					";
	params +=2;
	
	for(uint i = 2; i < pathsize -1; i++)
	{
		std::string qjoint = "";
		if( paths[i].at (0) == '-' )
		{
			qjoint = "EXCEPT";
			paths[i] = paths[i].substr (1);
		}
		else if( paths[i].at (0) == '+' )
		{
			qjoint = "UNION";
			paths[i] = paths[i].substr (1);
		}
		else
		{
			qjoint = "INTERSECT";
		}
		query << qjoint << " \
		select fid,file,filename,fsize, fhash from (\
				WITH RECURSIVE\
		                childTags(tid) as (\
		                        VALUES(?)\
		                        union\
		                        select tags.tid from tags, childTags where tparent == childTags.tid\
		                )\
		                select distinct fid,file,filename,fsize, fhash from filetag natural join files where tid in childTags and `filename` == ?\
			)";
		params +=2;
	}
	boost::shared_ptr<sqlite::query> queryStatment ( db->setUp(query.str()));
	for(uint i = 1, j = 1 ; i < pathsize -1;  i++)
	{
		int64 htag = XXH32(paths[i].c_str(),paths[i].size(),XXHASHSEED);
		std::cout << i <<" " <<paths[i] <<" "<<htag << std::endl;
		queryStatment->bind(j++,htag);
		queryStatment->bind(j++,name);
	}
	std::cout << query.str() << std::endl;
	boost::shared_ptr<sqlite::result> result = queryStatment->get_result();
	if(result->next_row())
	{
		Ffile* rf = new Ffile();
		rf->isfile = true;
		rf->id = result->get_int64( 0);
		rf->file.assign( result->get_string( 1) );
		rf->filename.assign( result->get_string( 2) );
		rf->fsize =  result->get_int64( 3);
		rf->fhash = result->get_int64( 4) ;
		return rf;
	}
	else
		return NULL;
}

Ftype* fileOrTagByName(std::vector<std::string> &paths, bool noparents)
{
	uint pathsize = paths.size();
	std::string name ( paths[pathsize-1] ); 
	if( paths[0].compare ("query") == 0 )
	{
		if( name.at (0) == '-' or name.at (0) == '+')
			name = name.substr (1);
	}
	//bool found = false;
	//std::istringstream query;
	//std::string query;
	sqlite::query *query;
	if( !noparents and !(( paths[pathsize-2].compare ("tagged") == 0 ) or (paths[pathsize-2].compare ("tags") == 0) or (paths[pathsize-2].compare ("_all") == 0) ))
	{
		query = db->selectTagByNameWithParent;
		query->clear();
		query->bind(1,name);
		int64 hparent = XXH32(paths[pathsize-2].c_str(),paths[pathsize-2].size(),XXHASHSEED);
		query->bind(2,hparent);
	}
	else
	{
		std::cout << "noParents "<< name << std::endl;
		query = db->selectTagByName;
		query->clear();
		query->bind(1,name);
	}
	
	boost::shared_ptr<sqlite::result> result = query->get_result();///FIXME boost::shared_ptr<sqlite::result>
	if(result->next_row())
    {
		Ftag* rf = new Ftag();
		rf->isfile = false;
		rf->id = result->get_int64(0);
		rf->tname.assign( result->get_string(1) );
		rf->tparent = result->get_int64(2);
		
		return rf;
	}
	
	if(pathsize < 2)
		return NULL;
	if(paths[0].compare("query") == 0)
	{
		return createQuery(paths, noparents);
	}
	else
	{
		int64 hparent = XXH32(paths[pathsize-2].c_str(),paths[pathsize-2].size(),XXHASHSEED);
		if(paths[0].compare("tagged") == 0)
		{
			query = db->selectTaggedFileByNameWithParent;
			query->clear();
			query->bind(2,name);
			query->bind(1,hparent);
		}
		else if( noparents)
		{
			query = db->selectFileByName;
			query->clear();
			query->bind(1,name);
		}
		else
		{
			query = db->selectFileByNameWithParent;
			query->clear();
			query->bind(1,name);
			query->bind(2,hparent);
		}
		
		boost::shared_ptr<sqlite::result> result = query->get_result();///FIXME boost::shared_ptr<sqlite::result>
		if(result->next_row())
		{
			Ffile* rf = new Ffile();
			rf->isfile = true;
			rf->id = result->get_int64(0);
			rf->file.assign( result->get_string(1) );
			rf->filename.assign( result->get_string(2) );
			rf->fsize = result->get_int64(3);
			rf->fhash = result->get_int64(4);
			
			return rf;
		}
	}
	return NULL;
}

int taggetattr(const char *path, struct stat *stbuf)
{
	std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();

	memset(stbuf, 0, sizeof(struct stat));
	if ( strcmp(path, "/") == 0 ) //root
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	} 
	bool tagsFile = false;
	if ( paths[pathsize-1].compare ( 0, 6, ".tags." ) == 0)//tekst file containing all tags for symlink eg: from movie.mkv a file .tags.movie.mkv
	{
		tagsFile = true;
		paths[pathsize-1] = paths[pathsize-1].substr(6);//cut the '.tags.' mark to finde if have symlink in fs
	}

	if ( ( paths[0].compare("tagged") == 0 ) or (paths[0].compare("tags") == 0) )
	{
		if ( ( pathsize == 2 and paths[1].compare("_all") == 0 ) or pathsize == 1 ) //top dir of tagged, tags, tags/_all
		{
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			return 0;
		}
		else
		{
			Ftype *ft = fileOrTagByName(paths);
			if ( ft != NULL )
			{
				stbuf->st_ino = ft->id;
				if ( ! ft->isfile ) // tag = dir
				{
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 2;
				}
				else if ( ft->isfile and !tagsFile ) // file = symlink
				{
					stbuf->st_mode = S_IFLNK | 0755;
					stbuf->st_nlink = 2;
					stbuf->st_size = ( static_cast<Ffile*>( ft ) )->file.size();
				}
				else // text file 
				{
					stbuf->st_mode = S_IFREG | 0444;
					stbuf->st_nlink = 2;
					stbuf->st_size = 1024;
				}
				return 0;
			}
			else
			{
				return -ENOENT;
			}
		}
	}
	else if (  paths[0].compare("query") == 0 )
	{
		if ( pathsize == 1 )
		{
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			return 0;
		}
		else
		{
			Ftype *ft = fileOrTagByName(paths, true);
			if ( ft != NULL )
			{
				stbuf->st_ino = ft->id;
				if ( ! ft->isfile ) // tag = dir
				{
					stbuf->st_mode = S_IFDIR | 0755;
					stbuf->st_nlink = 2;
				}
				else if ( ft->isfile and !tagsFile ) // file = symlink
				{
					stbuf->st_mode = S_IFLNK | 0755;
					stbuf->st_nlink = 2;
					stbuf->st_size = ( static_cast<Ffile*>( ft ) )->file.size();
				}
				else // text file 
				{
					stbuf->st_mode = S_IFREG | 0444;
					stbuf->st_nlink = 2;
					stbuf->st_size = 1024;
				}
				return 0;
			}
			else
			{
				return -ENOENT;
			}
		}
	}
	/*if (strcmp(path, hello_path) == 0) 
	{
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
		return res;
	}*/

	return -ENOENT;
}

void tagsOfFile(int64 fid, std::string &output)
{
	sqlite::query *query;
	query = db->selectTagsOfFile;
	query->clear();
	query->bind(1,fid);

	boost::shared_ptr<sqlite::result> result = query->get_result();///FIXME boost::shared_ptr<sqlite::result>

	while(result->next_row())
    {
    	output.append("\n");
    	output.append(result->get_string(0));
    }
    output.append("\n");
}
int filldir(void *buf, fuse_fill_dir_t filler, bool withParent, int64 parentHash)
{
	sqlite::query *query;
	if(withParent == false)
	{
		query = db->selectTagsByParentNULL;
		query->clear();
	}
	else if(withParent and parentHash == 0)
	{
		query = db->selectTags;
		query->clear();
	}
	else
	{
		query = db->selectTagsByParent;
		query->clear();
		query->bind(1,parentHash);
	}
	
	boost::shared_ptr<sqlite::result> result = query->get_result();
	
	try{
		while(result->next_row())
		{
			filler(buf, result->get_string(0).c_str(), NULL, 0);
		}
	
	}
	catch(std::exception const & e) 
	{
		std::cerr << "An error occurred: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int fillinks(void *buf, fuse_fill_dir_t filler,  int64 parentHash)
{
	
	sqlite::query *query;
	query = db->selectFileByTag;
	query->clear();
	query->bind(1,parentHash);
	
	boost::shared_ptr<sqlite::result> result = query->get_result();
	
	while(result->next_row())
    {
		filler(buf, result->get_string(0).c_str(), NULL, 0);
    }
	return 0;
}

int fillinksplus(void *buf, fuse_fill_dir_t filler,  int64 parentHash)
{
	
	sqlite::query *query;
	query = db->selectFileWithTags;
	query->clear();
	query->bind(1,parentHash);
	
	boost::shared_ptr<sqlite::result> result = query->get_result();
	
	while(result->next_row())
    {
		filler(buf, result->get_string(2).c_str(), NULL, 0);
    }
	return 0;
}

int fillinksQuery(std::vector<std::string> &paths,void *buf, fuse_fill_dir_t filler)
{
	uint pathsize = paths.size();
	//std::string &name = paths[pathsize-1]; 
	std::ostringstream query;
	int params = 0;
	std::string qnot = "";
	if( paths[1].at (0) == '-' )
	{
		qnot = "not";
		query << "select fid,file,filename,fsize, fhash from files\
		EXCEPT\
		";
	}
	if( paths[1].at (0) == '-' or paths[1].at (0) == '+')
		paths[1] = paths[1].substr (1);

	query << "\
		select fid,file,filename,fsize, fhash from (\
		WITH RECURSIVE\
                childTags(tid) as (\
                        VALUES(?)\
                        union\
                        select tags.tid from tags, childTags where tparent == childTags.tid\
                )\
                select distinct fid,file,filename,fsize, fhash from filetag natural join files where tid  in childTags\
		) \
					";
	params +=2;
	
	for(uint i = 2; i < pathsize; i++)
	{
		std::string qjoint = "";
		if( paths[i].at (0) == '-' )
		{
			qjoint = "EXCEPT";
			paths[i] = paths[i].substr (1);
		}
		else if( paths[i].at (0) == '+' )
		{
			qjoint = "UNION";
			paths[i] = paths[i].substr (1);
		}
		else
		{
			qjoint = "INTERSECT";
		}
		query << qjoint << " \
		select fid,file,filename,fsize, fhash from (\
				WITH RECURSIVE\
		                childTags(tid) as (\
		                        VALUES(?)\
		                        union\
		                        select tags.tid from tags, childTags where tparent == childTags.tid\
		                )\
		                select distinct fid,file,filename,fsize, fhash from filetag natural join files where tid in childTags\
			)";
		params +=2;
	}
	boost::shared_ptr<sqlite::query> queryStatment ( db->setUp(query.str()));
	for(uint i = 1; i < pathsize; )
	{
		int64 htag = XXH32(paths[i].c_str(),paths[i].size(),XXHASHSEED);
		queryStatment->bind(i,htag);
		i++;
	}
	boost::shared_ptr<sqlite::result> result = queryStatment->get_result();
	while(result->next_row())
	{
		filler(buf, result->get_string(2).c_str(), NULL, 0);
	}
	return 0;
}
 int tagreaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
 {
	std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size() ;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	if ( strcmp(path, "/") == 0 )
	{
		filler(buf, "tags", NULL, 0);
		filler(buf, "tagged", NULL, 0);
		filler(buf, "query", NULL, 0);
	}
	else if ( paths[0].compare("tags") == 0 )
	{
		if ( pathsize ==  1 )// /tags/
		{
			filler(buf, "_all", NULL, 0);
			filldir(buf,filler);
		}
		else if( pathsize ==  2 and paths[1].compare("_all") == 0 ) // /tags/_all
		{
			filldir(buf,filler,true,0);
		}
		else// /tags/*/*
		{
			int64 htag = XXH32(paths[pathsize-1].c_str(),paths[pathsize-1].size(),XXHASHSEED);
			std::cout<< paths[pathsize-1]<<" "<< htag<<std::endl;
			filldir(buf,filler,true,htag);
			fillinks(buf,filler,htag);
		}
	}
	else if( paths[0].compare("tagged") == 0)
	{
		if ( pathsize ==  1 )
		{
			filldir(buf,filler);
		}
		else
		{
			int64 htag = XXH32(paths[pathsize-1].c_str(),paths[pathsize-1].size(),XXHASHSEED);
			filldir(buf,filler,true,htag);
			fillinksplus(buf,filler,htag);
		}
	}
	else if ( paths[0].compare("query") == 0)
	{
		if ( pathsize !=  1 )
		{
			fillinksQuery(paths,buf, filler);
		}
	}
	else
	{
		return -ENOENT;
	}
	return 0;
 }

 int tagopen(const char *path, struct fuse_file_info *fi)
 {
	std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	if ( fi->flags & O_WRONLY or fi->flags & O_RDWR)
		return -EACCES;
	else
	{
		bool tagsFile = false;
		if ( paths[pathsize-1].compare ( 0, 6, ".tags." ) == 0)//tekst file containing all tags for symlink eg: from movie.mkv a file .tags.movie.mkv
		{
			tagsFile = true;
			paths[pathsize-1] = paths[pathsize-1].substr(6);//cut the '.tags.' mark to finde if have symlink in fs
		}
		else
			return -ENOENT;

		if ( ( paths[0].compare("tagged") == 0 ) or (paths[0].compare("tags") == 0) )
		{
			uint next = 1;
			if ( paths[next].compare("_all") == 0 )
				next = 2;
			if ( pathsize == next ) //top dir of tagged, tags, tags/_all
			{
				return 0;
			}
			else
			{
				Ftype *ft = fileOrTagByName(paths);
				if ( ft != NULL )
				{
					if ( ft->isfile and tagsFile ) // file = symlink
					{
						fi->fh = ft->id;
						return 0;
					}
					return -ENOENT;
				}
				else
				{
					return -ENOENT;
				}
			}
		}
		else if (  paths[0].compare("query") == 0 )
		{
			if ( pathsize == 1 )
			{
				return 0;
			}
			else
			{
				Ftype *ft = fileOrTagByName(paths, true);
				if ( ft != NULL )
				{
					if ( ft->isfile and tagsFile ) // file = symlink
					{
						fi->fh = ft->id;
						return 0;
					}
					return -ENOENT;
				}
				else
				{
					return -ENOENT;
				}
			}
		}
		else
			return -ENOENT;
	}
 }
 
 int tagopendir(const char *path, struct fuse_file_info *fi)
 {
	 std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	if ( fi->flags & O_WRONLY or fi->flags & O_RDWR)
		return -EACCES;
	else
	{
		if(strcmp(path, "/") == 0)
		{
			return 0;
		}
		else if ( ( paths[0].compare("tagged") == 0 ) or (paths[0].compare("tags") == 0) )
		{
			uint next = 1;
			if( ( pathsize == 2 and paths[1].compare("_all") == 0 ) or pathsize == 1 ) //top dir of tagged, tags, tags/_all
			{
				return 0;
			}
			else
			{
				Ftype *ft = fileOrTagByName(paths);
				if ( ft != NULL )
				{
					if ( !ft->isfile ) // 
					{
						return 0;
					}
					return -ENOENT;
				}
				else
				{
					return -ENOENT;
				}
			}
		}
		else if (  paths[0].compare("query") == 0 )
		{
			if ( pathsize == 1 )
			{
				return 0;
			}
			else
			{
				Ftype *ft = fileOrTagByName(paths, true);
				if ( ft != NULL )
				{
					if ( !ft->isfile  ) // file = symlink
					{
						return 0;
					}
					return -ENOENT;
				}
				else
				{
					return -ENOENT;
				}
			}
		}
		else
		{
			return -ENOENT;
		}
	}
 }
 
 int tagread(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
 {
	 if (fi->fh == 0)
		 return -EINVAL;
	std::string text;
	tagsOfFile(fi->fh, text);
	text.resize(1024,'#');
	if(offset > text.size())
		return 0;
	
    if (offset + size > text.size()) {
      memcpy(buf, text.c_str() + offset, text.size() - offset);
      return text.size() - offset;
    }

    memcpy(buf, text.c_str() + offset, size);
    return size;
 }
 
 int tagmkdir(const char *path, mode_t mode)
 {
	 std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	if ( (( paths[0].compare("tagged") == 0 ) or (paths[0].compare("tags") == 0))  and ( (pathsize != 1 ) or ( paths[1].compare("_all") == 0) ) )
	{
		int64 hash = XXH32(paths[pathsize-1].c_str(),paths[pathsize-1].size(),XXHASHSEED);
		
					sqlite::query *query;
		query = db->insertTags;
		query->clear();
		query->bind(1,hash);
		query->bind(2,paths[pathsize-1]);
		if ( ( paths[pathsize-2].compare("tagged") == 0 ) or (paths[pathsize-2].compare("tags") == 0) or ( paths[pathsize-2].compare("_all") == 0) )
		{
			query->bind(3);//NULL
		}
		else
		{
			int64 hashPar = XXH32(paths[pathsize-2].c_str(),paths[pathsize-2].size(),XXHASHSEED);
			query->bind(3,hashPar);
			
		}
		try{
		query->emit();
		}
	
	catch(std::exception const & e) 
	{
		std::cerr << "An error occurred: " << e.what() << std::endl;
		return 1;
	}
		return 0;
	}
	return -EACCES;
	
 }
 
int tagrmdir (const char *path)
{
	
	 std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	if ( ( paths[pathsize-1].compare("tagged") == 0 ) or (paths[pathsize-1].compare("tags") == 0) or ( paths[pathsize-1].compare("_all") == 0)  or (paths[pathsize-1].compare("query") == 0))
	{
		return -EACCES;
	}
	else
	{
					sqlite::query *query;
		query = db->selectTagByName;
		query->clear();
		query->bind(1,paths[pathsize-1]);		
		boost::shared_ptr<sqlite::result> result = query->get_result();///FIXME boost::shared_ptr<sqlite::result>
		if(result->next_row())
		{
			int64 hash = XXH32(paths[pathsize-1].c_str(),paths[pathsize-1].size(),XXHASHSEED);
			
			query = db->deleteTags;
			query->clear();
			query->bind(1,hash);
			query->emit();
			return 0;
		}
		else
		{
			return -ENOENT;
		}
	}
}

int tagrename (const char *oldpath, const char *newpath)
{
	std::vector<std::string> oldpaths = split(oldpath, '/');
	uint oldpathsize = oldpaths.size();
	std::vector<std::string> newpaths = split(newpath, '/');
	uint newpathsize = newpaths.size();
	if ( ( oldpaths[oldpathsize-1].compare("tagged") == 0 ) or (oldpaths[oldpathsize-1].compare("tags") == 0) or ( oldpaths[oldpathsize-1].compare("_all") == 0)  or (oldpaths[oldpathsize-1].compare("query") == 0))
	{
		return -EACCES;
	}
	else
	{
		Ftype *ft = fileOrTagByName(oldpaths);
		if ( ft != NULL )
		{
			if ( !ft->isfile ) // 
			{
				if ( ( ( ( oldpaths[0].compare("tagged") != 0 ) and (oldpaths[0].compare("tags") != 0 ) ) or ( oldpaths[1].compare("_all") == 0 ) ) or ( ( ( newpaths[0].compare("tagged") != 0 ) and (newpaths[0].compare("tags") != 0 ) ) or ( newpaths[1].compare("_all") == 0 ) ) )
					return -EACCES;
				else
				{
					int64 oldhash = ft->id;//XXH32(oldpaths[oldpathsize-1].c_str(),oldpaths[oldpathsize-1].size(),XXHASHSEED);
					int64 newhash = XXH32(newpaths[newpathsize-1].c_str(),newpaths[newpathsize-1].size(),XXHASHSEED);

					sqlite::query *query;
					query = db->updateTags;
					query->clear();
					query->bind(1,newhash);
					query->bind(2,newpaths[newpathsize-1]);
					query->bind(3,oldhash);
					query->emit();
					if ( oldpaths[oldpathsize-2].compare(newpaths[newpathsize-2]) != 0 )
					{

						query = db->updateTagsParent;
						query->clear();
						int64 newparent = XXH32(newpaths[newpathsize-2].c_str(),newpaths[newpathsize-2].size(),XXHASHSEED);
						query->bind(1,newparent);
						query->bind(2,newhash);
						query->emit();

					}
					return 0;

				}
			}
			return -EACCES;
		}
		return -ENOENT;
	}
}


int tagsymlink (const char *linkname, const char *path)
{
	
	 std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	 std::vector<std::string> linknames = split(linkname, '/');
	uint linknamesize = linknames.size();
	
	struct stat64 sb;
	stat64(linkname, &sb);
	sqlite::query *query;
	if(S_ISREG(sb.st_mode) or S_ISDIR(sb.st_mode))
	{
		query = db->selectTagByName;
		query->clear();
		query->bind(1,paths[pathsize-2]);		
		boost::shared_ptr<sqlite::result> result = query->get_result();///FIXME boost::shared_ptr<sqlite::result>
		//TODO now in query links to last tag : expand all tags or forbiden to link in query
		//ex when in query/foo/bar ln -s will create tag for bar only but query for ( foo and bar ) does not return newly link so ln return error
		if( ! result->next_row())
		{
			return -EACCES;
		}
		int64 tid = XXH32(paths[pathsize-2].c_str(),paths[pathsize-2].size(),XXHASHSEED);
		int64 fid = XXH32(linkname,strlen(linkname),XXHASHSEED);
		{
			query = db->selectFileByID;
			query->clear();
			query->bind(1,fid);		
			boost::shared_ptr<sqlite::result> result = query->get_result();///FIXME boost::shared_ptr<sqlite::result>
			if(!  result->next_row())
			{
				int64 fhash = 0;
				XXH32_state_t *hashstate = XXH32_createState();
				XXH32_reset  (hashstate, XXHASHSEED);
				{
					int length = sb.st_blksize;
					char *buffer = new char [length];
					std::ifstream is (path, std::ifstream::binary);
					while (is)
					{
						is.read (buffer,length);
						XXH32_update (hashstate, buffer, is.gcount());
					}
					is.close();
					delete[] buffer;
				}
				fhash = XXH32_digest (hashstate);
				XXH32_freeState(hashstate);
				
				query = db->insertFile;
				query->clear();
				query->bind(1,fid);
				query->bind(2,linkname);
				query->bind(3,linknames[linknamesize-1]);
				query->bind(4,sb.st_size);
				query->bind(5,fhash);
				query->bind(6, static_cast<int64> (sb.st_ino));
				query->emit();
			}
		}
		
		
		query = db->insertFileTaged;
		query->clear();
		query->bind(1,fid);
		query->bind(2,tid);
		query->emit();
		return 0;
	}
	return -EINVAL;
}

int tagreadlink (const char * path, char *buffer, size_t size)
{
	 std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	Ftype* ft = fileOrTagByName(paths);
	if (ft and ft->isfile )
	{
		strncpy(buffer, ( static_cast<Ffile*>( ft ) )->file.c_str(), size);
		return 0;
	}
	else
	{
		return -ENOENT;
	}
}

int tagunlink (const char *path)
{
	
	 std::vector<std::string> paths = split(path, '/');
	uint pathsize = paths.size();
	Ftype* ft = fileOrTagByName(paths);
	if(( paths[pathsize-1].compare("tagged") != 0 ) or (paths[pathsize-1].compare("tags") != 0))
	{
		return -EACCES;
	}
	if (ft == NULL)
	{
		return -ENOENT;
	}
	int64 tid = XXH32(paths[pathsize-2].c_str(),paths[pathsize-2].size(),XXHASHSEED);
	
	sqlite::query *query = db->insertFileTaged;
	query->clear();
	query->bind(1,ft->id);
	query->bind(2,tid);
	query->emit();
	return 0;
}