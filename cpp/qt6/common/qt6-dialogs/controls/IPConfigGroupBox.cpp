
#include "IPConfigGroupBox.h"
#include "../ResourceSelector.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QLabel>
#include <QMessageBox>
#include <QStyle>

namespace
{
	QCheckBox* addCheckBox(const ic4::PropertyMap& map, const char* prop_item, const char* label, QFormLayout& layout)
	{
		auto prop = map.findBoolean(prop_item, ic4::Error::Ignore());

		auto* check = new QCheckBox();
		check->setChecked(prop.getValue(ic4::Error::Ignore()));
		check->setEnabled(!prop.isReadOnly(ic4::Error::Ignore()));
		check->setMinimumHeight(24);
		layout.addRow(QObject::tr(label), check);
		return check;
	}

	class IPV4Validator : public QValidator
	{
	public:
		using QValidator::QValidator;

	public:
		QValidator::State	validate(QString& input, int& pos) const override
		{
			static QRegularExpression regex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

			if (regex.match(input).hasMatch())
			{
				return QValidator::State::Acceptable;
			}

			return QValidator::State::Intermediate;
		}
	};

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

		auto* ipValidator = new IPV4Validator(edit);
		edit->setValidator(ipValidator);

		layout.addRow(QObject::tr(label), edit);

		QObject::connect(edit, &QLineEdit::textChanged,
			[edit]()
			{
				if (edit->hasAcceptableInput())
				{
					edit->setStyleSheet("background-color: palette(base)");
				}
				else
				{
					if (ResourceSelector::isDarkMode())
					{
						edit->setStyleSheet("background-color: darkred");
					}
					else
					{
						edit->setStyleSheet("background-color: #FF4040");
					}
				}
			}
		);

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

			auto updateApplyButtonEnabled = [this]()
				{
					bool allIPValid = _persistentIPAddress->hasAcceptableInput()
						&& _persistentSubnetMask->hasAcceptableInput()
						&& _persistentDefaultGateway->hasAcceptableInput();

					_applyButton->setEnabled(allIPValid || (_chkPersistentIP->checkState() != Qt::Checked));
				};

			connect(_persistentIPAddress, &QLineEdit::textChanged, updateApplyButtonEnabled);
			connect(_persistentSubnetMask, &QLineEdit::textChanged, updateApplyButtonEnabled);
			connect(_persistentDefaultGateway, &QLineEdit::textChanged, updateApplyButtonEnabled);
			connect(_chkPersistentIP, &QCheckBox::stateChanged, updateApplyButtonEnabled);
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

	if (_chkPersistentIP->checkState() == Qt::Checked)
	{
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

	auto* frame = new QFrame();
	frame->setObjectName("WarningFrame");
	frame->setStyleSheet("QFrame#WarningFrame { border: 1px solid red; background-color: palette(base); color: red; padding: 4px }");

	auto* iconLabel = new QLabel();
	auto icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	auto iconSize = icon.actualSize(QSize(32, 32));
	iconLabel->setPixmap(icon.pixmap(iconSize));

	auto* textLabel = new QLabel(tr("The device is currently not reachable by unicast messages. "
		"It has to be reconfigured to be in (one of) the subnet(s) of the network adapter."));
	textLabel->setWordWrap(true);
	//label->setStyleSheet("border: 1px solid red; background-color: palette(base); color: red; padding: 4px");


	auto* hbox = new QHBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(iconLabel, 0);
	hbox->addWidget(textLabel, 1);
	frame->setLayout(hbox);

	_layout->addRow(frame);

	addOptionalCommand(_layout, itfPropertyMap, "IPConfigAssignFreeTemporaryIP", "Auto-Assign Temporary Address");
	addOptionalCommand(_layout, itfPropertyMap, "IPConfigAssignFreePersistentIP", "Auto-Assign Persistent Address");
	addOptionalCommand(_layout, itfPropertyMap, "IPConfigDHCPEnable", "Enable DHCP");

	_forceButton = new QPushButton(tr("Force Temporary IP Configuration"));

	_forceIPAddress = addIPEdit(_itfPropertyMap, "GevDeviceForceIPAddress", "0.0.0.0", "Force IP Address", *_layout);
	_forceSubnetMask = addIPEdit(_itfPropertyMap, "GevDeviceForceSubnetMask", "0.0.0.0", "Force Subnet Mask", *_layout);
	_forceDefaultGateway = addIPEdit(_itfPropertyMap, "GevDeviceForceGateway", "0.0.0.0", "Force Default Gateway", *_layout);

	_forceButton = new QPushButton(tr("Force Temporary IP Configuration"));
	_layout->addRow(_forceButton);

	connect(_forceButton, &QPushButton::pressed, this, &IPConfigGroupBox::onForceButtonPressed);

	auto updateForceButtonEnabled = [this]()
		{
			bool allIPValid = _forceIPAddress->hasAcceptableInput()
				&& _forceSubnetMask->hasAcceptableInput()
				&& _forceDefaultGateway->hasAcceptableInput();
			_forceButton->setEnabled(allIPValid);
		};

	connect(_forceIPAddress, &QLineEdit::textChanged, updateForceButtonEnabled);
	connect(_forceSubnetMask, &QLineEdit::textChanged, updateForceButtonEnabled);
	connect(_forceDefaultGateway, &QLineEdit::textChanged, updateForceButtonEnabled);
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

void IPConfigGroupBox::addOptionalCommand(QFormLayout* layout, const ic4::PropertyMap& itfPropertyMap, const char* cmdName, const QString& label)
{
	ic4::Error err;
	auto cmd = itfPropertyMap.findCommand(cmdName, err);
	if (err.isError())
		return;

	auto* cmdButton = new QPushButton(label);
	_layout->addRow(cmdButton);

	connect(cmdButton, &QPushButton::pressed,
		[cmd]() mutable
		{
			cmd.execute();
		}
	);
}