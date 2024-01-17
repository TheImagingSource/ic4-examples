
#include "SwitchDriverGroupBox.h"
#include "../ResourceSelector.h"

#include <QStyle>
#include <QMessageBox>

void SwitchDriverGroupBox::update(const ic4::DeviceInfo& deviceInfo)
{
	_deviceInfo = deviceInfo;
	_itfPropertyMap = deviceInfo.getInterface(ic4::Error::Ignore()).interfacePropertyMap(ic4::Error::Ignore());

	auto* frame = new QFrame();
	frame->setObjectName("WarningFrame");
	frame->setStyleSheet("QFrame#WarningFrame { border: 1px solid red; background-color: palette(base); color: red; padding: 4px }");

	auto* iconLabel = new QLabel();
	auto icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
	auto iconSize = icon.actualSize(QSize(32, 32));
	iconLabel->setPixmap(icon.pixmap(iconSize));

	auto* textLabel = new QLabel(tr("The device is currently not usable, because an incompatible kernel driver is installed for its USB3 Vision interface. "
		"The correct driver has to be enabled to operate the device."));
	textLabel->setWordWrap(true);

	auto* hbox = new QHBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(iconLabel, 0);
	hbox->addWidget(textLabel, 1);
	frame->setLayout(hbox);

	_layout->addRow(frame);

	_switchButton = new QPushButton(tr("Switch Driver"));
	_layout->addRow(_switchButton);

	connect(_switchButton, &QPushButton::pressed, this, &SwitchDriverGroupBox::onSwitchButtonPressed);
}

void SwitchDriverGroupBox::onSwitchButtonPressed()
{
	ic4::Error err;
	if (!_itfPropertyMap.executeCommand("DeviceInstallCompatibleDriver", err))
	{
		QMessageBox::critical(this, {},
			QString("Failed to set install compatible driver:\n%1")
				.arg(err.message().c_str())
		);
	}
}