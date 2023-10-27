

#include <QFileSelector>
#include <QStringList>
#include <QPalette>

class DemoAppFileSelector
{
private:
	static bool isDarkMode()
	{
		// Compare WindowText vs Window color lightness to detect dark mode
		const QPalette defaultPalette;
		return defaultPalette.color(QPalette::WindowText).lightness()
			 > defaultPalette.color(QPalette::Window).lightness();
	}

	static const QFileSelector& selector()
	{
		static QFileSelector selector;

		static bool initialized = false;
		if (!initialized)
		{
			QStringList extraSelectors({ isDarkMode() ? "theme_dark" : "theme_light" });
			selector.setExtraSelectors(extraSelectors);
			initialized = true;
		}

		return selector;
	}

public:
	static QString select(QString item)
	{
		return selector().select(item);
	}
};