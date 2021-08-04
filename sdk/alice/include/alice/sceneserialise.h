#pragma once

#include "alice/core.h"
#include "alice/entity.h"

ALICE_API void alice_serialise_scene(alice_scene_t* scene, const char* file_path);
ALICE_API void alice_deserialise_scene(alice_scene_t* scene, const char* file_path);
