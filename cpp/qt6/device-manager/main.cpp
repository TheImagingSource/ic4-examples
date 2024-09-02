
#include "DeviceSelectionDialog.h"

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setApplicationDisplayName("IC4 Device Manager");

	app.setStyle("fusion");

	ic4::InitLibraryConfig conf = {};
	conf.apiLogLevel = ic4::LogLevel::Warning;
	conf.logTargets = ic4::LogTarget::WinDebug;
	conf.defaultErrorHandlerBehavior = ic4::ErrorHandlerBehavior::Throw;
	ic4::initLibrary(conf);

	DeviceSelectionDialog dlg(nullptr, nullptr);
	dlg.setWindowTitle("");
	dlg.show();

	return app.exec();
}