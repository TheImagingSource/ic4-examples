
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
	bool update(const ic4::DeviceInfo& deviceInfo);
	void updateUnreachable(ic4::PropertyMap itfPropertyMap);

	void setEnable(bool);

private:
	ic4::DeviceInfo _deviceInfo;
	ic4::PropertyMap _itfPropertyMap;

	QCheckBox* _chkPersistentIP = nullptr;
	QCheckBox* _chkDHCP = nullptr;
	QLineEdit* _persistentIPAddress = nullptr;
	QLineEdit* _persistentSubnetMask = nullptr;
	QLineEdit* _persistentDefaultGateway = nullptr;
	QPushButton* _applyButton = nullptr;

	QLineEdit* _forceIPAddress = nullptr;
	QLineEdit* _forceSubnetMask = nullptr;
	QLineEdit* _forceDefaultGateway = nullptr;
	QPushButton* _forceButton = nullptr;

private:
	void onStatusChangedCheckPersistentIP(int checkState);
	void onApplyButtonPressed();

	void onForceButtonPressed();

	void addOptionalCommand(QFormLayout* layout, const ic4::PropertyMap& itfPropertyMap, const char* cmdName, const QString& label);

	void clearInternal() override;
};
