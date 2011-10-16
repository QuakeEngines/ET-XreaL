
project "ETXreaL"
	targetname  "ETXreaL"
	language    "C++"
	kind        "WindowedApp"
	flags       { "ExtraWarnings" }
	files
	{
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
		"qcommon/md5.c",
		"qcommon/msg.c",
		"qcommon/vm.c",
		"qcommon/vm_interpreted.c",
		"qcommon/net_*.c",
		"qcommon/cm_*.c",
		"qcommon/unzip.c",
		
		"qcommon/dl_**.h",
		"qcommon/dl_main_curl.c",
		
		"rendererGL3/**.c", "rendererGL3/**.cpp", "rendererGL3/**.h",
		
		"../libs/glew/src/glew.c",
		"../libs/glew/src/glew.h",
		
		"../libs/jpeg/**.c", "../../libs/jpeg/**.h",
		"../libs/png/**.c", "../../libs/png/**.h",
		"../libs/zlib/**.c", "../../libs/zlib/**.h",
		"../libs/openexr/**.cpp", "../../libs/openexr/**.h",
		
		--"../libs/ft2/**.c", "../../libs/ft2/**.h",
		
		"../libs/freetype/src/autofit/autofit.c",
		"../libs/freetype/src/bdf/bdf.c",
		"../libs/freetype/src/cff/cff.c",
		"../libs/freetype/src/base/ftbase.c",
		"../libs/freetype/src/base/ftbitmap.c",
		"../libs/freetype/src/cache/ftcache.c",
		"../libs/freetype/src/base/ftdebug.c",
		"../libs/freetype/src/base/ftgasp.c",
		"../libs/freetype/src/base/ftglyph.c",
		"../libs/freetype/src/gzip/ftgzip.c",
		"../libs/freetype/src/base/ftinit.c",
		"../libs/freetype/src/lzw/ftlzw.c",
		"../libs/freetype/src/base/ftstroke.c",
		"../libs/freetype/src/base/ftsystem.c",
		"../libs/freetype/src/smooth/smooth.c",
		"../libs/freetype/src/base/ftbbox.c",
		"../libs/freetype/src/base/ftmm.c",
		"../libs/freetype/src/base/ftpfr.c",
		"../libs/freetype/src/base/ftsynth.c",
		"../libs/freetype/src/base/fttype1.c",
		"../libs/freetype/src/base/ftwinfnt.c",
		"../libs/freetype/src/pcf/pcf.c",
		"../libs/freetype/src/pfr/pfr.c",
		"../libs/freetype/src/psaux/psaux.c",
		"../libs/freetype/src/pshinter/pshinter.c",
		"../libs/freetype/src/psnames/psmodule.c",
		"../libs/freetype/src/raster/raster.c",
		"../libs/freetype/src/sfnt/sfnt.c",
		"../libs/freetype/src/truetype/truetype.c",
		"../libs/freetype/src/type1/type1.c",
		"../libs/freetype/src/cid/type1cid.c",
		"../libs/freetype/src/type42/type42.c",
		"../libs/freetype/src/winfonts/winfnt.c",
	}
	includedirs
	{
		"../libs/zlib",
		"../libs/glew/include",
		"../libs/freetype/include",
	}
	defines
	{ 
		"REF_HARD_LINKED",
		"GLEW_STATIC",
		"BUILD_FREETYPE",
		"FT2_BUILD_LIBRARY",
		"BOTLIB",
		"USE_BOTLIB",
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
	configuration "x32"
		files
		{ 
			"code/qcommon/vm_x86.c",
		}
	
	--configuration "x64"
	--	targetdir 	"../../bin/x64"
	--	files
	--	{ 
	--		"qcommon/vm_x86_64.c",
	--		"qcommon/vm_x86_64_assembler.c",
	--	}
		
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
			
			"../libs/glew/src/wglew.h",
			
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
			"win32/win_eh.cpp",
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
			"opengl32",
		}
		--linkoptions
		--{
		--	"/NODEFAULTLIB:libc"
		--}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
			--"USE_INTERNAL_SPEEX",
			--"USE_INTERNAL_ZLIB",
			--"FLOATING_POINT",
			--"USE_ALLOCA"
		}
		
		
	configuration { "vs*", "x32" }
		targetdir 	"../../bin/win32"
		libdirs
		{
			"../libs/curl-7.12.2/lib"
		}
		links
		{ 
			"libcurl",
		}
		prebuildcommands
		{
		   "cd ../libs/curl-7.12.2",
		   "cd lib",
		   "nmake /f Makefile.vc6 CFG=release",
		}
		
	configuration { "vs*", "x64" }
		targetdir 	"../../bin/win64"
		--files
		--{
		--	"qcommon/dl_main_stubs.c",
		--}

	configuration { "linux", "gmake" }
		buildoptions
		{
			"`pkg-config --cflags x11`",
			"`pkg-config --cflags xext`",
			"`pkg-config --cflags xxf86dga`",
			"`pkg-config --cflags xxf86vm`",
			"`pkg-config --cflags sdl`",
			"`pkg-config --cflags libcurl`",
		}
		linkoptions
		{
			"`pkg-config --libs x11`",
			"`pkg-config --libs xext`",
			"`pkg-config --libs xxf86dga`",
			"`pkg-config --libs xxf86vm`",
			"`pkg-config --libs sdl`",
			"`pkg-config --libs libcurl`",
		}
	
	configuration { "linux", "x32" }
		targetdir 	"../../bin/linux-x86"
		
	configuration { "linux", "x64" }
		targetdir 	"../../bin/linux-x86_64"
	
	configuration "linux"
		targetname  "etxreal"
		files
		{
			"unix/linux_signals.c",
			"unix/unix_main.c",
			"unix/unix_net.c",
			"unix/unix_shared.c",
			"unix/sdl_snd.c",
			"unix/linux_joystick.c",
			"unix/linux_gl3imp.c",
			"../libs/glew/src/glew.c",
		}
		--buildoptions
		--{
		--	"-pthread"
		--}
		links
		{
			"GL",
		}
		defines
		{
            "PNG_NO_ASSEMBLER_CODE",
		}




