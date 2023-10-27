#pragma once

#include "deviceselection.h"
#include "controls/PropertyControls.h"
#include "ResourceSelector.h"

#include <iostream>
#include <QMessagebox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QEvent>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QtAlgorithms>
#include <QScrollArea>
#include <QScrollBar>

const QEvent::Type EVENT_DEVICE_LIST_CHANGED = static_cast<QEvent::Type>(QEvent::User + 3);

DeviceSelectionDlg::DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber, std::function<bool(const ic4::DeviceInfo&)> filter)
	: QDialog(parent)
	, _filter_func(filter)
	, _pgrabber(pgrabber)
{
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

struct InterfaceDeviceItemData
{
	ic4::Interface itf;
	ic4::PropertyMap itfPropertyMap;
	ic4::DeviceInfo device;
	int64_t deviceIndex = 0;

	bool isDevice() const
	{
		return device.is_valid();
	}
};
Q_DECLARE_METATYPE(InterfaceDeviceItemData)

void DeviceSelectionDlg::createUI()
{
	Q_INIT_RESOURCE(qt6dialogs);

	this->setWindowTitle("Select Device");
	setMinimumSize(900, 400);

	auto topLayout = new QHBoxLayout();
	auto leftLayout = new QVBoxLayout();

	_cameraTree = new QTreeWidget();
	_cameraTree->setIconSize(QSize(24, 24));
	_cameraTree->setIndentation(16);
	//_cameraTree->setStyleSheet("QTreeWidget { border: 1px solid #3f3f46; max-height: 1024; padding: 4px 4px 4px 4px; } QTreeView::item  {  padding: 4px 4px 4px 4px; } ");
	_cameraTree->setStyleSheet("QTreeView::item { padding: 4px; } ");
	_cameraTree->setRootIsDecorated(false);
	_cameraTree->setMinimumWidth(450);
	_cameraTree->setItemsExpandable(false);
	QStringList headerLabels = { tr("Device"), tr("Serial Number"), tr("IP Address"), tr("Device User ID") };
	_cameraTree->setColumnCount(headerLabels.count());
	_cameraTree->setHeaderLabels(headerLabels);
	_cameraTree->setColumnWidth(0, 160);
	_cameraTree->setColumnWidth(1, 100);
	_cameraTree->setColumnWidth(2, 100);
	_cameraTree->setColumnWidth(3, 80);
	_cameraTree->setHeaderHidden(false);

	connect(_cameraTree, &QTreeWidget::currentItemChanged, this, &DeviceSelectionDlg::onCurrentItemChanged);
	connect(_cameraTree, &QTreeWidget::itemDoubleClicked, [&](QTreeWidgetItem* item, int column) { OnOK(); });

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

	_itfInfoGroup = new FormGroupBox(tr("Interface Information"));
	_devInfoGroup = new FormGroupBox(tr("Device Information"));
	_ipConfigGroup = new IPConfigGroupBox(tr("IP Configuration"));

	_rightScroll = new QScrollArea();
	_rightScroll->setObjectName("rightScroll");
	_rightScroll->setStyleSheet("QScrollArea#rightScroll { border-width: 1; border-style: solid; border-color: palette(base); }");

	auto rightBox = new QFrame();
	rightBox->setObjectName("rightBox");
	rightBox->setStyleSheet("QFrame#rightBox { padding: 0px; }");

	auto rightLayout = new QVBoxLayout();
	rightLayout->setContentsMargins(0, 0, 0, 0);

	rightLayout->addWidget(_itfInfoGroup, 0);
	rightLayout->addWidget(_devInfoGroup, 0);
	rightLayout->addWidget(_ipConfigGroup, 0);
	rightLayout->addStretch(1);

	rightBox->setLayout(rightLayout);

	_rightScroll->setWidget(rightBox);
	_rightScroll->setWidgetResizable(true);

	topLayout->addWidget(_rightScroll, 2);

	setLayout(topLayout);
}

void DeviceSelectionDlg::enumerateDevices()
{
	//QSignalBlocker blk(_cameraTree);
	_cameraTree->clear();

	for (auto&& itf : ic4::DeviceEnum::enumInterfaces())
	{
		auto itf_devices = itf.enumDevices();

		std::vector<ic4::DeviceInfo> filtered_itf_devices;
		if (_filter_func)
		{
			std::copy_if(itf_devices.begin(), itf_devices.end(), std::back_inserter(filtered_itf_devices), _filter_func);
		}
		else
		{
			filtered_itf_devices = itf_devices;
		}

		if (filtered_itf_devices.empty())
			continue;

		ic4::Error err;

		auto map = itf.interfacePropertyMap(err);
		if (err.isError())
			continue;

		auto* itf_item = new QTreeWidgetItem(_cameraTree);
		itf_item->setText(0, QString::fromStdString(itf.interfaceDisplayName()));
		itf_item->setForeground(0, QPalette().windowText());
		itf_item->setData(0, Qt::UserRole + 1, QVariant::fromValue(InterfaceDeviceItemData{ itf, map }));
		itf_item->setFirstColumnSpanned(true);

		int index = 0;
		for (auto&& dev : filtered_itf_devices)
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

			auto variant = QVariant::fromValue(InterfaceDeviceItemData{ itf, map, dev, index });
			node->setData(0, Qt::UserRole + 1, variant);

			node->setText(0, QString::fromStdString(device));
			node->setText(1, QString::fromStdString(serial));
			node->setText(2, strIPAddress);
			node->setText(3, QString::fromStdString(deviceUserID));

			auto cam = ResourceSelector::instance().loadIcon(":/images/camera_icon_usb3.png");
			node->setIcon(0, cam);

			itf_item->addChild(node);

			index += 1;
		}
	}

	_cameraTree->expandAll();
}

