#pragma once

#include "controls/PropertyTreeWidget.h"

#include <ic4/ic4.h> 

#include <QDialog>

#include <vector>

class PropertyDialog : public QDialog
{
	Q_OBJECT

public:
	PropertyDialog(ic4::PropertyMap map, QWidget* parent, ic4::Grabber* grabber);

private:
	void createUI();

	ic4::Grabber* _grabber;
	ic4::PropertyMap _map;
	std::vector<uint8_t> oldstate;
};



