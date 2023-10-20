#pragma once

#include "deviceselection.h"
#include "PropertyControls.h"
#include "PropertyTreeWidget.h"

#include <iostream>
#include <QMessagebox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <iostream>

class DeviceItem : public QStandardItem
{
public:
	DeviceItem(ic4::DeviceInfo devinfo, int index)
		: QStandardItem((devinfo.modelName() + " - " + devinfo.serial()).c_str())
		, _devinfo(devinfo)
		, _index(index)
	{
		setEditable(false);
	}

	ic4::DeviceInfo getDevInfo() const { return _devinfo; }
	int index() const { return _index; }

private:
	ic4::DeviceInfo _devinfo;
	int _index;
};


class InterfaceItem : public QStandardItem
{
public:
	InterfaceItem(ic4::Interface itf)
		: QStandardItem(itf.interfaceDisplayName().c_str())
		, _itf(itf)
	{
		setEditable(false);
		enumdevices();
	}

	ic4::PropertyMap propertyMap() const
	{
		return _itf.interfacePropertyMap();
	}

private:
	void enumdevices()
	{
		int index = 0;
		for (auto&& dev : _itf.enumDevices())
		{
			appendRow(new DeviceItem(dev, index++));
		}
	}

	ic4::Interface _itf;
};

DeviceSelectionDlg::DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber) : QDialog(parent)
{
	_pgrabber = pgrabber;

	createUI();
	enumerateDevices();

	auto root = _cameraTree->model()->index(0, 0);

	_cameraTree->selectionModel()->select(root, QItemSelectionModel::SelectionFlag::Select);
}


void DeviceSelectionDlg::createUI()
{
	this->setWindowTitle("Device Selection");
	setMinimumSize(800, 600);

	auto topLayout = new QHBoxLayout();

	auto MainLayout = new QVBoxLayout();

	_cameraTree = new QTreeView();
	_cameraTree->setHeaderHidden(true);
	_cameraTree->setSelectionMode(QAbstractItemView::SingleSelection);
	_cameraTree->setModel(&_model);
	connect(_cameraTree, &QTreeView::clicked, this, &DeviceSelectionDlg::onClickedDevice);
	connect(_cameraTree, &QTreeView::doubleClicked, this, &DeviceSelectionDlg::OnOK);
	connect(_cameraTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &DeviceSelectionDlg::onSelectDevice);

	MainLayout->addWidget(_cameraTree);

	QHBoxLayout* buttons = new QHBoxLayout();

	//////////////////////////////////////////////////////////
	auto UpdateButton = new QPushButton(tr("Update"));
	connect(UpdateButton, &QPushButton::pressed, this, &DeviceSelectionDlg::OnUpdateButton);
	buttons->addWidget(UpdateButton);

	auto cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, &QPushButton::pressed, this, &QDialog::reject);
	buttons->addWidget(cancelButton);

	_OKButton = new QPushButton(tr("OK"));
	connect(_OKButton, &QPushButton::pressed, this, &DeviceSelectionDlg::OnOK);
	buttons->addWidget(_OKButton);

	MainLayout->addLayout(buttons);
	topLayout->addLayout(MainLayout, 1);

	auto rightLayout = new QVBoxLayout();
	topLayout->addLayout(rightLayout, 2);

	setLayout(topLayout);
}

void DeviceSelectionDlg::enumerateDevices()
{
	_model.clear();

	for (auto&& itf : ic4::DeviceEnum::enumInterfaces())
	{
		if (!itf.enumDevices().empty())
		{
			_model.appendRow(new InterfaceItem(itf));
		}
	}

	_cameraTree->setModel(&_model);
	_cameraTree->expandAll();
}


void DeviceSelectionDlg::onSelectDevice(const QModelIndex& current, const QModelIndex& previous)
{
	onClickedDevice(current);
}

void DeviceSelectionDlg::onClickedDevice(const QModelIndex& index)
{	
	auto* deviceItem = dynamic_cast<DeviceItem*>(_model.itemFromIndex(index));
	auto* interfaceItem = dynamic_cast<InterfaceItem*>(_model.itemFromIndex(index));

	_OKButton->setEnabled(deviceItem != nullptr);

	ic4::PropertyMap map;

	ic4::ui::PropertyTreeWidget<QWidget>::Settings settings = {};
	settings.showRootItem = false;
	settings.showFilter = false;
	settings.showInfoBox = false;
	settings.initialVisibility = ic4::PropVisibility::Guru;
	
	if (deviceItem)
	{
		map = deviceItem->getDevInfo().getInterface().interfacePropertyMap();
		map.setValue("DeviceSelector", deviceItem->index());

		settings.initialFilter = "InterfaceDisplayName|MaximumTransmissionUnit|DeviceVendorName|DeviceSerialNumber|DeviceManufacturer|DeviceVersion|DeviceUserID|GevDeviceMACAddress|GevDeviceIPAddress";
	}
	else
	{
		map = interfaceItem->propertyMap();

		settings.initialFilter = "InterfaceDisplayName|MaximumTransmissionUnit";
	}

	auto root = map.findCategory("Root");
	auto rightLayout = dynamic_cast<QVBoxLayout*>(layout()->itemAt(1));

	delete _propTree;
	_propTree = new ic4::ui::PropertyTreeWidget(root, nullptr, settings, this);
	rightLayout->addWidget(_propTree);
}

void DeviceSelectionDlg::OnUpdateButton()
{
	enumerateDevices();
}

void DeviceSelectionDlg::OnOK()
{	
	auto index = _cameraTree->selectionModel()->currentIndex();

	auto* deviceItem = dynamic_cast<DeviceItem*>(_model.itemFromIndex(index));
	if( deviceItem )
	{
		try
		{
			_pgrabber->deviceClose();
			_pgrabber->deviceOpen(deviceItem->getDevInfo());
			QDialog::done(Accepted);
		}
		catch (ic4::IC4Exception ex)
		{
			QMessageBox::critical(this, "IC4 DemoApp", ex.what());
		}
	}	
}
