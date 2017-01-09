solution "TagFS"
   configurations { "debug", "release" }

project "tagfs"
   kind "ConsoleApp"
   language "C++"
	location "bin"
   -- includedirs { "include" }
   files { "src/**.h", "src/**.cpp" }
   links { "sqlite3", "sqlite", "vsqlitepp", "xxhash", "fuse"}
   -- linkoptions { "`pkg-config --libs sqlite3 sqlite `" }
   -- libdirs { os.findlib("sqlite3") }
   defines { "_FILE_OFFSET_BITS=64" }

   configuration "debug"
      flags { "Symbols" }
      defines { "_DEBUG" }

   configuration "release"
      flags { "Optimize" }
      defines { "NDEBUG" }