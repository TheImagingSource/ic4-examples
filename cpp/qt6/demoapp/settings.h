
#pragma once

#include <ic4/ic4.h>

struct Settings
{
	bool start_full_screen = false;
	bool full_screen_show_menu_bar = false;
	bool full_screen_show_tool_bar = false;
	bool full_screen_show_status_bar = false;
	ic4::PropVisibility default_visibility = ic4::PropVisibility::Beginner;

	bool show_settings_menu = false;
	bool start_stream_on_open = true;

	void read();
	void write();
};