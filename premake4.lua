--
-- ETXreaL build configuration script
-- 
solution "ETXreaL"
	--configurations { "Release", "ReleaseWithSymbols", "Debug" }
	configurations { "Release", "Debug" }
	platforms {"x32", "x64"}
	
	--
	-- Release/Debug Configurations
	--
	configuration "Release"
		defines     "NDEBUG"
		flags      
		{
			"OptimizeSpeed",
			"EnableSSE",
			"StaticRuntime"
		}
		
	--configuration "ReleaseReleaseWithSymbols"
	--	defines     "NDEBUG"
	--	flags
	--	{
	--		"OptimizeSpeed",
	--		"EnableSSE",
	--		"Symbols",
	--		"StaticRuntime"
	--	}
	
	configuration "Debug"
		defines     "_DEBUG"
		flags
		{
			"Symbols",
			"StaticRuntime"
		}
	
--
-- Options
--
--newoption
--{
--	trigger = "with-freetype",
--	description = "Compile with freetype support"
--}
		
--newoption
--{
--	trigger = "with-openal",
--	value = "TYPE",
--	description = "Specify which OpenAL library",
--	allowed = 
--	{
--		{ "none", "No support for OpenAL" },
--		{ "dlopen", "Dynamically load OpenAL library if available" },
--		{ "link", "Link the OpenAL library as normal" },
--		{ "openal-dlopen", "Dynamically load OpenAL library if available" },
--		{ "openal-link", "Link the OpenAL library as normal" }
--	}
--}

--		
-- Platform specific defaults
--

-- We don't support freetype on VS platform
--if _ACTION and string.sub(_ACTION, 2) == "vs" then
--	_OPTIONS["with-freetype"] = false
--end

-- Default to dlopen version of OpenAL
--if not _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "dlopen"
--end
--if _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "openal-" .. _OPTIONS["with-openal"]
--end

include "src/engine"
--include "src/engine/rendererGL3"
include "etmain/src/game"
include "etmain/src/cgame"
include "etmain/src/ui"

include "src/tools/etxmap"

if not os.is("linux") and not os.is64bit() then
	include "omni-bot/src"
end

