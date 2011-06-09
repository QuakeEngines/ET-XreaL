
project "ETXreaL"
	targetname  "ETXreaL"
	language    "C++"
	kind        "WindowedApp"
	targetdir 	"../.."
	flags       { "ExtraWarnings" }
	files
	{
		"*.txt", 
		
		"../shared/**.c", "../shared/**.h",
		
		"botlib/**.c", "botlib/**.h",
		
		"client/**.c", "client/**.h",
		"server/**.c", "server/**.h",
		
		"splines/**.cpp", "splines/**.h",
		
		"qcommon/**.h", 
		"qcommon/cmd.c",
		"qcommon/common.c",
		"qcommon/cvar.c",
		"qcommon/files.c",
		"qcommon/huffman.c",
		"qcommon/md4.c",
		"qcommon/msg.c",
		"qcommon/vm.c",
		"qcommon/vm_interpreted.c",
		"qcommon/net_*.c",
		"qcommon/cm_*.c",
		"qcommon/unzip.c",
		
		"qcommon/dl_main_curl.c",
		"qcommon/dl_**.h",
		
		"rendererGL3/**.c", "rendererGL3/**.cpp", "rendererGL3/**.h",
		
		"../libs/glew/src/glew.c",
		"../libs/glew/src/glew.h",
		
		"../libs/jpeg/**.c", "../../libs/jpeg/**.h",
		"../libs/png/**.c", "../../libs/png/**.h",
		"../libs/zlib/**.c", "../../libs/zlib/**.h",
		"../libs/openexr/**.cpp", "../../libs/openexr/**.h",
		
		"../libs/ft2/**.c", "../../libs/ft2/**.h",
	}
	includedirs
	{
		"../libs/zlib",
		"../libs/glew/include",
	}
	defines
	{ 
		"REF_HARD_LINKED",
		"GLEW_STATIC",
		"BUILD_FREETYPE",
		"FT2_BUILD_LIBRARY",
		"BOTLIB",
		--"USE_CURL", 
		--"USE_CODEC_VORBIS", 
		--"USE_CIN_THEORA",
		--"USE_MUMBLE",
		--"USE_VOIP",
		--"USE_SSE=2",
		--"USE_INTERNAL_GLFW",
		--"USE_INTERNAL_GLEW",
		--"PRODUCT_VERSION=\\\"ioquake3-trim-1.36\\\"",
	}
	excludes
	{
		"botlib/botlib_stub.c",
		"server/sv_rankings.c",
	}
	
	--
	-- Platform Configurations
	-- 	
	configuration "x86"
		files
		{ 
			"code/qcommon/vm_x86.c",
		}
	
	--configuration "x64"
	--	files
	--	{ 
	--		"qcommon/vm_x86_64.c",
	--		"qcommon/vm_x86_64_assembler.c",
	--	}
	
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
	-- Options Configurations
	--
	--configuration "with-freetype"
	--	links        { "freetype" }
	--	buildoptions { "`pkg-config --cflags freetype2`" }
	--	defines      { "BUILD_FREETYPE" }

	--configuration "openal-dlopen"
	--	defines      
	--	{
	--		"USE_OPENAL",
	--		"USE_OPENAL_DLOPEN",
	--		"USE_OPENAL_LOCAL_HEADERS"
	--	}
		
	--configuration "openal-link"
	--	links        { "openal " }
	--	defines      { "USE_OPENAL" }

	
	--configuration { "vs*", "Release" }
	-- newaction {
		-- trigger = "prebuild",
		-- description = "Compile libcurl.lib",
		-- execute = function ()
			-- os.execute("cd ../libs/curl-7.12.2;cd lib;nmake /f Makefile.vc6 CFG=release")
		-- end
	-- }
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "WinMain" }
		files
		{
			"win32/**.c", "win32/**.cpp", "win32/**.h",
			"win32/winquake.rc",
			"win32/win_gamma.c",
			"win32/win_gl3imp.c",
			
			--"code/libspeex/**.c", "code/libspeex/**.h",
			--"code/zlib/**.c", "code/zlib/**.h",
			--"code/libcurl/**.c", "code/libcurl/**.h",
			--"code/ogg_vorbis/**.c", "code/ogg_vorbis/**.h",
			--"code/glfw/lib/win32/*.c", "code/glfw/lib/win32/*.h",
		}
		excludes
		{
			--"win32/win_gamma.c",
			"win32/win_glimp.c",
		}
		includedirs
		{
			"../libs/glew/src/wglew.h",
		}
		libdirs
		{
			"../libs/curl-7.12.2/lib"
		}
		links
		{ 
			"dinput8",
			"winmm",
			"wsock32",
			"Iphlpapi",
			"libcurl",
			"opengl32",
		}
		--linkoptions
		--{
		--	"/NODEFAULTLIB:libc"
		--}
		buildoptions
		{
			"/EHa",
			"/fp:fast",
			"/arch:SSE"
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
		prebuildcommands
		{
		   "cd ../libs/curl-7.12.2",
		   "cd lib",
		   "nmake /f Makefile.vc6 CFG=release",
		}
	
	configuration "linux"
		files
		{
			"code/sys/sys_unix.c",
			"code/sys/con_log.c",
			"code/sys/con_tty.c",
			
			"code/glfw/lib/x11/*.c", "code/glfw/lib/x11/*.h",
		}
		includedirs
		{
			"code/glfw/lib/x11",
		}
		links
		{ 
			"m", 
			"dl", 
			"z", 
			"GL",
			"IL",
			"ILU",
			"ILUT",
			"theora",
			"speex",
			"speexdsp",
			"curl",
			"ogg",
			"vorbis",
			"vorbisfile",
			"X11",
			"Xrandr",
			"pthread"
		}
		buildoptions
		{
			"-pthread"
		}
		defines
		{
            "_GLFW_USE_LINUX_JOYSTICKS",
			"_GLFW_HAS_XRANDR",
			"_GLFW_HAS_PTHREAD",
			"_GLFW_HAS_SCHED_YIELD",
			"_GLFW_HAS_GLXGETPROCADDRESSARB",
			"_GLFW_HAS_DLOPEN",
			"_GLFW_HAS_SYSCONF"
		}
		
