project "physfs"
	kind "StaticLib"
	language "C"
	
	staticruntime "on"

	architecture "x64"

	pic "on"

	targetdir "bin"
	objdir "obj"

	files {
		"src/**.c",
		"src/**.h"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"

	filter "configurations:release"
		runtime "release"
		optimize "on"
