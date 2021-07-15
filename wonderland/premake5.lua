project "wonderland"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	staticruntime "on"

	targetdir "../bin"
	objdir "obj"
	
	architecture "x64"

	files {
		"src/**.hpp",
		"src/**.cpp",
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"../sdk/alice/include",
		"../sdk/extern/glfw/include",
		"../sdk/extern/glad/include"
	}

	links {
		"alice",
		"glad",
		"glfw"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"
	
	filter "configurations:release"
		runtime "release"
		optimize "on"
