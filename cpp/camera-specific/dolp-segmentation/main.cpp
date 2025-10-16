
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setApplicationDisplayName("DoLP Segmentation");

	app.setStyle("fusion");

	ic4::InitLibraryConfig conf = {};
	conf.apiLogLevel = ic4::LogLevel::Warning;
	conf.logTargets = ic4::LogTarget::WinDebug;
	conf.defaultErrorHandlerBehavior = ic4::ErrorHandlerBehavior::Throw;
	ic4::initLibrary(conf);

	MainWindow w;
	w.show();

	return app.exec();
}