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
	auto deviceslayout = new QHBoxLayout();

	_cameraTree = new QTreeView();
	bool ok = connect(_cameraTree, &QAbstractItemView::clicked, this, &DeviceSelectionDlg::onClickedDevice);

	deviceslayout->addWidget(_cameraTree);

	MainLayout->addLayout(deviceslayout);

	QHBoxLayout* buttons = new QHBoxLayout();

	//////////////////////////////////////////////////////////
	auto UpdateButton = new QPushButton(tr("Update"));
	connect(UpdateButton, SIGNAL(released()), this, SLOT(OnUpdateButton()));
	buttons->addWidget(UpdateButton);

	auto CancelButton = new QPushButton(tr("Cancel"));
	connect(CancelButton, SIGNAL(released()), this, SLOT(OnCancel()));
	buttons->addWidget(CancelButton);

	auto OKButton = new QPushButton(tr("OK"));
	connect(OKButton, SIGNAL(released()), this, SLOT(OnOK()));
	buttons->addWidget(OKButton);

	MainLayout->addLayout(buttons);

	setLayout(MainLayout);
}

void DeviceSelectionDlg::enumerateDevices()
{
	int r = 0;
	_model.clear();

	for (auto itf : ic4::DeviceEnum::enumInterfaces())
	{
		if (itf.enumDevices().size() > 0)
		{
			_model.setItem(r, 0, new InterfaceItem(itf));
			r++;
		}
	}

	_cameraTree->setModel(&_model);
	_cameraTree->expandAll();
}


void DeviceSelectionDlg::onClickedDevice(const QModelIndex& index)
{
	std::cout << index.row() << "  " << index.column() << std::endl;

	_selectedindex = index;
	if (((ic4item*)_model.itemFromIndex(index))->isDevice())
	{
		auto x = (DeviceItem*)_model.itemFromIndex(index);
		std::cout << x->getDevInfo().modelName() << " " << x->getDevInfo().getInterface().interfaceDisplayName() << std::endl;
	}
}

void DeviceSelectionDlg::OnUpdateButton()
{
	enumerateDevices();
}

void DeviceSelectionDlg::OnOK()
{
	if (_model.itemFromIndex(_selectedindex) != nullptr)
	{
		if (((ic4item*)_model.itemFromIndex(_selectedindex))->isDevice())
		{
			auto x = (DeviceItem*)_model.itemFromIndex(_selectedindex);
			try
			{
				_pgrabber->deviceClose();
				std::cout << x->getDevInfo().modelName() << " " << x->getDevInfo().getInterface().interfaceDisplayName() << std::endl;
				_pgrabber->deviceOpen(x->getDevInfo());
			}
			catch (ic4::IC4Exception ex)
			{
				QMessageBox::critical(this, "IC4 DemoApp", ex.what());
			}
		}
	}
	this->done(Accepted);
}

void DeviceSelectionDlg::OnCancel()
{
	this->done(Rejected);
}