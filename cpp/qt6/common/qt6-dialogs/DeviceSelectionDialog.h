#pragma once

#include "controls/IPConfigGroupBox.h"
#include "controls/SwitchDriverGroupBox.h"

#include <QDialog>
#include <QTreeWidget>
#include <QModelIndex>
#include <QPushButton>
#include <QFormLayout>
#include <QScrollArea>

#include <ic4/ic4.h>


class DeviceSelectionDialog : public QDialog
{
	Q_OBJECT

public:
	DeviceSelectionDialog(QWidget* parent, ic4::Grabber* pgrabber, std::function<bool(const ic4::DeviceInfo&)> filter = nullptr);

protected:
	void customEvent(QEvent* event) override;

private slots:
	void onOK();
	void onSystemInfoButton();
	void onRefreshButton();
	void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);


private:
	void createUI();
	void enumerateDevices();
	bool selectPreviousItem(QVariant itemData);

	std::function<bool(const ic4::DeviceInfo&)> _filter_func;

	ic4::Grabber* _grabber;
	ic4::DeviceEnum _enumerator;
	QTreeWidget* _cameraTree = nullptr;
	QPushButton* _okButton = nullptr;

	QScrollArea* _rightScroll = nullptr;
	FormGroupBox* _itfInfoGroup = nullptr;
	FormGroupBox* _devInfoGroup = nullptr;
	IPConfigGroupBox* _ipConfigGroup = nullptr;
	SwitchDriverGroupBox* _switchDriverGroup = nullptr;
};