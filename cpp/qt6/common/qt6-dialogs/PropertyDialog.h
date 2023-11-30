#pragma once

#include "controls/PropertyTreeWidget.h"

#include <ic4/ic4.h> 

#include <QDialog>

#include <vector>

class PropertyDialog : public QDialog
{
	Q_OBJECT

public:
	PropertyDialog(ic4::PropertyMap map, QWidget* parent, const QString& title = {});
	PropertyDialog(ic4::Grabber& grabber, QWidget* parent, const QString& title = {});

private:
	PropertyDialog(ic4::PropertyMap map, ic4::Grabber* grabber, QWidget* parent, const QString& title);

	void createUI();

	ic4::Grabber* _grabber = nullptr;
	ic4::PropertyMap _map;
};



