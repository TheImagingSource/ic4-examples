#pragma once

#include <QDialog>
#include <QTreeView>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QPushButton>
#include <ic4/ic4.h> 


class DeviceSelectionDlg : public QDialog
{
	Q_OBJECT

public:
	DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber);

private slots:
	void OnOK();
	void OnUpdateButton();
	void onClickedDevice(const QModelIndex& index);
	void onSelectDevice(const QModelIndex& current, const QModelIndex& previous);


private:
	void createUI();
	void enumerateDevices();

	ic4::Grabber* _pgrabber;
	QTreeView* _cameraTree;
	QStandardItemModel _model;
	QPushButton* _OKButton;
	QWidget* _propTree = nullptr;
};