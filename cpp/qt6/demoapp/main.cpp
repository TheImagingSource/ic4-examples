
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

	MainWindow::init_options init = {};

	{

		auto appDataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QDir(appDataDirectory).mkpath(".");

		auto device_setup_path = std::filesystem::path(appDataDirectory.toStdString()) / "device.json";

		QCommandLineParser parser;
		parser.setApplicationDescription("IC4 Demo Application");
		auto helpOption = parser.addHelpOption();
		auto versionOption = parser.addVersionOption();

		QCommandLineOption cli_option_device_file("device-file");
		cli_option_device_file.setDescription(
			QString("Sets the device-setup-file to load on startup. Default='") % QString::fromStdString(device_setup_path.string()) % "'.)"
		);
		cli_option_device_file.setValueName("file");
		parser.addOption(cli_option_device_file);

		QCommandLineOption cli_option_settings("settings");
		cli_option_settings.setDescription("Show settings menu");
		cli_option_settings.setValueName("enable");
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
			init.deviceSetupFile = std::filesystem::path(parser.value(cli_option_device_file).toStdString());
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