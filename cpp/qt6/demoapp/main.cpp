#include <QApplication>
#include "mainwindow.h"
#ifdef WIN32__
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	int argc = 0;
	QApplication a(argc, NULL);

#else
int  main(int argc, char* argv[])
{
	QApplication a(argc, argv);
#endif

	auto x = a.setStyle("fusion");
	ic4::InitLibraryConfig conf = {};
	conf.apiLogLevel = ic4::LogLevel::Warning;
	conf.logTargets = ic4::LogTarget::WinDebug;
	conf.defaultErrorHandlerBehavior = ic4::ErrorHandlerBehavior::Throw;

	ic4::initLibrary(conf);
	MainWindow w;
	w.show();
	return a.exec();
}