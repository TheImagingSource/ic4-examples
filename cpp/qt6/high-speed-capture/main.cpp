
#include "HighSpeedCaptureDialog.h"

#include <ic4/ic4.h>

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setApplicationDisplayName("IC4 High Speed Capture");

	app.setStyle("fusion");

	ic4::InitLibraryConfig conf = {};
	conf.apiLogLevel = ic4::LogLevel::Warning;
	conf.logTargets = ic4::LogTarget::WinDebug;
	conf.defaultErrorHandlerBehavior = ic4::ErrorHandlerBehavior::Throw;
	ic4::initLibrary(conf);

	HighSpeedCaptureDialog dlg;
	dlg.show();

	return app.exec();
}