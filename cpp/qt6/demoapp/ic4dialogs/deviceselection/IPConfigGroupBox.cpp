
#include "IPConfigGroupBox.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QLabel>
#include <QMessageBox>

namespace
{
	QCheckBox* addCheckBox(const ic4::PropertyMap& map, const char* prop_item, const char* label, QFormLayout& layout)
	{
		auto prop = map.findBoolean(prop_item, ic4::Error::Ignore());

		auto* check = new QCheckBox();
		check->setChecked(prop.getValue(ic4::Error::Ignore()));
		check->setEnabled(!prop.isReadOnly(ic4::Error::Ignore()));
		layout.addRow(QObject::tr(label), check);
		return check;
	}

	QLineEdit* addIPEdit(const ic4::PropertyMap& map, const char* propName, const std::string& defaultValue, const char* label, QFormLayout& layout)
	{
		ic4::Error err;
		auto value = map.getValueString(propName, err);
		if (!err.isSuccess())
		{
			value = defaultValue;
		}

		auto* edit = new QLineEdit(QString::fromStdString(value));
		edit->setEnabled(true);

		QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
		auto* ipValidator = new QRegularExpressionValidator(ipRegex, edit);
		edit->setValidator(ipValidator);

		layout.addRow(QObject::tr(label), edit);
		return edit;
	}
}

void IPConfigGroupBox::update(const ic4::DeviceInfo& deviceInfo)
{
	ic4::Error err;
	ic4::Grabber g;
	if (!g.deviceOpen(deviceInfo, err))
	{
		_layout->addRow(new QLabel("Error: Unable to access device to change IP Configuration settings"));
	}
	else
	{
		auto driverProperties = g.driverPropertyMap(err);
		if (err.isError())
		{
			_layout->addRow(new QLabel("Error: Unable to access device to change IP Configuration settings"));
		}
		else
		{
			_deviceInfo = deviceInfo;

			_chkPersistentIP = addCheckBox(driverProperties, "GevDeviceIPConfigPersistentIPEnable", "Enable Persistent IP", *_layout);
			_persistentIPAddress = addIPEdit(driverProperties, "GevDeviceIPConfigPersistentIPAddress", "0.0.0.0", "Persistent IP Address", *_layout);
			_persistentSubnetMask = addIPEdit(driverProperties, "GevDeviceIPConfigPersistentSubnetMask", "0.0.0.0", "Persistent Subnet Mask", *_layout);
			_persistentDefaultGateway = addIPEdit(driverProperties, "GevDeviceIPConfigPersistentGateway", "0.0.0.0", "Persistent Default Gateway", *_layout);

			_chkDHCP = addCheckBox(driverProperties, "GevDeviceIPConfigDHCPEnable", "Enable DHCP", *_layout);
			addCheckBox(driverProperties, "GevDeviceIPConfigLinkLocalAddressEnable", "Enable Link-Local Address", *_layout);

			connect(_chkPersistentIP, &QCheckBox::stateChanged, this, &IPConfigGroupBox::onStatusChangedCheckPersistentIP);
			onStatusChangedCheckPersistentIP(_chkPersistentIP->checkState());

			_applyButton = new QPushButton(tr("Apply Permanent IP Configuration"));
			_layout->addRow(_applyButton);

			connect(_applyButton, &QPushButton::pressed, this, &IPConfigGroupBox::onApplyButtonPressed);
		}
	}
}

void IPConfigGroupBox::onStatusChangedCheckPersistentIP(int state)
{
	bool persistentIPEnabled = (state == Qt::Checked);
	_persistentIPAddress->setEnabled(persistentIPEnabled);
	_persistentSubnetMask->setEnabled(persistentIPEnabled);
	_persistentDefaultGateway->setEnabled(persistentIPEnabled);
}

void IPConfigGroupBox::onApplyButtonPressed()
{
	ic4::Error err;
	ic4::Grabber grabber;
	if (!grabber.deviceOpen(_deviceInfo, err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to acquire device access:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	auto map = grabber.driverPropertyMap(err);
	if (err.isError())
	{
		QMessageBox::critical(this, {},
		QString("Failed to query driver property map:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!map.setValue("GevDeviceIPConfigPersistentIPEnable", _chkPersistentIP->checkState() == Qt::Checked, err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceIPConfigPersistentIPEnable:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!map.setValue("GevDeviceIPConfigDHCPEnable", _chkDHCP->checkState() == Qt::Checked, err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceIPConfigDHCPEnable:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!map.setValue("GevDeviceIPConfigPersistentIPAddress", _persistentIPAddress->text().toStdString(), err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceIPConfigPersistentIPAddress:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!map.setValue("GevDeviceIPConfigPersistentSubnetMask", _persistentSubnetMask->text().toStdString(), err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceIPConfigPersistentSubnetMask:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!map.setValue("GevDeviceIPConfigPersistentGateway", _persistentDefaultGateway->text().toStdString(), err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceIPConfigPersistentGateway:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!map.executeCommand("GevDeviceIPConfigApply", err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to execute GevDeviceIPConfigApply:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	_applyButton->setEnabled(false);
}

void IPConfigGroupBox::updateUnreachable(ic4::PropertyMap itfPropertyMap)
{
	_itfPropertyMap = itfPropertyMap;

	_forceIPAddress = addIPEdit(_itfPropertyMap, "GevDeviceForceIPAddress", "0.0.0.0", "Force IP Address", *_layout);
	_forceSubnetMask = addIPEdit(_itfPropertyMap, "GevDeviceForceSubnetMask", "0.0.0.0", "Force Subnet Mask", *_layout);
	_forceDefaultGateway = addIPEdit(_itfPropertyMap, "GevDeviceForceGateway", "0.0.0.0", "Force Default Gateway", *_layout);

	_forceButton = new QPushButton(tr("Force Temporary IP Configuration"));
	_layout->addRow(_forceButton);

	connect(_forceButton, &QPushButton::pressed, this, &IPConfigGroupBox::onForceButtonPressed);
}

void IPConfigGroupBox::onForceButtonPressed()
{
	ic4::Error err;

	if (!_itfPropertyMap.setValue("GevDeviceForceIPAddress", _forceIPAddress->text().toStdString(), err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceForceIPAddress:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!_itfPropertyMap.setValue("GevDeviceForceSubnetMask", _forceSubnetMask->text().toStdString(), err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceForceSubnetMask:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!_itfPropertyMap.setValue("GevDeviceForceGateway", _forceDefaultGateway->text().toStdString(), err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set GevDeviceForceGateway:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	if (!_itfPropertyMap.executeCommand("GevDeviceForceIP", err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to execute GevDeviceForceIP:\n%1")
				.arg(err.message().c_str())
		);
		return;
	}

	_forceButton->setEnabled(false);
}