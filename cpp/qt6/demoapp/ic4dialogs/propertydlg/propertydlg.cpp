
#include "propertydlg.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

PropertyMapDlg::PropertyMapDlg(ic4::PropertyMap map, QWidget* parent)
	: QDialog(parent)
	, _map(map)
{
	_map.serialize(oldstate);
	createUI(map);
}

void PropertyMapDlg::createUI(ic4::PropertyMap map)
{
	this->setWindowTitle("Properties");
	setMinimumSize(500, 700);

	auto mainLayout = new QVBoxLayout();

	ic4::ui::PropertyTreeWidget<QWidget>::Settings displaysettigns = { false, true, true };
	auto tree = new ic4::ui::PropertyTreeWidget(_map.findCategory("Root"), displaysettigns, this);
	mainLayout->addWidget(tree);

	auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttons);

	connect(this, &QDialog::rejected,
		[this] {
			_map.deSerialize(oldstate, ic4::Error::Ignore());
		}
	);
	
	setLayout(mainLayout);
}