

project "rendererGL3"
	targetname  "rendererGL3"
	language    "C++"
	kind        "StaticLib"
	flags       { "ExtraWarnings" }
	files
	{
		"../../shared/**.c", "../../shared/**.h",
		
		"**.c", "**.cpp", "**.h",
		
		"../../libs/glew/src/glew.c",
		"../../libs/glew/src/glew.h",
		
		"../../libs/jpeg/**.c", "../../libs/jpeg/**.h",
		"../../libs/png/**.c", "../../libs/png/**.h",
		"../../libs/zlib/**.c", "../../libs/zlib/**.h",
		"../../libs/openexr/**.cpp", "../../libs/openexr/**.h",
		
		"../../libs/ft2/**.c", "../../libs/ft2/**.h",
	}
	includedirs
	{
		"../../libs/zlib",
		"../../libs/glew/include",
	}
	defines
	{ 
		"REF_HARD_LINKED",
		"GLEW_STATIC",
		"BUILD_FREETYPE",
		"FT2_BUILD_LIBRARY",
		--"PRODUCT_VERSION=\\\"ioquake3-trim-1.36\\\"",
	}
	
	--
	-- Platform Configurations
	-- 	
	configuration "x86"
		defines
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
	-- Project Configurations
	-- 
	configuration "vs*"
		files
		{
			"../win32/win_gamma.c",
			"../win32/win_gl3imp.c",
		}
		includedirs
		{
			"../../libs/glew/src/wglew.h",
		}
		links
		{ 
			"opengl32"
		}
		buildoptions
		{
			"/fp:fast",
			"/arch:SSE"
		}
		defines
		{
			"WIN32",
			"_CRT_SECURE_NO_WARNINGS",
		}
	
	
	-- configuration { "linux", "gmake" }
		-- TODO linkoptions { "`wx-config --ldflags`" }
	
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
			"PNG_NO_ASSEMBLER_CODE",
            "_GLFW_USE_LINUX_JOYSTICKS",
			"_GLFW_HAS_XRANDR",
			"_GLFW_HAS_PTHREAD",
			"_GLFW_HAS_SCHED_YIELD",
			"_GLFW_HAS_GLXGETPROCADDRESSARB",
			"_GLFW_HAS_DLOPEN",
			"_GLFW_HAS_SYSCONF"
		}