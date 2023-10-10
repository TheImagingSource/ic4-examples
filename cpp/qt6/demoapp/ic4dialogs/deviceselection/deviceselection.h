#pragma once

#include <QDialog>
#include <QTreeView>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <ic4/ic4.h> 

class ic4item : public QStandardItem
{
public:
	ic4item(bool  isDevice) : QStandardItem(),
		_isDevice(isDevice)
	{}

	bool isDevice() { return _isDevice; }

private:
	bool _isDevice;
};

class DeviceItem : public ic4item
{
public:
	DeviceItem(ic4::DeviceInfo devinfo) : ic4item(true),
		_devinfo(devinfo)
	{
		//this->setText(_devinfo.uniqueName().c_str());
		this->setText((_devinfo.modelName() + " - " + _devinfo.serial()).c_str());
		setEditable(false);
	}

	std::string getName()
	{
		return _devinfo.uniqueName();
	}

	ic4::DeviceInfo getDevInfo() { return _devinfo; }

private:
	ic4::DeviceInfo _devinfo;
};


class InterfaceItem : public ic4item
{
public:
	InterfaceItem(ic4::Interface itf) : ic4item(false),
		_itf(itf)
	{
		this->setText(_itf.interfaceDisplayName().c_str());
		enumdevices();
	}

	std::string getName()
	{
		return _itf.interfaceDisplayName();
	}


private:
	void enumdevices()
	{
		for (auto dev : _itf.enumDevices())
		{
			this->appendRow(new DeviceItem(dev));
		}
	}

	ic4::Interface _itf;
};


class DeviceSelectionDlg : public QDialog
{
	Q_OBJECT

public:
	DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber);

private slots:
	void OnCancel();
	void OnOK();
	void OnUpdateButton();
	void onClickedDevice(const QModelIndex& index);


private:
	void createUI();
	void enumerateDevices();

	ic4::Grabber* _pgrabber;
	QTreeView* _cameraTree;
	QStandardItemModel _model;
	QModelIndex _selectedindex;

};