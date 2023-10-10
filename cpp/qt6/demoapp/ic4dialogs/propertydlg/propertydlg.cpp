
#include "propertydlg.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

PropertyMapDlg::PropertyMapDlg(ic4::PropertyMap map, QWidget* parent) : QDialog(parent),
_map(map)
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
	auto tree = new ic4::ui::PropertyTreeWidget<QWidget>(_map.findCategory("Root"), displaysettigns, this);

	mainLayout->addWidget(tree);

	auto buttonslayout = new QHBoxLayout();

	QPushButton* btnCancel = new QPushButton("Cancel");
	buttonslayout->addWidget(btnCancel);

	QPushButton* btnOK = new QPushButton("OK");
	buttonslayout->addWidget(btnOK);

	connect(btnOK, &QPushButton::clicked, this, &QDialog::accept);
	connect(btnCancel, &QPushButton::clicked, this, &PropertyMapDlg::reject);

	mainLayout->addLayout(buttonslayout);
	setLayout(mainLayout);
}

void PropertyMapDlg::reject()
{
	_map.deSerialize(oldstate);

	QDialog::reject();
}