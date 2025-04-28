
#include <QString>
#include <filesystem>

namespace ic4demoapp
{
	inline auto QString_to_fspath(const QString& str) -> std::filesystem::path {
#if defined _WIN32
		return str.toStdWString();
#else
		return str.toStdString();
#endif
	}

	inline auto fspath_to_QString(const std::filesystem::path& str) -> QString {
#if defined _WIN32
		return QString::fromStdWString(str.wstring());
#else
		return QString::fromStdString(str.string());
#endif
	}
}