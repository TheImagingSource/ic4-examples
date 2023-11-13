
#pragma once

#include "props/PropIntControl.h"
#include "props/PropFloatControl.h"

#include <ic4/ic4.h>

#include <QTextEdit>
#include <QTextBlock>

class PropertyInfoBox : public QTextEdit
{
	Q_OBJECT

public:
	PropertyInfoBox(QWidget* parent)
		: QTextEdit(parent)
	{
	}

	void clear()
	{
		setMarkdown("");
	}

	void update(ic4::Property prop)
	{
		auto name = prop.name();
		auto desc = prop.description();

		ic4::Error err;

		QString text;
		text += QString("**%1**\n\n").arg(QString::fromStdString(name));
		if (!desc.empty())
			text += QString("%1\n\n").arg(QString::fromStdString(desc));
		text += QString("\n\n");

		bool is_locked = prop.isLocked();
		bool is_readonly = prop.isReadOnly();

		if (is_readonly)
		{
			text += QString("Access: Read-Only\n\n");
		}
		else if (is_locked)
		{
			text += QString("Access: Readable, Locked\n\n");
		}
		else
		{
			text += QString("Access: Readable, Writable\n\n");
		}

		switch (prop.type())
		{
			case ic4::PropType::Category:
				text += QString("Type: Category\n\n");
				break;
			case ic4::PropType::Integer:
				text += showIntegerInfo(prop.asInteger());
				break;
			case ic4::PropType::Float:
				text += showFloatInfo(prop.asFloat());
				break;
			case ic4::PropType::String:
				text += showStringInfo(prop.asString());
				break;
			case ic4::PropType::Enumeration:
				text += showEnumerationInfo(prop.asEnumeration());
				break;
			case ic4::PropType::Boolean:
				text += showBooleanInfo(prop.asBoolean());
				break;
		}

		// disable selection
		setTextInteractionFlags(Qt::NoTextInteraction);
		setReadOnly(true);
		setMarkdown(text);
		setContentsMargins(8, 8, 8, 8);
		setStyleSheet("QTextEdit {"
			"font-size: 13px;"
			"}");

		// line spacing
		auto doc = this->document();
		auto currentBlock = doc->firstBlock();
		if (currentBlock.isValid())
		{
			QTextCursor cursor(currentBlock);
			QTextBlockFormat blockFormat = currentBlock.blockFormat();
			blockFormat.setLineHeight(160, QTextBlockFormat::ProportionalHeight);
			cursor.setBlockFormat(blockFormat);
			currentBlock = currentBlock.next();

			while (currentBlock.isValid())
			{
				QTextCursor cursor(currentBlock);
				QTextBlockFormat blockFormat = currentBlock.blockFormat();
				blockFormat.setLineHeight(120, QTextBlockFormat::ProportionalHeight);
				cursor.setBlockFormat(blockFormat);
				currentBlock = currentBlock.next();
			}
		}
	}

