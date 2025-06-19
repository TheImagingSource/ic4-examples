
#include "settings.h"

#include <QSettings>

void Settings::read()
{
	QSettings s;

	start_full_screen = s.value("start_full_screen", start_full_screen).toBool();
	full_screen_show_menu_bar = s.value("full_screen_show_menu_bar", full_screen_show_menu_bar).toBool();
	full_screen_show_tool_bar = s.value("full_screen_show_tool_bar", full_screen_show_tool_bar).toBool();
	full_screen_show_status_bar = s.value("full_screen_show_status_bar", full_screen_show_status_bar).toBool();

	switch (s.value("default_visibility", (int)default_visibility).toInt())
	{
	case 1:	default_visibility = ic4::PropVisibility::Expert; break;
	case 2:	default_visibility = ic4::PropVisibility::Guru; break;
	case 3:	default_visibility = ic4::PropVisibility::Invisible; break;
	case 0:
	default:
		default_visibility = ic4::PropVisibility::Beginner;
		break;
	}

	show_settings_menu = s.value("show_settings_menu", show_settings_menu).toBool();
	start_stream_on_open = s.value("start_stream_on_open", start_stream_on_open).toBool();
}

void Settings::write()
{
	QSettings s;

	s.setValue("default_visibility", (int)default_visibility);
	s.setValue("start_full_screen", start_full_screen);
	s.setValue("full_screen_show_menu_bar", full_screen_show_menu_bar);
	s.setValue("full_screen_show_status_bar", full_screen_show_status_bar);
	s.setValue("full_screen_show_tool_bar", full_screen_show_tool_bar);

	s.setValue("show_settings_menu", show_settings_menu);
	s.setValue("start_stream_on_open", start_stream_on_open);

}