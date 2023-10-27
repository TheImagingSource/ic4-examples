#pragma once

#include "IPConfigGroupBox.h"

#include <QDialog>
#include <QTreeWidget>
#include <QModelIndex>
#include <QPushButton>
#include <QFormLayout>
#include <QGroupBox>

#include <ic4/ic4.h>


class DeviceSelectionDlg : public QDialog
{
	Q_OBJECT

public:
	DeviceSelectionDlg(QWidget* parent, ic4::Grabber* pgrabber);

protected:
	void customEvent(QEvent* event) override;

private slots:
	void OnOK();
	void OnUpdateButton();
	void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);


private:
	void createUI();
	void enumerateDevices();
	void selectPreviousItem(QVariant itemData);

	ic4::Grabber* _pgrabber;
	ic4::DeviceEnum _enumerator;
	QTreeWidget* _cameraTree;
	QPushButton* _OKButton;

	QGroupBox* _itfInfoGroup;
	QFormLayout* _itfInfoLayout;
	QGroupBox* _devInfoGroup;
	QFormLayout* _devInfoLayout;
	IPConfigGroupBox* _ipConfigGroup;
};