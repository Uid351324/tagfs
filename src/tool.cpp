#include "tool.h"
#include <iostream>
#include <vector>
//enum class  Contain{ Name, Hash, Both};

char  ** tool::howCont(int &numFiles,char  **files, Contain & ct)
{
//	Contain ct{Contain::Name};
	printf("how[%d] %s\n",0, files[0]);
	assert(numFiles >=1);
	if(strcmp(files[0],"--name")==0)//2:tool 3:???
	{
		ct = Contain::Path;
		numFiles--;
		return &files[1];
	}
	else if(strcmp(files[0],"--hash")==0)
	{
		ct = Contain::Hash;
		numFiles--;
		return &files[1];
	}
	else if(strcmp(files[0],"--both")==0)
	{
		ct = Contain::Both;
		numFiles--;
		return &files[1];
	}
	else if(strcmp(files[0],"--either")==0)
	{
		ct = Contain::Either;
		numFiles--;
		return &files[1];
	}
	return &files[0];
}

char  ** tool::howPrint(int &numFiles,char  **files, PrintType & pt)
{
//	Contain ct{Contain::Name};
	assert(numFiles >=1);
	if(strcmp(files[0],"--all")==0)//2:tool 3:???
	{
		pt = PrintType::All;
		numFiles--;
		return &files[1];
	}
	else if(strcmp(files[0],"--all-tag")==0)
	{
		pt = PrintType::AllTaged;
		numFiles--;
		return &files[1];
	}
	else if(strcmp(files[0],"--found")==0)
	{
		pt = PrintType::Found;
		numFiles--;
		return &files[1];
	}
	else if(strcmp(files[0],"--not-found")==0)
	{
		pt = PrintType::NotFound;
		numFiles--;
		return &files[1];
	}
	return &files[0];
}

bool tool::printResult(const char *file, const boost::shared_ptr<sqlite::result> &result,const PrintType& pt)
{
	bool nextRow = result->next_row();
	if(nextRow)
	{
		if( pt == PrintType::TagLess)
			printf("%s\n",result->get_string( 1).c_str());
//		printf("%d %s %s %d %d\n",  result->get_int64( 0),
//			 result->get_string( 1).c_str() ,
//			 result->get_string( 2).c_str() ,
//			 result->get_int64( 3),
//			result->get_int64( 4) );
		if( pt == PrintType::All or pt == PrintType::AllTaged )
			printf("found: ");
		if( pt == PrintType::All or pt == PrintType::AllTaged or pt == PrintType::Found)
			printf("%s\n", file);
		if( pt == PrintType::AllTaged)
		{
			printf("tags: ");
			getTagsOfFid(result->get_int64( 0));
			
		}
		
	}
	else
	{
		if( pt == PrintType::All or pt == PrintType::AllTaged )
			printf("not found: ");
		if( pt == PrintType::All  or pt == PrintType::AllTaged or pt == PrintType::NotFound)
			printf("%s\n", file);
	}
//			found++;
	return nextRow;
}

tool::tool(sqlite::connection &_db, int _argc, char  **_argv):db(_db),argc(_argc),argv(_argv)
{
	for(int i=0;i<argc;i++)
	{
		printf("arg[%d] %s\n",i, argv[i]);
	}	
		printf("tool-");
	if(strcmp(argv[1],"contain")==0)// 
	{
		toolContains();
	}
	if(strcmp(argv[1],"tagless")==0)// 
	{
		toolTagless();
	}
	if(strcmp(argv[1],"linkless")==0)// 
	{
		toolLinkless();
	}
	if(strcmp(argv[1],"rehash")==0)// 
	{
		toolReHash();
	}
	if(strcmp(argv[1],"relink")==0)// 
	{
		toolReLink();
	}
}


