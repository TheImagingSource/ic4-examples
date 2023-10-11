#pragma once

#include <QDialog>
#include <ic4/ic4.h> 
#include "PropertyTreeWidget.h"
#include <vector>

class PropertyMapDlg : public QDialog
{
	Q_OBJECT

public:
	PropertyMapDlg(ic4::PropertyMap map, QWidget* parent);

private:
	void createUI(ic4::PropertyMap map);
	ic4::PropertyMap _map;
	std::vector<uint8_t> oldstate;
};



