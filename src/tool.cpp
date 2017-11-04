#include "tool.h"

tool::tool(sqlite::connection &_db, int _argc, char  **_argv):db(_db),argc(_argc),argv(_argv)
{
	for(int i=0;i<argc;i++)
	{
		printf("%d %s\n",i, argv[i]);
	}	
		printf("tool-");
	if(strcmp(argv[2],"cont")==0)
	{
		printf("cont\n");
		select = new sqlite::query(db, "select fid,file,filename,fsize, fhash from files where fid = ? or file = ? or fhash = ?");
		char resolved_path[PATH_MAX]; 
		realpath(argv[3], resolved_path); 
		contain(resolved_path);

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

int tool::contain(const char *file)
{

	struct stat64 sb;
	stat64(file, &sb);
	printf("%s:\n", file);
	if(S_ISREG(sb.st_mode) or S_ISDIR(sb.st_mode))
	{

		int64 fid = XXH64(file,strlen(file),XXHASHSEED);

		int64 fhash = calcualteHash(file,sb.st_blksize);
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
		select->bind(1,fid);
		select->bind(2,file);
		select->bind(3,fhash);
		boost::shared_ptr<sqlite::result> result = select->get_result();
		int found = 0;
		while(result->next_row())
		{
			printf("%d %s %s %d %d\n",  result->get_int64( 0),
				 result->get_string( 1).c_str() ,
				 result->get_string( 2).c_str() ,
				 result->get_int64( 3),
				result->get_int64( 4) );
			getTagsOfFid(result->get_int64( 0));
			found++;
		}
		printf("%d found\n", found);
	}
	else
	{
		printf("not file\n");
	}
}