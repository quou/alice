include "scripts"

project "sandbox"
	kind "ConsoleApp"
	language "C++"

	staticruntime "on"

	targetdir "../bin"
	objdir "obj"
	
	architecture "x64"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"../sdk/extern/glad/include",
		"../sdk/alice/include"
	}

	defines {
		"ALICE_IMPORT_SYMBOLS"
	}

	links {
		"alice",
		"glad"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"
	
	filter "configurations:release"
		runtime "release"
		optimize "on"
