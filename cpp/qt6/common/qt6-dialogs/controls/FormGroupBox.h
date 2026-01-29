
#pragma once

#include <QFrame>
#include <QString>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

#include "PropertyControls.h"


static void clearLayout(QLayout* layout, bool delete_widgets)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (delete_widgets)
        {
            if (QWidget* widget = item->widget())
			{
                widget->deleteLater();
			}
        }
        if (QLayout* childLayout = item->layout())
		{
            clearLayout(childLayout, delete_widgets);
		}
        delete item;
    }
}

class FormGroupBox : public QFrame
{
	Q_OBJECT

public:
	explicit FormGroupBox(const QString& title)
	{
		auto vbox = new QVBoxLayout();
		vbox->setContentsMargins(0, 0, 0, 8);

		_layout = new QFormLayout();
		_layout->setContentsMargins(7, 0, 7, 0);
		_layout->setLabelAlignment(Qt::AlignRight);

		auto label = new QLabel(title);
		label->setStyleSheet(ic4::ui::CustomStyle.FormGroupBoxStyle);

		vbox->addWidget(label);
		vbox->addLayout(_layout);

		setLayout(vbox);
	}

public:
	QFormLayout* formLayout() const
	{
		return _layout;
	}

public:
	void clear()
	{
		clearLayout(_layout, true);
		clearInternal();
	}

protected:
	QFormLayout* _layout;

	virtual void clearInternal()
	{
	};
};

class ReturnFocusNextLineEdit : public QLineEdit
{
public:
	using QLineEdit::QLineEdit;
protected:
	void keyPressEvent(QKeyEvent* event) override
	{
		if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		{
			if (hasAcceptableInput())
			{
				focusNextChild();
			}
			return;
		}
		QLineEdit::keyPressEvent(event);
	}
};