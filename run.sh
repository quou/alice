premake5 gmake
if make; then
	cd sandbox
	./../bin/sandbox
fi