static void clearFormLayout(QFormLayout& layout)
{
	while (layout.count() != 0)
	{
		QLayoutItem* forDeletion = layout.takeAt(0);
		delete forDeletion->widget();
		delete forDeletion;
	}
}

static QString buildIPAddress(const ic4::PropertyMap& map, const char* ip_feature, const char* subnet_feature)
{
	ic4::Error err;

	auto ip = map.getValueString(ip_feature, err);
	if (err.isError())
		return {};

	auto subnet = map.getValueInt64(subnet_feature, err);
	if (err.isError())
		return {};

	int leadingOnes = qCountLeadingZeroBits(static_cast<uint32_t>(~subnet));
	int trailingZeros = qCountTrailingZeroBits(static_cast<uint32_t>(subnet));

	if (leadingOnes + trailingZeros == 32)
	{
		return QString("%1/%2")
			.arg(ip.c_str())
			.arg(leadingOnes);
	}
	else
	{
		auto subnetString = map.getValueString(subnet_feature, err);
		if (err.isError())
			return {};

		return QString("%1/%2")
			.arg(ip.c_str())
			.arg(subnetString.c_str());
	}
}

static QStringList buildInterfaceIPAddressList(const ic4::PropertyMap& map)
{
	ic4::Error err;
	auto selector = map.findInteger("GevInterfaceSubnetSelector", err);
	if (err.isError())
		return {};

	int64_t max = selector.getMaximum(ic4::Error::Ignore());
	QStringList result;

	for (int64_t i = 0; i <= max; ++i)
	{
		if (!selector.setValue(i, err))
			continue;

		auto addr = buildIPAddress(map, "GevInterfaceSubnetIPAddress", "GevInterfaceSubnetMask");
		if (addr.isEmpty())
			continue;

		result.push_back(addr);
	}

	return result;
}

void synchronizeColumnWidths(std::vector<QFormLayout*> layouts)
{
	int maxWidth = 0;

	for (auto&& layout : layouts)
	{
		for (int i = 0; i < layout->rowCount(); ++i)
		{
			auto* label = layout->itemAt(i, QFormLayout::ItemRole::LabelRole);
			if (label != nullptr)
			{
				auto* w = label->widget();
				auto* lbl = (QLabel*)w;
				auto width = lbl->fontMetrics().size(0, lbl->text()).width();
				maxWidth = std::max(maxWidth, width);
			}
		}
	}

	for (auto&& layout : layouts)
	{
		for (int i = 0; i < layout->rowCount(); ++i)
		{
			auto* label = layout->itemAt(i, QFormLayout::ItemRole::LabelRole);
			if (label != nullptr)
			{
				label->widget()->setMinimumWidth(maxWidth);
			}
		}
	}
}

