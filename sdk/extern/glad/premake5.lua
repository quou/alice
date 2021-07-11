project "glad"
	kind "StaticLib"
	language "C"
	staticruntime "on"

	architecture "x64"

	targetdir "bin"
	objdir "obj"

	pic "on"

	includedirs {
		"include"
	}

	files {
		"include/glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	filter "configurations:debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:release"
		runtime "Release"
		optimize "on"
