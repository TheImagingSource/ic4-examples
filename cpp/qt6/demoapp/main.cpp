
#include "mainwindow.h"

#include <QApplication>


int main(int argc, char* argv[])
{
	//qputenv("QT_COMMAND_LINE_PARSER_NO_GUI_MESSAGE_BOXES", "1");

	QApplication app(argc, argv);
	QApplication::setApplicationName("ic4-demoapp");
#if defined IC4_DEMOAPP_VERSION_LINE
	QApplication::setApplicationVersion(IC4_DEMOAPP_VERSION_LINE);
#endif
	QApplication::setApplicationDisplayName("IC4 Demo Application");
	QApplication::setStyle("fusion");

	MainWindow::init_options init = {
		/*.appDataDirectory =*/ ic4demoapp::QString_to_fspath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)),
		/*.deviceSetupFile =*/ std::nullopt,
	};

	{
		auto device_setup_path = init.appDataDirectory / "device.json";

		QCommandLineParser parser;
		parser.setApplicationDescription("IC4 Demo Application");
		auto helpOption = parser.addHelpOption();
		auto versionOption = parser.addVersionOption();

		QCommandLineOption cli_option_device_file("device-file",
			"Sets the device state file to load on startup. Default='" % QString::fromStdString(device_setup_path.string()) % "'.)",
			"file"
		);
		parser.addOption(cli_option_device_file);

		QCommandLineOption cli_option_app_data_dir("app-data-directory",
			"Sets the directory to load/store program settings like 'last open device' and 'codec-config'."
			" Default='" % QString::fromStdString(init.appDataDirectory.string()) % "'.)",
			"directory"
		);
		parser.addOption(cli_option_app_data_dir);

		QCommandLineOption cli_option_settings("settings", "Show settings menu", "enable");
		parser.addOption(cli_option_settings);

		auto res = parser.parse(QApplication::arguments());
		if (!res) {
#if defined _WIN32
			QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
								 "<html><head/><body><h2>" + parser.errorText() + "</h2><pre>"
								 + parser.helpText() + "</pre></body></html>");
#else
			std::fputs(qPrintable(parser.errorText()), stderr);
			std::fputs("\n\n", stderr);
			std::fputs(qPrintable(parser.helpText()), stderr);
#endif
			return 1;
		}

		if (parser.isSet(helpOption))
		{
#if defined _WIN32
			QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
					 "<html><head/><body><pre>"
					 + parser.helpText() + "</pre></body></html>");
#else
			parser.showHelp();
#endif
			return 0;
		}
		if (parser.isSet(versionOption))
		{
			parser.showVersion();
			return 0;
		}
		if (parser.isSet(cli_option_device_file))
		{
			init.deviceSetupFile = ic4demoapp::QString_to_fspath(parser.value(cli_option_device_file));
		}
		if (parser.isSet(cli_option_app_data_dir))
		{
			init.appDataDirectory = ic4demoapp::QString_to_fspath(parser.value(cli_option_app_data_dir));
		}
		if (parser.isSet(cli_option_settings))
		{
			init.show_settings_menu = parser.value(cli_option_settings).toInt() != 0;
		}
	}

	ic4::InitLibraryConfig conf = {};
	conf.apiLogLevel = ic4::LogLevel::Warning;
	conf.logTargets = ic4::LogTarget::WinDebug;
	conf.defaultErrorHandlerBehavior = ic4::ErrorHandlerBehavior::Throw;
	ic4::initLibrary(conf);

	MainWindow mainWindow(init);
	mainWindow.show();

	return app.exec();
}