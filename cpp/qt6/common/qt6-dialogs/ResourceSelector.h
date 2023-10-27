

#include <QFileSelector>
#include <QStringList>
#include <QPalette>
#include <QIcon>

class ResourceSelector
{
private:
	QFileSelector fileSelector;

	static bool isDarkMode()
	{
		// Compare WindowText vs Window color lightness to detect dark mode
		const QPalette defaultPalette;
		return defaultPalette.color(QPalette::WindowText).lightness()
			 > defaultPalette.color(QPalette::Window).lightness();
	}

public:
	ResourceSelector()
	{
		QStringList extraSelectors({ isDarkMode() ? "theme_dark" : "theme_light" });
		fileSelector.setExtraSelectors(extraSelectors);
	}

public:
	QString select(const QString& item) const
	{
		return fileSelector.select(item);
	}

	QIcon loadIcon(const QString& item) const
	{
		return QIcon(select(item));
	}

public:
	static const ResourceSelector& instance()
	{
		static ResourceSelector selector;
		return selector;
	}
};
