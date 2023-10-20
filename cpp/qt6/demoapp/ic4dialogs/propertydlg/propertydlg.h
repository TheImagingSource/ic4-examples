#pragma once

#include <QDialog>
#include <ic4/ic4.h> 
#include "PropertyTreeWidget.h"
#include <vector>

class PropertyMapDlg : public QDialog
{
	Q_OBJECT

public:
	PropertyMapDlg(ic4::PropertyMap map, QWidget* parent, ic4::Grabber* grabber);

private:
	void createUI();

	ic4::Grabber* _grabber;
	ic4::PropertyMap _map;
	std::vector<uint8_t> oldstate;
};