void DeviceSelectionDlg::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{	
	_rightScroll->hide();

	_OKButton->setEnabled(false);

	_itfInfoGroup->hide();
	_itfInfoGroup->clear();
	_devInfoGroup->hide();
	_devInfoGroup->clear();
	_ipConfigGroup->hide();
	_ipConfigGroup->clear();

	if (current == nullptr)
	{
		_rightScroll->show();
		return;
	}

	auto variant = current->data(0, Qt::UserRole + 1);
	auto itemData = variant.value<InterfaceDeviceItemData>();

	bool isGigEVisionInterface = itemData.itf.transportLayerType(ic4::Error::Ignore()) == ic4::TransportLayerType::GigEVision;
	ic4::PropertyMap map = itemData.itfPropertyMap;

	if (itemData.isDevice())
	{
		if (map.setValue("DeviceSelector", itemData.deviceIndex, ic4::Error::Ignore()))
		{
			_OKButton->setEnabled(true);
		}
	}

	auto addStringItem = [](const char* label, const QString& value, QFormLayout& layout) -> QLineEdit*
		{
			auto edit = new QLineEdit(value);
			edit->setReadOnly(true);
			edit->setCursorPosition(0);
			layout.addRow(tr(label), edit);
			return edit;
		};

	auto buildStringItemIfExists = [&addStringItem](const ic4::PropertyMap& map, const char* prop_item, const char* label, QFormLayout& layout) -> QLineEdit*
		{
			ic4::Error err;
			auto value = map.getValueString(prop_item, err);
			if (err.isSuccess())
			{
				return addStringItem(label, QString::fromStdString(value), layout);
			}
			return nullptr;
		};

	_itfInfoGroup->show();

	buildStringItemIfExists(map, "InterfaceDisplayName", "Interface Name", *_itfInfoGroup->formLayout());	

	if (isGigEVisionInterface)
	{
		auto interfaceIPAddresses = buildInterfaceIPAddressList(map);
		if (interfaceIPAddresses.count() == 1)
		{
			auto edit = new QLineEdit(interfaceIPAddresses.at(0));
			edit->setReadOnly(true);
			_itfInfoGroup->formLayout()->addRow(tr("IP Address"), edit);
		}
		else if (!interfaceIPAddresses.isEmpty())
		{
			auto txt = interfaceIPAddresses.join("\r\n");
			auto edit = new QPlainTextEdit(txt);
			edit->document()->setDocumentMargin(2);
			edit->setReadOnly(true);
			auto docmargin = edit->document()->documentMargin();
			auto margins = edit->contentsMargins();
			auto frameWidth = edit->frameWidth();
			auto fontHeight = edit->fontMetrics().height();
			edit->setFixedHeight(fontHeight * interfaceIPAddresses.count() + frameWidth * 2 + margins.top() + margins.bottom() + (int)docmargin * 2);
			edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
			_itfInfoGroup->formLayout()->addRow("IP Addresses", edit);
		}

		buildStringItemIfExists(map, "MaximumTransmissionUnit", "Maximum Transmission Unit", *_itfInfoGroup->formLayout());
	}

	if (itemData.isDevice())
	{
		_devInfoGroup->show();

		buildStringItemIfExists(map, "DeviceModelName", "Model Name", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceVendorName", "Vendor Name", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceSerialNumber", "Serial Number", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceVersion", "Device Version", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceUserID", "Device User ID", *_devInfoGroup->formLayout());

		if (isGigEVisionInterface)
		{
			auto devIPAddress = buildIPAddress(map, "GevDeviceIPAddress", "GevDeviceSubnetMask");
			if (!devIPAddress.isEmpty())
				addStringItem("Device IP Address", devIPAddress, *_devInfoGroup->formLayout());

			buildStringItemIfExists(map, "GevDeviceGateway", "Device Gateway", *_devInfoGroup->formLayout());
			buildStringItemIfExists(map, "GevDeviceMACAddress", "Device MAC Address", *_devInfoGroup->formLayout());

			auto reachableStatus = map.getValueString("DeviceReachableStatus", ic4::Error::Ignore());
			if (reachableStatus == "Reachable")
			{				
				_ipConfigGroup->update(itemData.device);
			}
			else
			{
				_ipConfigGroup->updateUnreachable(map);
			}

			_ipConfigGroup->show();
		}
	}

	synchronizeColumnWidths({ _devInfoGroup->formLayout(), _itfInfoGroup->formLayout(), _ipConfigGroup->formLayout()});

	_rightScroll->verticalScrollBar()->setValue(0);
	_rightScroll->show();
}

void DeviceSelectionDlg::selectPreviousItem(QVariant itemVariant)
{
	auto itemData = itemVariant.value<InterfaceDeviceItemData>();

	for (int i = 0; i < _cameraTree->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* itfItem = _cameraTree->topLevelItem(i);

		if (!itemData.isDevice())
		{
			auto itfVariant = itfItem->data(0, Qt::UserRole + 1);
			if (itfVariant.value<InterfaceDeviceItemData>().itf == itemData.itf)
			{
				_cameraTree->setCurrentItem(itfItem);
				return;
			}
		}
		else
		{
			for (int j = 0; j < itfItem->childCount(); ++j)
			{
				QTreeWidgetItem* devItem = itfItem->child(j);
				auto devVariant = devItem->data(0, Qt::UserRole + 1);

				if (devVariant.value<InterfaceDeviceItemData>().device == itemData.device)
				{
					_cameraTree->setCurrentItem(devItem);
					return;
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
}

void DeviceSelectionDlg::OnOK()
{
	auto* item = _cameraTree->currentItem();
	if (item == nullptr)
		return;

	auto variant = item->data(0, Qt::UserRole + 1);
	auto itemData = variant.value<InterfaceDeviceItemData>();

	if (itemData.isDevice())
	{
		try
		{
			_pgrabber->deviceClose();
			_pgrabber->deviceOpen(itemData.device);
			accept();
		}
		catch (ic4::IC4Exception ex)
		{
			QMessageBox::critical(this, {}, ex.what());
		}
	}
}