int tool::getTagsOfFid(int64 fid)
{
	sqlite::query *sel = new sqlite::query(db, "WITH RECURSIVE\
                childTags(tid) as (\
                        select tid from filetag where fid  == ?\
                        union\
                        select tags.tparent from tags, childTags where childTags.tid == tags.tid\
                )\
                select distinct tname from tags where tid in childTags;");
	sel->clear();
	sel->bind(1,fid);
	boost::shared_ptr<sqlite::result> result = sel->get_result();
	while(result->next_row())
	{
		printf("\t %s",  result->get_string(0).c_str() );
	}
	printf("\n");
	return 0;
}

boost::shared_ptr<sqlite::result> tool::containPath(const char *file)
{

	struct stat64 sb;
	if(stat64(file, &sb) == -1)
	{
		perror("stat");
		return nullptr;
	}
//	printf("%s:\n", file);
	if(S_ISREG(sb.st_mode) or S_ISDIR(sb.st_mode))
	{

//		int64 fid = XXH64(file,strlen(file),XXHASHSEED);

//		int64 fhash = calcualteHash(file,sb.st_blksize);
//		XXH64_state_t *hashstate = XXH64_createState();
//		XXH64_reset  (hashstate, XXHASHSEED);
//		 {
//			int length = sb.st_blksize;
//			char *buffer = new char [length];
//			// printf("path: %s\n", file);
//			std::ifstream is (file, std::ifstream::binary);
//			while (is)
//			{
//				is.read (buffer,length);
//				XXH64_update (hashstate, buffer, is.gcount());
//				// printf("tash %x\n", XXH64_digest (hashstate));
//			}
//			is.close();
//			delete[] buffer;
//		 }
//		fhash = (int64)XXH64_digest (hashstate);

		select->clear();
//		select->bind(1,fid);
		select->bind(1,file);
//		select->bind(3,fhash);
		boost::shared_ptr<sqlite::result> result = select->get_result();
//		int found = 0;
		return result;
//		while(result->next_row())
//		{
//			printf("%d %s %s %d %d\n",  result->get_int64( 0),
//				 result->get_string( 1).c_str() ,
//				 result->get_string( 2).c_str() ,
//				 result->get_int64( 3),
//				result->get_int64( 4) );
//			getTagsOfFid(result->get_int64( 0));
//			found++;
//		}
//		printf("%d found\n", found);
	}
	else
	{
		printf("not file\n");
		return nullptr;
	}
}

boost::shared_ptr<sqlite::result> tool::containHash(const char *file)
{

	struct stat64 sb;
	if(stat64(file, &sb) == -1)
	{
		perror("stat");
		return nullptr;
	}
	printf("%s:\n", file);
	if(S_ISREG(sb.st_mode) or S_ISDIR(sb.st_mode))
	{
		printf("%s is file\n", file);
		int64 fhash = calcualteHash(file,sb.st_blksize);
		std::cout<<fhash<<std::endl;
		select->clear();
//		select->bind(1,fid);
		select->bind(1,fhash);
//		select->bind(3,fhash);
		boost::shared_ptr<sqlite::result> result = select->get_result();
//		int found = 0;
		return result;
	}
	else
	{
		printf("not file\n");
		return nullptr;
	}
}
boost::shared_ptr<sqlite::result> tool::containBoth(const char *file)
{
	struct stat64 sb;
	if(stat64(file, &sb) == -1)
	{
		perror("stat");
		return nullptr;
	}
	if(S_ISREG(sb.st_mode) or S_ISDIR(sb.st_mode))
	{
		int64 fhash = calcualteHash(file,sb.st_blksize);
		select->clear();
//		select->bind(1,fid);
		select->bind(1,file);
		select->bind(2,fhash);
		boost::shared_ptr<sqlite::result> result = select->get_result();
//		int found = 0;
		return result;
	}
	else
	{
		printf("not file\n");
		return nullptr;
	}
}
void tool::toolContains(void)
{
		int numFiles = argc-2;
		char  **files = argv+2;
		Contain ct{Contain::Path} ;
		PrintType pt{PrintType::All};
		files = howCont(numFiles, files, ct);
		files = howPrint(numFiles, files, pt);
		printf("cont-");
		boost::shared_ptr<sqlite::result> (tool::*containFunction)(const char *) = nullptr;
		if(ct == Contain::Path)
		{
			printf("by-name\n");
			select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files where file = ?");
			containFunction=&tool::containPath;
		}
		if(ct == Contain::Hash)
		{
			printf("by-hash\n");
			select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files where fhash = ?");
			containFunction=&tool::containHash;
		}
		if(ct == Contain::Both)
		{
			printf("by-hash\n");
			select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files where file = ? and fhash = ?");
			containFunction=&tool::containBoth;
		}
		if(ct == Contain::Either)
		{
			printf("by-hash\n");
			select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files where file = ? or fhash = ?");
			containFunction=&tool::containBoth;
		}
		char resolved_path[PATH_MAX];
		for( int i = 0; i < numFiles ; ++i)
		{
			auto file = files[i];
			printf(">>%d %s\n",i, file);
			realpath(file, resolved_path); 
			printResult(file, (this->*containFunction)(resolved_path), pt);
		}
	delete select;
	select = nullptr;
}

