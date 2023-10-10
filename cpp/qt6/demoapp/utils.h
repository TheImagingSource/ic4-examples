#pragma once

#include <string>

/// <summary>
/// Get the directory where the program stores data which is automaticalle restored
/// Get the picture and video directories.
/// </summary>
/// <param name="appname">Name of application subdir</param>
/// <returns>%appdata%/appname windows, ~/.appname Linux.</returns>
std::string getAppDataDir(std::string appname);
std::string getPictureDir(std::string appname);
std::string getVideoDir(std::string appname);

bool fileexist(std::string name);
