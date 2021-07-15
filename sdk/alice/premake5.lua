project "alice"
	kind "StaticLib"
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
		"include"
	}

	links {
		"glfw",
		"glad",
		"stb",
		"physfs"
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
			"GL"
		}

	filter "system:windows"
		links {
			"opengl32",
			"gdi32",
			"user32",
			"kernel32"
		}
