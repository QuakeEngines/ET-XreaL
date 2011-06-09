
project "ETXMap"
	targetname  "ETXMap"
	language    "C++"
	kind        "ConsoleApp"
	targetdir 	"../../../etxradiant"
	flags       { "ExtraWarnings" }
	files
	{
		"*.txt", 
		
		"../common/cmdlib.c", "../common/cmdlib.h",
		"../common/imagelib.c", "../common/imagelib.h",
		"../common/inout.c", "../common/inout.h",
		"../common/mathlib.c", "../common/mathlib.h",
		"../common/md4.c", "../common/md4.h",
		"../common/mutex.c", "../common/mutex.h",
		"../common/polylib.c", "../common/polylib.h",
		"../common/scriplib.c", "../common/scriplib.h",
		"../common/threads.c", "../common/threads.h",
		"../common/unzip.c", "../common/unzip.h",
		"../common/vfs.c", "../common/vfs.h",
		
		"**.c", "**.cpp", "**.h",
		
		"../../libs/picomodel/**.c", "../../libs/picomodel/**.h",
		"../../libs/picomodel/lwo/**.c", "../../libs/picomodel/lwo/**.h",
		
		"../../libs/jpeg/**.c", "../../../libs/jpeg/**.h",
		"../../libs/png/**.c", "../../../libs/png/**.h",
		"../../libs/zlib/**.c", "../../../libs/zlib/**.h",
		--"../libs/openexr/**.cpp", "../../libs/openexr/**.h",
	}
	includedirs
	{
		"../common",
		"../../libs/picomodel",
		"../../libs/png",
		"../../libs/zlib",
	}
	defines
	{ 
		--"USE_XML", 
	}
	
	--
	-- Release/Debug Configurations
	--
	configuration "Debug"
		defines     "_DEBUG"
		flags       { "Symbols", "StaticRuntime" }
	
	configuration "Release"
		defines     "NDEBUG"
		flags       { "OptimizeSpeed", "StaticRuntime" }
	
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		--flags       {"WinMain"}
		files
		{
			"etxmap.ico",
			"etxmap.rc",
		}
		includedirs
		{
			"../../libs/glib/include/glib-2.0",
			"../../libs/glib/lib/glib-2.0/include",
			"../../libs/sdl/include",
		}
		libdirs
		{
			"../../libs/glib/lib",
			"../../libs/sdl/lib",
		}
		links
		{ 
			"wsock32",
			"glib-2.0",
			"SDL",
			"SDLmain",
			"opengl32",
			"glu32",
		}
		buildoptions
		{
			--"/EHa",
			--"/fp:fast",
			--"/arch:SSE"
		}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
			--"USE_INTERNAL_SPEEX",
			--"USE_INTERNAL_ZLIB",
			--"FLOATING_POINT",
			--"USE_ALLOCA"
		}
	
	configuration { "linux", "gmake" }
		buildoptions
		{
			"`pkg-config --cflags glib-2.0`",
			"`pkg-config --cflags sdl`",
		}
		linkoptions
		{
			"`pkg-config --libs glib-2.0`",
			"`pkg-config --libs sdl`",
		}

	configuration "linux"
		targetname  "etxmap"
		links
		{
			"GL",
			"GLU",
		}
		