void tool::toolTagless(void)
{
	printf("tagless\n");
	select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files where fid not in (select distinct fid from filetag)");
	select->clear();
	boost::shared_ptr<sqlite::result> result = select->get_result();
	while(printResult(nullptr, result, PrintType::TagLess));
	delete select;
	select = nullptr;
}

void tool::toolLinkless(void)
{
	printf("linkless\n");
	select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files");
	select->clear();
	boost::shared_ptr<sqlite::result> result = select->get_result();
	while(result->next_row())
	{
		const std::string &file = result->get_string( 1);
//		std::cout << file << ":"<<std::endl;
		struct stat64 sb;
		if ( stat64(file.c_str(), &sb) == -1) 
			printf("%s\n", file.c_str());

	}
	delete select;
	select = nullptr;
}

void tool::toolReHash(void)
{
	int numFiles = argc-2;
	char  **files = argv+2;
	char resolved_path[PATH_MAX];
	select = new sqlite::query(db, "update files set fhash = ?, fsize = ? where fid == ?");
//	select->clear();
	for( int i = 0; i < numFiles ; ++i)
	{
		auto file = files[i];
		printf(">>%d %s\n",i, file);
		struct stat64 sb;
		if(stat64(file, &sb) == -1)
		{
			perror("stat");
			continue;
		}
		int64 fhash = calcualteHash(file,sb.st_blksize);
		int64 fid = XXH64(file,strlen(file),XXHASHSEED);
		select->clear();
		select->bind(1,fhash);
		select->bind(2,sb.st_size);
		select->bind(3,fid);
		select->emit();
		
	}
	delete select;
	select = nullptr;
}

void tool::toolReLink(void)
{
	int numFiles = argc-2;
	char  **files = argv+2;
	char resolved_path[PATH_MAX];
	select = new sqlite::query(db, "update files set fid = ?, file = ?, filename = ? where fhash == ?");
//	select->clear();
	for( int i = 0; i < numFiles ; ++i)
	{
		auto file = files[i];
		printf(">>%d %s\n",i, file);
		std::vector<std::string> fileParts = split(file, '/');
//		uint linknamesize = linknames.size();
		struct stat64 sb;
		if(stat64(file, &sb) == -1)
		{
			perror("stat");
			continue;
		}
		int64 fhash = calcualteHash(file,sb.st_blksize);
		int64 fid = XXH64(file,strlen(file),XXHASHSEED);
		select->clear();
		select->bind(1,fid);
		select->bind(2,file);
		select->bind(3,fileParts.back());
		select->bind(4,fhash);
		
		select->emit();
		
	}
	delete select;
	select = nullptr;
}