	/// <summary>
	/// Show information about a string property
	/// </summary>
	/// <param name="prop"></param>
	/// <returns></returns>
	QString showStringInfo(ic4::Property prop)
	{
		auto text = QString("Type: String\n\n");

		auto stringProp = prop.asString();
		try
		{
			auto val = stringProp.getValue();
			auto str = QString::fromStdString(val);
			str.replace("@", "<span>@</span>");

			text += QString("Value: %1\n\n").arg(str);
		}
		catch (const ic4::IC4Exception& iex)
		{
			text += QString("Value: <span style='color:red'>%1</span>\n\n").arg(iex.what());
		}

		if (!stringProp.isReadOnly())
		{
			try
			{
				text += QString("Maximum Length: %1\n\n").arg(stringProp.maxLength());
			}
			catch (...) {}
		}

		return text;
	}
	/// <summary>
	/// Show information about an integer property.
	/// </summary>
	/// <param name="prop"></param>
	/// <returns></returns>
	QString showIntegerInfo(ic4::PropInteger prop)
	{
		auto text = QString("Type: Integer\n\n");

		auto rep = prop.representation();
		auto unit = prop.unit(ic4::Error::Ignore());
		if (!unit.empty())
		{
			text += QString("Unit: %1\n\n").arg(unit.c_str());
		}

		try
		{
			auto val = prop.getValue();
			text += QString("Value: %1\n\n").arg(ic4::ui::PropIntControl::value_to_string(val, rep));
		}
		catch (const ic4::IC4Exception &iex)
		{
			text += QString("Value: <span style='color:red'>%1</span>\n\n").arg(iex.what());
		}

		if (!prop.isReadOnly())
		{
			ic4::Error err;

			auto minimum = prop.minimum(err);
			if (err.isSuccess())
			{
				text += QString("Minimum: %1\n\n").arg(minimum);
			}

			auto maximum = prop.maximum(err);
			if (err.isSuccess())
			{
				text += QString("Maximum: %1\n\n").arg(maximum);
			}

			auto increment = prop.increment(err);
			if (err.isSuccess())
			{
				text += QString("Increment: %1\n\n").arg(increment);
			}
		}

		return text;
	}

	/// <summary>
	/// Show information about a float property.
	/// </summary>
	/// <param name="prop"></param>
	/// <returns></returns>
	QString showFloatInfo(ic4::PropFloat prop)
	{
		auto text = QString("Type: Float\n\n");

		auto rep = prop.representation(ic4::Error::Ignore());
		auto notation = prop.displayNotation(ic4::Error::Ignore());
		auto precision = prop.displayPrecision(ic4::Error::Ignore());
		auto unit = prop.unit(ic4::Error::Ignore());
		if (!unit.empty())
		{
			text += QString("Unit: %1\n\n").arg(unit.c_str()) ;
		}

		try
		{
			auto val = prop.getValue();
			text += QString("Value: %1\n\n").arg(ic4::ui::PropFloatControl::textFromValue(val, notation, precision, locale()));
		}
		catch (const ic4::IC4Exception& iex)
		{
			text += QString("Value: <span style='color:red'>%1</span>\n\n").arg(iex.what());
		}

		if (!prop.isReadOnly())
		{
			ic4::Error err;

			auto minimum = prop.minimum(err);
			if (err.isSuccess())
			{
				text += QString("Minimum: %1\n\n").arg(minimum);
			}

			auto maximum = prop.maximum(err);
			if (err.isSuccess())
			{
				text += QString("Maximum: %1\n\n").arg(maximum);
			}

			auto increment = prop.increment(err);
			if (err.isSuccess())
			{
				text += QString("Increment: %1\n\n").arg(increment);
			}
		}

		return text;
	}

	QString showEnumerationInfo(ic4::PropEnumeration prop)
	{
		auto text = QString("Type: Enumeration\n\n");

		ic4::Error err;
		auto val = prop.getValue(err);
		if (err.isSuccess())
		{
			text += QString("Value: %1\n\n").arg(val.c_str());
		}
		else
		{
			text += QString("Value: <span style='color:red'>%1</span>\n\n").arg(err.message().c_str());
		}

		text += "Possible Values: ";
		auto entries = prop.entries(err);
		if (err.isSuccess())
		{
			bool first = true;
			for (auto&& entry : entries)
			{
				if (!first)
					text += ", ";
				else
					first = false;
				text += entry.name(ic4::Error::Ignore());
			}
			text += "\n\n";
		}
		else
		{
			text += QString("<span style='color:red'>%1</span>\n\n").arg(err.message().c_str());
		}

		return text;
	}

	QString showBooleanInfo(ic4::PropBoolean prop)
	{
		auto text = QString("Type: Boolean\n\n");

		ic4::Error err;
		auto val = prop.getValue(err);
		if (err.isSuccess())
		{
			text += QString("Value: %1\n\n").arg(val ? "True" : "False");
		}
		else
		{
			text += QString("Value: <span style='color:red'>%1</span>\n\n").arg(err.message().c_str());
		}

		return text;
	}
};