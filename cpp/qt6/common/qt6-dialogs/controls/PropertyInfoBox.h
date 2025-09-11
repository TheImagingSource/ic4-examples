
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
        // without this an empty info box would be writeable
        // and could trap user cursor when using tab for navigation
        setReadOnly(true);
	}

	void clear()
	{
		setHtml("");
	}

	void update(ic4::Property prop)
	{
		try
		{
			auto name = prop.name();
			auto desc = prop.description();

			ic4::Error err;

			QString text;
			text += QString("<p style='margin-bottom:0px'><b>%1</b></p>").arg(QString::fromStdString(name));
			if (!desc.empty())
				text += QString("<p style='margin-top:0px;margin-bottom:5px'>%1</p>").arg(QString::fromStdString(desc));

			text += QString("<p style='margin-top:0px'>");
			bool is_locked = prop.isLocked();
			bool is_readonly = prop.isReadOnly();

			if (is_readonly)
			{
				text += QString("Access: Read-Only<br/>");
			}
			else if (is_locked)
			{
				text += QString("Access: Readable, Locked<br/>");
			}
			else
			{
				text += QString("Access: Readable, Writable<br/>");
			}

			switch (prop.type())
			{
			case ic4::PropType::Category:
				text += QString("Type: Category<br/>");
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
			default:
				break;
			}

			text += QString("</p>");
			setHtml(text);
		}
		catch (ic4::IC4Exception& ex)
		{
			setText(ex.what());
		}

		// disable selection
		setTextInteractionFlags(Qt::NoTextInteraction);
		setReadOnly(true);
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
				QTextCursor textCursor(currentBlock);
				blockFormat = currentBlock.blockFormat();
				blockFormat.setLineHeight(120, QTextBlockFormat::ProportionalHeight);
				textCursor.setBlockFormat(blockFormat);
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
		auto text = QString("Type: String<br/>");

		auto stringProp = prop.asString();
		try
		{
			auto val = stringProp.getValue();
			auto str = QString::fromStdString(val);
			str.replace("@", "<span>@</span>");

			text += QString("Value: %1<br/>").arg(str);
		}
		catch (const ic4::IC4Exception& iex)
		{
			text += QString("Value: <span style='color:red'>%1</span><br/>").arg(iex.what());
		}

		if (!stringProp.isReadOnly())
		{
			try
			{
				text += QString("Maximum Length: %1<br/>").arg(stringProp.maxLength());
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
		auto text = QString("Type: Integer<br/>");

		auto rep = prop.representation();
		auto unit = prop.unit(ic4::Error::Ignore());
		if (!unit.empty())
		{
			text += QString("Unit: %1<br/>").arg(unit.c_str());
		}

		try
		{
			auto val = prop.getValue();
			text += QString("Value: %1<br/>").arg(ic4::ui::PropIntControl::value_to_string(val, rep));
		}
		catch (const ic4::IC4Exception &iex)
		{
			text += QString("Value: <span style='color:red'>%1</span><br/>").arg(iex.what());
		}

		if (!prop.isReadOnly())
		{
			ic4::Error err;

			auto minimum = prop.minimum(err);
			if (err.isSuccess())
			{
				text += QString("Minimum: %1<br/>").arg(minimum);
			}

			auto maximum = prop.maximum(err);
			if (err.isSuccess())
			{
				text += QString("Maximum: %1<br/>").arg(maximum);
			}

			switch (prop.incrementMode(ic4::Error::Ignore()))
			{
			case ic4::PropIncrementMode::Increment:
			{
				auto increment = prop.increment(err);
				if (err.isSuccess())
				{
					text += QString("Increment: %1<br/>").arg(increment);
				}
				else
				{
					text += QString("Increment: <span style='color:red'>%1</span><br/>").arg(err.message().c_str());
				}
				break;
			}
			case ic4::PropIncrementMode::ValueSet:
			{
				auto valueSet = prop.validValueSet(err);
				if (err.isSuccess())
				{
					QStringList lst;
					for (auto&& v : valueSet)
					{
						lst.push_back(QString::number(v));
					}

					text += QString("Valid Value Set: %1<br/>").arg(lst.join(", "));
				}
				else
				{
					text += QString("Valid Value Set: <span style='color:red'>%1</span><br/>").arg(err.message().c_str());
				}
				break;
			}
			case ic4::PropIncrementMode::None:
				break;
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
		auto text = QString("Type: Float<br/>");

		auto notation = prop.displayNotation(ic4::Error::Ignore());
		auto precision = prop.displayPrecision(ic4::Error::Ignore());
		auto unit = prop.unit(ic4::Error::Ignore());
		if (!unit.empty())
		{
			text += QString("Unit: %1<br/>").arg(unit.c_str()) ;
		}

		try
		{
			auto val = prop.getValue();
			text += QString("Value: %1<br/>").arg(ic4::ui::PropFloatControl::textFromValue(val, notation, precision, locale()));
		}
		catch (const ic4::IC4Exception& iex)
		{
			text += QString("Value: <span style='color:red'>%1</span><br/>").arg(iex.what());
		}

		if (!prop.isReadOnly())
		{
			ic4::Error err;

			auto minimum = prop.minimum(err);
			if (err.isSuccess())
			{
				text += QString("Minimum: %1<br/>").arg(ic4::ui::PropFloatControl::textFromValue(minimum, notation, precision, locale()));
			}

			auto maximum = prop.maximum(err);
			if (err.isSuccess())
			{
				text += QString("Maximum: %1<br/>").arg(ic4::ui::PropFloatControl::textFromValue(maximum, notation, precision, locale()));
			}

			switch (prop.incrementMode(ic4::Error::Ignore()))
			{
			case ic4::PropIncrementMode::Increment:
			{
				auto increment = prop.increment(err);
				if (err.isSuccess())
				{
					text += QString("Increment: %1<br/>").arg(ic4::ui::PropFloatControl::textFromValue(increment, notation, precision, locale()));
				}
				else
				{
					text += QString("Increment: <span style='color:red'>%1</span><br/>").arg(err.message().c_str());
				}
				break;
			}
			case ic4::PropIncrementMode::ValueSet:
			{
				auto valueSet = prop.validValueSet(err);
				if (err.isSuccess())
				{
					QStringList lst;
					for (auto&& v : valueSet)
					{
						lst.push_back(ic4::ui::PropFloatControl::textFromValue(v, notation, precision, locale()));
					}

					text += QString("Valid Value Set: %1<br/>").arg(lst.join(", "));
				}
				else
				{
					text += QString("Valid Value Set: <span style='color:red'>%1</span><br/>").arg(err.message().c_str());
				}
				break;
			}
			case ic4::PropIncrementMode::None:
				break;
			}
		}

		return text;
	}

	QString showEnumerationInfo(ic4::PropEnumeration prop)
	{
		auto text = QString("Type: Enumeration<br/>");

		ic4::Error err;
		auto val = prop.getValue(err);
		if (err.isSuccess())
		{
			text += QString("Value: %1<br/>").arg(val.c_str());
		}
		else
		{
			text += QString("Value: <span style='color:red'>%1</span><br/>").arg(err.message().c_str());
		}

		text += "Possible Values: ";
		auto entries = prop.entries(err);
		if (err.isSuccess())
		{
			bool first = true;
			bool any_unavailable = false;
			for (auto&& entry : entries)
			{
				if (!entry.isAvailable(ic4::Error::Ignore()))
				{
					any_unavailable = true;
					continue;
				}

				if (!first)
					text += ", ";
				else
					first = false;
				text += QString::fromStdString(entry.name(ic4::Error::Ignore()));
			}
			text += "<br/>";

			if (any_unavailable)
			{
				text += "Unavailable Values: ";
				first = true;
				for (auto&& entry : entries)
				{
					if (entry.isAvailable(ic4::Error::Ignore()))
						continue;

					if (!first)
						text += ", ";
					else
						first = false;
					text += QString::fromStdString(entry.name(ic4::Error::Ignore()));
				}
				text += "<br/>";
			}
		}
		else
		{
			text += QString("<span style='color:red'>%1</span><br/>").arg(err.message().c_str());
		}

		return text;
	}

	QString showBooleanInfo(ic4::PropBoolean prop)
	{
		auto text = QString("Type: Boolean<br/>");

		ic4::Error err;
		auto val = prop.getValue(err);
		if (err.isSuccess())
		{
			text += QString("Value: %1<br/>").arg(val ? "True" : "False");
		}
		else
		{
			text += QString("Value: <span style='color:red'>%1</span><br/>").arg(err.message().c_str());
		}

		return text;
	}
};
