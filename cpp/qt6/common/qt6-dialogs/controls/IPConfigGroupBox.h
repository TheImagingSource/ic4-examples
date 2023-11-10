
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

class IPConfigGroupBox : public FormGroupBox
{
	Q_OBJECT

public:
	IPConfigGroupBox(const QString& title)
		: FormGroupBox(title)
	{
	}

public:
	void update(const ic4::DeviceInfo& deviceInfo);
	void updateUnreachable(ic4::PropertyMap itfPropertyMap);

private:
	ic4::DeviceInfo _deviceInfo;
	ic4::PropertyMap _itfPropertyMap;

	QCheckBox* _chkPersistentIP;
	QCheckBox* _chkDHCP;
	QLineEdit* _persistentIPAddress;
	QLineEdit* _persistentSubnetMask;
	QLineEdit* _persistentDefaultGateway;
	QPushButton* _applyButton;

	QLineEdit* _forceIPAddress;
	QLineEdit* _forceSubnetMask;
	QLineEdit* _forceDefaultGateway;
	QPushButton* _forceButton;

private:
	void onStatusChangedCheckPersistentIP(int checkState);
	void onApplyButtonPressed();

	void onForceButtonPressed();
};