#include "sql.h"

void tagDB::createDB()
{
	sqlite::execute(db,  "\
		CREATE TABLE `files` (\
		`fid`	INTEGER,\
		`fhash`	INTEGER,\
		`finode`	INTEGER,\
		`file`	TEXT NOT NULL,\
		`filename`	TEXT NOT NULL,\
		`fsize`	INTEGER NOT NULL,\
		PRIMARY KEY(fid)\
		);\
	", true);
	sqlite::execute(db, "\
		CREATE TABLE `tags` (\
		`tid`	INTEGER,\
		`tname`	TEXT UNIQUE,\
		`tparent`	INTEGER,\
		PRIMARY KEY(tid)\
		);\
	", true);
	sqlite::execute(db, "\
				CREATE TABLE `filetag` (\
		`tid`	INTEGER,\
		`fid`	INTEGER,\
		PRIMARY KEY(tid,fid)\
		FOREIGN KEY(tid) REFERENCES tags(tid) ON UPDATE CASCADE ON DELETE CASCADE,\
		FOREIGN KEY(fid) REFERENCES files(fid) ON UPDATE CASCADE ON DELETE CASCADE\
		);\
	", true);
	sqlite::execute(db, "\
		CREATE TRIGGER delete_tag BEFORE DELETE ON tags \
		  BEGIN\
		    UPDATE `tags` SET `tparent`=OLD.tparent  Where `tparent` == OLD.tid ;\
		    UPDATE `filetag` SET `tid`=OLD.tparent  Where `tid` == OLD.tid ;\
		    DELETE FROM `filetag`   Where `tid` is NULL ;\
		  END;\
		  CREATE TRIGGER rename_tag BEFORE UPDATE OF tid ON tags \
		  BEGIN\
		    UPDATE `tags` SET `tparent`=NEW.tid  Where `tparent` == OLD.tid ;\
		    UPDATE `filetag` SET `tid`=NEW.tid  Where `tid` == OLD.tid ;\
		  END;\
	", true);
	// sqlite::execute(db, sql", true);
}


sqlite::query *tagDB::setUp(const std::string &statment)
{
	return new sqlite::query(db, statment);
}

void tagDB::init()
{
	
	
 	{
 		std::string statment ("SELECT `tid`,`tname`,`tparent` FROM `tags`  Where `tname` == ? ORDER BY `tname` ASC ;");
 		selectTagByName = setUp(statment); 		
 	}
 	{
 		std::string statment  ("SELECT `tid`,`tname`,`tparent` FROM `tags`  Where `tname` == ? and tparent == ? ORDER BY `tname` ASC ;");
 		selectTagByNameWithParent = setUp(statment); 		
 	}
 	{
 		std::string statment  ("SELECT `fid`,`file`,`filename`,`fsize`, `fhash` FROM  `files`  Where `filename` == ? ORDER BY `filename` ASC ;");
 		selectFileByName = setUp(statment); 		
 	}
 	{
 		std::string statment  ("SELECT `fid`,`file`,`filename`,`fsize`, `fhash` FROM  `files`  Where `fid` == ? ORDER BY `filename` ASC ;");
 		selectFileByID = setUp(statment); 		
 	}
 	{
 		std::string statment  ("SELECT `fid`,`file`,`filename`,`fsize`, `fhash` FROM `filetag` natural join `files`  Where `filename` == ? and tid == ? ORDER BY `filename` ASC ;");
 		selectFileByNameWithParent = setUp(statment); 		
 	}
 	{
 		std::string statment  ("WITH RECURSIVE\
                childTags(tid) as (\
                        VALUES(?)\
                        union\
                        select tags.tid from tags, childTags where tparent == childTags.tid\
                )\
                select distinct `fid`,`file`,`filename`,`fsize`, `fhash` from filetag natural join files where tid in childTags and `filename` == ?;");
 		selectTaggedFileByNameWithParent = setUp(statment); 		
 	}
 	{
 		std::string statment  ("WITH RECURSIVE\
                childTags(tid) as (\
                        select tid from filetag where fid  == ?\
                        union\
                        select tags.tparent from tags, childTags where childTags.tid == tags.tid\
                )\
                select distinct tname from tags where tid in childTags;");
 		selectTagsOfFile = setUp(statment); 		
 	}
	
 	{
 		std::string statment ("SELECT `tname` FROM `tags`  WHERE `tparent` == ?  ORDER BY `tname` ASC;");
 		selectTagsByParent = setUp(statment); 		
 	}
 	{
 		std::string statment ("SELECT `tname` FROM `tags`  WHERE `tparent` is NULL  ORDER BY `tname` ASC;");
 		selectTagsByParentNULL = setUp(statment); 		
 	}
 	{
 		std::string statment ("SELECT `tname` FROM `tags`    ORDER BY `tname` ASC;");
 		selectTags = setUp(statment); 		
 	}
 	{
 		std::string statment ("select `filename` from filetag natural join files where tid == ?  ORDER BY `filename` ASC;");
 		selectFileByTag = setUp(statment); 		
 	}
 	{
 		std::string statment ("WITH RECURSIVE\
                childTags(tid) as (\
                        VALUES(?)\
                        union\
                        select tags.tid from tags, childTags where tparent == childTags.tid\
                )\
                select distinct `fid`,`file`,`filename`,`fsize`, `fhash` from filetag natural join files where tid in childTags");
 		selectFileWithTags = setUp(statment); 		
 	}
 	{
 		std::string statment ("INSERT INTO `tags`(`tid`,`tname`,`tparent`) VALUES (?,?,?);");
 		insertTags = setUp(statment); 		
 	}
 	{
 		std::string statment ("DELETE FROM `tags`   Where `tid` == ? ;");
 		deleteTags = setUp(statment); 		
 	}
 	{
 		std::string statment ("UPDATE `tags` SET  `tid` = ?, `tname` = ? Where `tid` == ? ;");
 		updateTags = setUp(statment); 		
 	}
 	{
 		std::string statment ("UPDATE `tags` SET  `tparent` = ? Where `tid` == ? ;");
 		updateTagsParent = setUp(statment); 		
 	}
 	{
 		std::string statment ("INSERT INTO `files`(`fid`,`file`,`filename`,`fsize`, `fhash`, `finode`) VALUES (?,?,?, ?, ?, ?);");
 		insertFile = setUp(statment); 		
 	}
 	{
 		std::string statment ("INSERT INTO `filetag`(`fid`,`tid`) VALUES (?, ?);");
 		insertFileTaged = setUp(statment); 		
 	}
 	{
 		std::string statment ("DELETE FROM `filetag` WHERE `fid` == ? and `tid` == ?;");
 		deleteFileTaged = setUp(statment); 		
 	}
	
 }

tagDB::tagDB(const std::string& filename): db(filename)
{
	sqlite::execute(db, "PRAGMA foreign_keys = 1;", true);
	// init();
}
