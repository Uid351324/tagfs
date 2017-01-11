# tagfs
==================
##about
------------------
*fuse based filesystem for taging and organizing your files.
*it does not hold any files, only symbolic links to them 
*you do not need to use another tool, only standard operation like rm, mv, ln...
*can be used with any software that browses files
------------------
##usage
------------------
mounting:
`./tagfs  mount_point [fuse_options]  [--db other_tags_database]`
to get fuse options:
`./tagfs -h`
example after mounting:
`cd $mount_point
cd tags
mkdir foo #create foo tag
ln -s some_file foo/ #tags file as foo
ls foo/ #show files with tag foo`
------------------
##building
------------------
requirement: sqlite3, fuse, xxhash, vsqlite++
`premake gmake
make`