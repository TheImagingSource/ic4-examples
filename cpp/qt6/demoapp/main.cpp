
#include "mainwindow.h"

#include <QApplication>


int main(int argc, char* argv[])
{
	//qputenv("QT_COMMAND_LINE_PARSER_NO_GUI_MESSAGE_BOXES", "1");

	QApplication app(argc, argv);
	QApplication::setApplicationName("ic4-demoapp");
	QApplication::setApplicationDisplayName("IC4 Demo Application");
	QApplication::setStyle("fusion");
#if defined IC4_QTDIALOG_APPVERSION
	QApplication::setApplicationVersion(IC4_QTDIALOG_APPVERSION);
#endif

	MainWindow::init_options init = {
		/*.appDataDirectory =*/ ic4demoapp::QString_to_fspath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)),
		/*.deviceSetupFile =*/ std::nullopt,
		/*.show_settings_menu = */
	};

	bool only_show_program_version = false;

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

		QCommandLineOption cli_option_settings("settings", "Add the settings menu to the menu bar. Default='0'.", "enable");
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
			only_show_program_version = true;
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

	if (only_show_program_version)
	{
		std::string version_text = app.applicationDisplayName().toStdString();
		auto app_ver = app.applicationVersion();
		if (!app_ver.isEmpty()) {
			version_text += " " + app_ver.toStdString();
		}
		version_text += "\n\n";
		version_text += ic4::getVersionInfo(ic4::VersionInfoFlags::Default);

#if defined _WIN32
		QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
				 "<html><head/><body><pre>"
				 + QString::fromStdString(version_text) + "</pre></body></html>");
#else
		std::fputs(version_text.c_str(), stdout);

		//parser.showVersion();
#endif
		return 0;
	}


	MainWindow mainWindow(init);
	mainWindow.show();

	return app.exec();
}