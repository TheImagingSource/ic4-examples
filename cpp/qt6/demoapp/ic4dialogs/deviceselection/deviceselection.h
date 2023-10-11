#pragma once

#include <QDialog>
#include <QTreeView>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QPushButton>
#include <ic4/ic4.h> 

class DeviceItem : public QStandardItem
{
public:
	DeviceItem(ic4::DeviceInfo devinfo)
		: QStandardItem((devinfo.modelName() + " - " + devinfo.serial()).c_str())
		, _devinfo(devinfo)
	{
		setEditable(false);
	}

	ic4::DeviceInfo getDevInfo() { return _devinfo; }

private:
	ic4::DeviceInfo _devinfo;
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

private:
	void enumdevices()
	{
		for (auto&& dev : _itf.enumDevices())
		{
			appendRow(new DeviceItem(dev));
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
	void OnOK();
	void OnUpdateButton();
	void onClickedDevice(const QModelIndex& index);


private:
	void createUI();
	void enumerateDevices();

	ic4::Grabber* _pgrabber;
	QTreeView* _cameraTree;
	QStandardItemModel _model;
	QPushButton* _OKButton;
};