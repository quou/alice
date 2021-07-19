project "scripts"
	kind "SharedLib"
	language "C"
	cdialect "C99"

	staticruntime "on"

	targetdir "../"
	objdir "obj"
	
	architecture "x64"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"../../sdk/alice/include"
	}

	defines {
		"ALICE_EXPORT_SYMBOLS"
	}

	links {
		"alice"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"
	
	filter "configurations:release"
		runtime "release"
		optimize "on"
