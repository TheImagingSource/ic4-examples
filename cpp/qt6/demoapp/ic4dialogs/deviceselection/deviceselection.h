#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QModelIndex>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
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

	QLabel* _itfInfoHeader;
	QFormLayout* _itfInfoLayout;
	QLabel* _devInfoHeader;
	QFormLayout* _devInfoLayout;
};