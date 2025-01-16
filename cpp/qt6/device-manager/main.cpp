
#include "DeviceSelectionDialog.h"

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	app.setApplicationName("ic4-device-manager");
	app.setApplicationDisplayName("IC4 Device Manager");
#if defined IC4_QTDIALOG_APPVERSION
	app.setApplicationVersion(IC4_QTDIALOG_APPVERSION);
#endif

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