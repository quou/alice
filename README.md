![logo](https://raw.githubusercontent.com/veridisquot/alice/master/sandbox/res/splash.png)

![screenshot](https://raw.githubusercontent.com/veridisquot/alice/master/screenshots/005.png)

![screenshot](https://raw.githubusercontent.com/veridisquot/alice/master/screenshots/stress.png)

A high performance game engine written in C99 that makes
use of Data Oriented Design.

The renderer requires OpenGL 4.3+. Simply because I refuse
to code in OpenGL without the message callback feature -
the renderer doesn't use any actual features that OpenGL 3.3
doesn't have.

## Current Features
 - Entity management
 - 3D rendering
 - 2D rendering
 - Resource management
 - Custom text serialisation format
 - Scene serialisation
 - Scripting
 - Physics
 - GUI

## Roadmap
 - Level editor
 - Audio
 - Custom shading language
 - Vulkan renderer
 - Software renderer
 - Raytraced renderer
 - C++11 interface

## Building
Generate build files using Premake5. Tested with GCC
on Gentoo and Linux Mint, and MSVC on Windows. Mac OS
probably doesn't work.

## Architecture overview
Alice handles entities in a way that's fairly unique - It's somewhere halfway
between a purely data oriented ECS and a traditional inheritance model.

The first part of the puzzle is the `alice_entity_handle_t`. This is a 64 bit unsigned
integer, where the first 32 bits represents an index into an entity array for the specific
entity type, with the last 32 bits being the id of the type of entity. Type IDs are
generated using a simple hash function on the string name of the type. Entity handles are
used to get back a generic pointer from the scene to the actual entity object.

Alice's entity system uses something that I like to call "struct inheritance", because it works
somewhat similarly to single inheritance in object oriented languages - The base entity
looks like a little like this (simplified):

```c
struct alice_entity_t {
	char* name;

	alice_v3f_t position;
	alice_v3f_t rotation;
	alice_v3f_t scale;

	alice_entity_handle_t parent;

	/* Array of children */
	alice_entity_handle_t children[];
};
```

An "inherited" entity looks a little like this:

```c
struct alice_renderable_3d_t {
	alice_entity_t base;

	alice_model_t* model;
	alice_material_t* materials[];
};
```

Because of the way that structs are aligned in memory, a pointer to an `alice_renderable_3d_t`
can be cast back to an `alice_entity_t` pointer and be operated on - and vise versa, as long
as enough memory has been allocated for the "inherited" struct. This way, generic functions
can be created that take in `alice_entity_t` pointers, for things such as calculating a
transform matrix for a given entity.

A scene contains an array of "entity pools", one entry for each unique type of entity,
and within each pool, entities are sub-allocated from a contiguous block.
This way, by iterating a pool for a specific entity type, logic can be applied to
all entities of that type - for example the 3D renderer iterates all entities of
type `alice_renderable_3d_t`, and draws them to the screen.

The only disadvantage that this gives is that entities of different types cannot
be combined like in a pure ECS - thus, we are stuck with a similar issue that the 
traditional inheritance model had. Alice gets around this in a similar way to Godot
(another engine that uses an inheritance model instead of ECS 
[[Source](https://godotengine.org/article/why-isnt-godot-ecs-based-game-engine)]) - 
by harnessing entity parenting in order to combine different entity behaviours.
This does create the issue that more entities have to be created in order to
achieve a similar effect, though since iteration of said entities is quick and
cache-friendly, these extra entities are more than made up for.
