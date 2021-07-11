project "sandbox"
	kind "ConsoleApp"
	language "C"
	cdialect "C99"

	staticruntime "on"

	targetdir "../bin"
	objdir "obj"
	
	architecture "x64"

	files {
		"src/**.c"
	}

	includedirs {
		"../sdk/alice/include"
	}

	links {
		"alice"
	}

	defines {
		"ALICE_IMPORT_SYMBOLS"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"
	
	filter "configurations:release"
		runtime "release"
		optimize "on"
