/*
* Helper for OS dependend functions
*/

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <shlobj_core.h>
#include <direct.h>

std::string getAppSpecialfolder(std::string appname, int csidl)
{
	std::string appdatadir;
	char appdata[MAX_PATH + 1];
	SHGetSpecialFolderPathA(NULL, appdata, csidl, false);
	appdatadir = appdata;
	appdatadir += "\\";
	appdatadir += appname;

	struct stat info;
	if (stat(appdatadir.c_str(), &info) != 0)
	{
		_mkdir(appdatadir.c_str());
	}
	return appdatadir;
}

std::string getAppDataDir(std::string appname)
{
	return getAppSpecialfolder(appname, CSIDL_APPDATA);
}

std::string getPictureDir(std::string appname)
{
	return getAppSpecialfolder(appname, CSIDL_MYPICTURES);
}

std::string getVideoDir(std::string appname)
{
	return getAppSpecialfolder(appname, CSIDL_MYVIDEO);
}

bool fileexist(std::string name)
{
	struct stat info;
	return (stat(name.c_str(), &info) == 0);
}
#endif