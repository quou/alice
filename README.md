![logo](https://raw.githubusercontent.com/veridisquot/alice/master/sandbox/res/splash.png)

![screenshot](https://raw.githubusercontent.com/veridisquot/alice/master/screenshots/001.png)

A high performance game engine written in C99 that makes
use of Data Oriented Design.

The renderer requires OpenGL 4.3+. Simply because I refuse
to code in OpenGL without the message callback feature -
the renderer doesn't use any actual features that OpenGL 3.3
doesn't have.

## Current Features
 - Entity management
 - 3D rendering
 - Resource management

## Roadmap
 - Scripting
 - Level editor
 - Custom shading language
 - Vulkan renderer
 - Software renderer
 - Raytraced renderer
 - C++11 interface

## Building
Generate build files using Premake5. Tested with GCC
on Gentoo and Linux Mint, and MSVC on Windows. Mac OS
probably doesn't work.

Mostly uses standard C99, though scripting relies on the
nonstandard feature where you can cast a void pointer
into a function pointer.
