#pragma once

#include "deviceselection.h"
#include "PropertyControls.h"

#include <iostream>
#include <QMessagebox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <iostream>

const QEvent::Type EVENT_DEVICE_LIST_CHANGED = static_cast<QEvent::Type>(QEvent::User + 3);

DeviceSelectionDlg::DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber) : QDialog(parent)
{
	_pgrabber = pgrabber;

	createUI();
	enumerateDevices();

	_enumerator.eventAddDeviceListChanged(
		[this](auto&)
		{
			QApplication::postEvent(this, new QEvent(EVENT_DEVICE_LIST_CHANGED));
		}
	);
}

void DeviceSelectionDlg::customEvent(QEvent* event)
{
	if (event->type() == EVENT_DEVICE_LIST_CHANGED)
	{
		OnUpdateButton();
	}
}


struct DeviceInfoContainer
{
	ic4::DeviceInfo info;
	int64_t index = 0;
};
Q_DECLARE_METATYPE(DeviceInfoContainer)

struct InterfaceContainer
{
	ic4::Interface itf;
};
Q_DECLARE_METATYPE(InterfaceContainer)

void DeviceSelectionDlg::createUI()
{
	this->setWindowTitle("Device Selection");
	setMinimumSize(900, 600);

	auto topLayout = new QHBoxLayout();
	auto leftLayout = new QVBoxLayout();

	_cameraTree = new QTreeWidget();
	_cameraTree->setIconSize(QSize(32, 32));
	_cameraTree->setIndentation(16);
	//_cameraTree->setStyleSheet("QTreeWidget { border: 1px solid #3f3f46; max-height: 1024; padding: 4px 4px 4px 4px; } QTreeView::item  {  padding: 4px 4px 4px 4px; } ");
	_cameraTree->setStyleSheet("QTreeView::item { padding: 4px; } ");
	_cameraTree->setRootIsDecorated(false);
	_cameraTree->setMinimumWidth(450);
	_cameraTree->setItemsExpandable(false);
	QStringList headerLabels = { tr("Device"), tr("Serial Number"), tr("IP Address"), tr("Device User ID") };
	_cameraTree->setColumnCount(headerLabels.count());
	_cameraTree->setHeaderLabels(headerLabels);
	_cameraTree->setColumnWidth(0, 140);
	_cameraTree->setHeaderHidden(false);

	connect(_cameraTree, &QTreeWidget::currentItemChanged, this, &DeviceSelectionDlg::onCurrentItemChanged);
	connect(_cameraTree, &QTreeWidget::itemDoubleClicked, [&](QTreeWidgetItem* item, int column) { OnOK();	});

	leftLayout->addWidget(_cameraTree);

	QHBoxLayout* buttons = new QHBoxLayout();

	//////////////////////////////////////////////////////////
	auto UpdateButton = new QPushButton(tr("Update"));
	connect(UpdateButton, &QPushButton::pressed, this, &DeviceSelectionDlg::OnUpdateButton);
	buttons->addWidget(UpdateButton);

	auto cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, &QPushButton::pressed, this, &QDialog::reject);
	buttons->addWidget(cancelButton);

	_OKButton = new QPushButton(tr("OK"));
	_OKButton->setDefault(true);
	connect(_OKButton, &QPushButton::pressed, this, &DeviceSelectionDlg::OnOK);
	buttons->addWidget(_OKButton);

	leftLayout->addLayout(buttons);
	topLayout->addLayout(leftLayout, 1);

	auto rightLayout = new QVBoxLayout();

	ic4::ui::PropertyTreeWidget::Settings settings = {};
	settings.showRootItem = false;
	settings.showFilter = false;
	settings.showInfoBox = false;
	settings.initialVisibility = ic4::PropVisibility::Guru;

	_propTree = new ic4::ui::PropertyTreeWidget(ic4::PropCategory(), nullptr, settings, this);
	rightLayout->addWidget(_propTree);

	topLayout->addLayout(rightLayout, 2);

	setLayout(topLayout);
}

void DeviceSelectionDlg::enumerateDevices()
{
	//QSignalBlocker blk(_cameraTree);
	_cameraTree->clear();

	for (auto&& itf : ic4::DeviceEnum::enumInterfaces())
	{
		auto itf_devices = itf.enumDevices();
		if (itf_devices.empty())
			continue;

		auto* itf_item = new QTreeWidgetItem(_cameraTree);
		itf_item->setText(0, QString::fromStdString(itf.interfaceDisplayName()));
		itf_item->setForeground(0, QPalette().windowText());
		itf_item->setData(0, Qt::UserRole + 1, QVariant::fromValue(InterfaceContainer{ itf }));
		itf_item->setFirstColumnSpanned(true);

		auto map = itf.interfacePropertyMap(ic4::Error::Ignore());

		int index = 0;
		for (auto&& dev : itf_devices)
		{
			QString strIPAddress;

			ic4::Error err;
			if (map.setValue("DeviceSelector", index, err))
			{
				auto ip = map.getValueInt64("GevDeviceIPAddress", err);
				if (err.isSuccess())
				{
					strIPAddress = QString("%1.%2.%3.%4")
						.arg((ip >> 24) & 0xFF)
						.arg((ip >> 16) & 0xFF)
						.arg((ip >> 8) & 0xFF)
						.arg((ip >> 0) & 0xFF);
				}
			}

			auto device = dev.modelName(ic4::Error::Ignore());
			auto serial = dev.serial(ic4::Error::Ignore());
			auto deviceUserID = dev.userID(ic4::Error::Ignore());

			auto* node = new QTreeWidgetItem();

			auto variant = QVariant::fromValue(DeviceInfoContainer{ dev, index });
			node->setData(0, Qt::UserRole + 1, variant);

			node->setText(0, QString::fromStdString(device));
			node->setText(1, QString::fromStdString(serial));
			node->setText(2, strIPAddress);
			node->setText(3, QString::fromStdString(deviceUserID));

			itf_item->addChild(node);

			index += 1;
		}
	}

	_cameraTree->expandAll();
}

