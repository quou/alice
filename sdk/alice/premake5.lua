project "alice"
	kind "SharedLib"
	language "C"
	cdialect "C99"
	
	staticruntime "on"

	targetdir "../../bin"
	objdir "obj"
	
	architecture "x64"
	pic "on"

	files {
		"include/**.h",
		"src/**.c"
	}

	includedirs {
		"../extern/glfw/include",
		"../extern/glad/include",
		"../extern/stb/include",
		"../extern/physfs/src",
		"../extern/lua/src",
		"../extern/assimp/include",
		"include"
	}

	links {
		"assimp",
		"glfw",
		"glad",
		"stb",
		"physfs"
	}

	defines {
		"ALICE_EXPORT_SYMBOLS",
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"

	filter "configurations:release"
		runtime "release"
		optimize "on"

	filter "system:linux"
		links {
			"pthread",
			"dl",
			"X11",
			"m",
			"GL",
		}

	filter "system:windows"
		links {
			"opengl32",
			"gdi32",
			"user32",
			"kernel32"
		}
