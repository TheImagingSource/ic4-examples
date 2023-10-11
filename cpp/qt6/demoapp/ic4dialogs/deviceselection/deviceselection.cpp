#pragma once

#include "deviceselection.h"
#include <iostream>
#include <QMessagebox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <iostream>

DeviceSelectionDlg::DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber) : QDialog(parent)
{
	_pgrabber = pgrabber;

	createUI();
	enumerateDevices();
}


void DeviceSelectionDlg::createUI()
{
	this->setWindowTitle("Device Selection");
	setMinimumSize(200, 200);

	auto MainLayout = new QVBoxLayout();

	_cameraTree = new QTreeView();
	_cameraTree->setHeaderHidden(true);
	connect(_cameraTree, &QTreeView::clicked, this, &DeviceSelectionDlg::onClickedDevice);
	connect(_cameraTree, &QTreeView::doubleClicked, this, &DeviceSelectionDlg::OnOK);

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

	setLayout(MainLayout);
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


void DeviceSelectionDlg::onClickedDevice(const QModelIndex& index)
{	
	auto* deviceItem = dynamic_cast<DeviceItem*>(_model.itemFromIndex(index));
	_OKButton->setEnabled(deviceItem != nullptr);
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