void DeviceSelectionDlg::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	_OKButton->setEnabled(false);

	if (current == nullptr)
	{
		_propTree->updateModel(ic4::PropCategory());
		return;
	}

	auto variant = current->data(0, Qt::UserRole + 1);

	auto prop_filter_interface = [](const ic4::Property& prop) -> bool
		{
			auto name = prop.getName(ic4::Error::Ignore());
			if (name == "InterfaceDisplayName")
				return true;
			if (name == "GevInterfaceSubnetSelector")
				return true;
			if (name == "GevInterfaceSubnetIPAddress")
				return true;
			if (name == "GevInterfaceSubnetMask")
				return true;
			if (name == "MaximumTransmissionUnit")
				return true;
			return false;
		};

	auto prop_filter_device = [&prop_filter_interface](const ic4::Property& prop) -> bool
	{
		auto name = prop.getName(ic4::Error::Ignore());
		if (name == "InterfaceDisplayName")
			return true;
		if (name == "MaximumTransmissionUnit")
			return true;
		if (name == "DeviceVendorName")
			return true;
		if (name == "DeviceSerialNumber")
			return true;
		if (name == "DeviceManufacturer")
			return true;
		if (name == "DeviceVersion")
			return true;
		if (name == "DeviceUserID")
			return true;
		if (name == "GevDeviceMACAddress")
			return true;
		if (name == "GevDeviceIPAddress")
			return true;
		if (name == "GevDeviceSubnetMask")
			return true;
		if (name == "GevDeviceGateway")
			return true;
		return prop_filter_interface(prop);
	};

	if (variant.canConvert<DeviceInfoContainer>())
	{
		auto selectedDeviceInfo = variant.value<DeviceInfoContainer>();

		auto map = selectedDeviceInfo.info.getInterface().interfacePropertyMap(ic4::Error::Ignore());
		if (map.setValue("DeviceSelector", selectedDeviceInfo.index, ic4::Error::Ignore()))
		{
			_OKButton->setEnabled(true);

			auto root = map.findCategory("Root", ic4::Error::Ignore());

			_propTree->updateModel(root);
			_propTree->setPropertyFilter(prop_filter_device);
			return;
		}
	}
	else if (variant.canConvert<InterfaceContainer>())
	{
		auto selectedInterfaceInfo = variant.value<InterfaceContainer>();

		auto map = selectedInterfaceInfo.itf.interfacePropertyMap(ic4::Error::Ignore());
		auto root = map.findCategory("Root", ic4::Error::Ignore());

		_propTree->updateModel(root);
		_propTree->setPropertyFilter(prop_filter_interface);
		return;
	}
	
	_propTree->updateModel(ic4::PropCategory());
}

void DeviceSelectionDlg::selectPreviousItem(QVariant itemData)
{
	for (int i = 0; i < _cameraTree->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* itf_item = _cameraTree->topLevelItem(i);

		if (itemData.canConvert<InterfaceContainer>())
		{
			auto itf_variant = itf_item->data(0, Qt::UserRole + 1);

			if (itf_variant.canConvert<InterfaceContainer>())
			{
				if (itf_variant.value<InterfaceContainer>().itf == itemData.value<InterfaceContainer>().itf)
				{
					_cameraTree->setCurrentItem(itf_item);
					return;
				}
			}
		}
		else if (itemData.canConvert<DeviceInfoContainer>())
		{
			for (int j = 0; j < itf_item->childCount(); ++j)
			{
				QTreeWidgetItem* dev_item = itf_item->child(j);

				auto dev_variant = dev_item->data(0, Qt::UserRole + 1);

				if (dev_variant.canConvert<DeviceInfoContainer>())
				{
					if (dev_variant.value<DeviceInfoContainer>().info == itemData.value<DeviceInfoContainer>().info)
					{
						_cameraTree->setCurrentItem(dev_item);
						return;
					}
				}
			}
		}
	}
}

void DeviceSelectionDlg::OnUpdateButton()
{
	QVariant previousData;

	auto* item = _cameraTree->currentItem();
	if (item != nullptr)
	{
		previousData = item->data(0, Qt::UserRole + 1);
	}

	enumerateDevices();
	if (previousData.isValid())
	{
		selectPreviousItem(previousData);
	}
	else
	{
		_cameraTree->setCurrentItem(nullptr);
	}
	//_cameraTree->setFocus();
}

void DeviceSelectionDlg::OnOK()
{
	auto* item = _cameraTree->currentItem();
	if (item == nullptr)
		return;

	auto variant = item->data(0, Qt::UserRole + 1);
	if (variant.canConvert<DeviceInfoContainer>())
	{
		auto selectedDeviceInfo = variant.value<DeviceInfoContainer>();

		try
		{
			_pgrabber->deviceClose();
			_pgrabber->deviceOpen(selectedDeviceInfo.info);
			accept();
		}
		catch (ic4::IC4Exception ex)
		{
			QMessageBox::critical(this, {}, ex.what());
		}
	}
}
