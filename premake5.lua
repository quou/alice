workspace "alice"
	configurations { "debug", "release" }

	startproject "sandbox"

group "sdk"
include "sdk/alice"

group "sdk/extern"
include "sdk/extern/glfw"
include "sdk/extern/glad"
include "sdk/extern/physfs"
include "sdk/extern/stb"
include "sdk/extern/assimp"
include "sdk/extern/miniaudio"
include "sdk/extern/microui"

group "projects"
include "sandbox"
group ""