project "ETXreaL-dedicated"
	targetname  "ETXreaL-dedicated"
	language    "C++"
	kind        "WindowedApp"
	flags       { "ExtraWarnings" }
	files
	{
		"../shared/**.c", "../shared/**.h",
		
		"botlib/**.c", "botlib/**.h",
		"server/**.c", "server/**.h",
		
		"splines/**.cpp", "splines/**.h",

		"null/null_client.c",
		"null/null_input.c",
		"null/null_snddma.c",
		
		"qcommon/**.h", 
		"qcommon/cmd.c",
		"qcommon/common.c",
		"qcommon/cvar.c",
		"qcommon/files.c",
		"qcommon/huffman.c",
		"qcommon/md4.c",
		"qcommon/md5.c",
		"qcommon/msg.c",
		"qcommon/vm.c",
		"qcommon/vm_interpreted.c",
		"qcommon/net_*.c",
		"qcommon/cm_*.c",
		"qcommon/unzip.c",
		
		"qcommon/dl_main_stubs.c",
		"qcommon/dl_**.h",
		
		"../libs/zlib/**.c", "../../libs/zlib/**.h",
	}
	includedirs
	{
		"../libs/zlib",
	}
	defines
	{ 
		"DEDICATED",
		"BOTLIB",
		"USE_BOTLIB",
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
	configuration "x32"
		files
		{ 
			"code/qcommon/vm_x86.c",
		}
	
	--configuration "x64"
	--	targetdir 	"../../bin64"
	
	-- 
	-- Project Configurations
	-- 
	configuration "vs*"
		flags       { "WinMain" }
		files
		{
			"win32/win_local.h",
			"win32/win_eh.cpp",
			"win32/win_main.c",
			"win32/win_net.c",
			"win32/win_shared.c",
			"win32/win_syscon.c",
			"win32/win_wndproc.c",
			"win32/winquake.rc",
		}
		excludes
		{
			--"win32/win_gamma.c",
			--"win32/win_glimp.c",
		}
		libdirs
		{
			--"../libs/curl-7.12.2/lib"
		}
		links
		{ 
			"winmm",
			"wsock32",
			"Iphlpapi",
		}
		--linkoptions
		--{
		--	"/NODEFAULTLIB:libc"
		--}
		buildoptions
		{
			"/EHa",
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
		
	configuration { "vs*", "x32" }
		targetdir 	"../../bin/win32"
		
	configuration { "vs*", "x64" }
		targetdir 	"../../bin/win64"

	configuration { "linux", "gmake" }
		buildoptions
		{
			--"`pkg-config --cflags sdl`",
		}
		linkoptions
		{
			--"`pkg-config --libs sdl`",
		}
		
	configuration { "linux", "x32" }
		targetdir 	"../../bin/linux-x86"
		
	configuration { "linux", "x64" }
		targetdir 	"../../bin/linux-x86_64"
	
	configuration "linux"
		targetname  "etxreal-dedicated"
		files
		{
			"unix/linux_signals.c",
			"unix/unix_main.c",
			"unix/unix_net.c",
			"unix/unix_shared.c",
		}
		--buildoptions
		--{
		--	"-pthread"
		--}
		links
		{
			"dl",
			"m",
		}
		defines
		{
            "PNG_NO_ASSEMBLER_CODE",
		}
		
