
#pragma once

#include "FormGroupBox.h"

#include <ic4/ic4.h>

#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>

class SwitchDriverGroupBox : public FormGroupBox
{
	Q_OBJECT

public:
	SwitchDriverGroupBox(const QString& title)
		: FormGroupBox(title)
	{
	}

public:
	void update(const ic4::DeviceInfo& deviceInfo);

private:
	ic4::DeviceInfo _deviceInfo;
	ic4::PropertyMap _itfPropertyMap;

	QPushButton* _switchButton = nullptr;

private:
	void onSwitchButtonPressed();
};