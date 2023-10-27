#pragma once

#include "deviceselection.h"
#include "PropertyControls.h"

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
#include <QtAlgorithms>

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

	_itfInfoLayout = new QFormLayout();
	_itfInfoLayout->setContentsMargins(4, 4, 4, 4);
	_itfInfoLayout->setLabelAlignment(Qt::AlignRight);
	_devInfoLayout = new QFormLayout();
	_devInfoLayout->setContentsMargins(4, 4, 4, 4);
	_devInfoLayout->setLabelAlignment(Qt::AlignRight);

	_itfInfoGroup = new QGroupBox(tr("Interface Information"));
	_itfInfoGroup->setLayout(_itfInfoLayout);
	_devInfoGroup = new QGroupBox(tr("Device Information"));
	_devInfoGroup->setLayout(_devInfoLayout);

	auto rightLayout = new QVBoxLayout();
	rightLayout->addWidget(_itfInfoGroup, 0);
	rightLayout->addWidget(_devInfoGroup, 1);
	rightLayout->addStretch(10);
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

			auto variant = QVariant::fromValue(InterfaceDeviceItemData{ itf, map, dev, index });
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
	_OKButton->setEnabled(false);	

	clearFormLayout(*_itfInfoLayout);
	_devInfoGroup->hide();
	clearFormLayout(*_devInfoLayout);

	if (current == nullptr)
	{
		return;
	}

	auto variant = current->data(0, Qt::UserRole + 1);
	auto itemData = variant.value<InterfaceDeviceItemData>();

	ic4::PropertyMap map = itemData.itfPropertyMap;

	if (itemData.isDevice())
	{
		if (map.setValue("DeviceSelector", itemData.deviceIndex, ic4::Error::Ignore()))
		{
			_OKButton->setEnabled(true);
		}
	}

	auto addStringItem = [](const char* label, const QString& value, QFormLayout& layout)
		{
			auto edit = new QLineEdit(value);
			edit->setReadOnly(true);
			edit->setCursorPosition(0);
			layout.addRow(tr(label), edit);
		};

	auto buildStringItemIfExists = [&addStringItem](const ic4::PropertyMap& map, const char* prop_item, const char* label, QFormLayout& layout)
		{
			ic4::Error err;
			auto value = map.getValueString(prop_item, err);
			if (err.isSuccess())
			{
				addStringItem(label, QString::fromStdString(value), layout);
			}
		};

	buildStringItemIfExists(map, "InterfaceDisplayName", "Interface Name", *_itfInfoLayout);	

	auto interfaceIPAddresses = buildInterfaceIPAddressList(map);
	if (interfaceIPAddresses.count() == 1)
	{
		auto edit = new QLineEdit(interfaceIPAddresses.at(0));
		edit->setReadOnly(true);
		_itfInfoLayout->addRow(tr("IP Address"), edit);
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
		_itfInfoLayout->addRow("IP Addresses", edit);
	}

	buildStringItemIfExists(map, "MaximumTransmissionUnit", "Maximum Transmission Unit", *_itfInfoLayout);

	if (itemData.isDevice())
	{
		_devInfoGroup->show();

		buildStringItemIfExists(map, "DeviceModelName", "Model Name", *_devInfoLayout);
		buildStringItemIfExists(map, "DeviceVendorName", "Vendor Name", *_devInfoLayout);
		buildStringItemIfExists(map, "DeviceSerialNumber", "Serial Number", *_devInfoLayout);
		buildStringItemIfExists(map, "DeviceVersion", "Device Version", *_devInfoLayout);
		buildStringItemIfExists(map, "DeviceUserID", "Device User ID", *_devInfoLayout);

		auto devIPAddress = buildIPAddress(map, "GevDeviceIPAddress", "GevDeviceSubnetMask");
		if( !devIPAddress.isEmpty() )
			addStringItem("Device IP Address", devIPAddress, *_devInfoLayout);

		buildStringItemIfExists(map, "GevDeviceGateway", "Device Gateway", *_devInfoLayout);
		buildStringItemIfExists(map, "GevDeviceMACAddress", "Device MAC Address", *_devInfoLayout);
	}

	synchronizeColumnWidths({ _devInfoLayout, _itfInfoLayout });
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
