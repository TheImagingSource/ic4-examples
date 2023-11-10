
#pragma once

#include <QFrame>
#include <QString>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

class FormGroupBox : public QFrame
{
	Q_OBJECT

public:
	FormGroupBox(const QString& title)
	{
		auto vbox = new QVBoxLayout();
		vbox->setContentsMargins(0, 0, 0, 8);

		_layout = new QFormLayout();
		_layout->setContentsMargins(7, 0, 7, 0);
		_layout->setLabelAlignment(Qt::AlignRight);

		auto label = new QLabel(title);
		label->setStyleSheet("QLabel { margin: 0px; background-color: palette(base); padding: 4px }");

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
		while (_layout->count() != 0)
		{
			QLayoutItem* forDeletion = _layout->takeAt(0);
			delete forDeletion->widget();
			delete forDeletion;
		}
	}

protected:
	QFormLayout* _layout;
};