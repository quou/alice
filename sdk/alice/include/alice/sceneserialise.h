#pragma once

#include "alice/core.h"
#include "alice/entity.h"

ALICE_API void alice_serialise_scene(alice_Scene* scene, const char* file_path);
ALICE_API void alice_deserialise_scene(alice_Scene* scene, const char* file_path);
