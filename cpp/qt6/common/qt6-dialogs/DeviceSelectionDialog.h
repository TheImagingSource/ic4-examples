#pragma once

#include "controls/IPConfigGroupBox.h"

#include <QDialog>
#include <QTreeWidget>
#include <QModelIndex>
#include <QPushButton>
#include <QFormLayout>
#include <QScrollArea>

#include <ic4/ic4.h>


class DeviceSelectionDlg : public QDialog
{
	Q_OBJECT

public:
	DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber, std::function<bool(const ic4::DeviceInfo&)> filter = nullptr);

protected:
	void customEvent(QEvent* event) override;

private slots:
	void onOK();
	void onUpdateButton();
	void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);


private:
	void createUI();
	void enumerateDevices();
	bool selectPreviousItem(QVariant itemData);

	std::function<bool(const ic4::DeviceInfo&)> _filter_func;

	ic4::Grabber* _grabber;
	ic4::DeviceEnum _enumerator;
	QTreeWidget* _cameraTree;
	QPushButton* _okButton;

	QScrollArea* _rightScroll;
	FormGroupBox* _itfInfoGroup;
	FormGroupBox* _devInfoGroup;
	IPConfigGroupBox* _ipConfigGroup;
};