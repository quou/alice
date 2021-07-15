#pragma once

extern "C" {
	#include <alice/application.h>
}

void init_gui(alice_Application* app);
void quit_gui();
void gui_begin_frame();
void gui_end_frame();
