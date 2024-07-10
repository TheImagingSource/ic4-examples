
#include "PropertyDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

PropertyDialog::PropertyDialog(ic4::PropertyMap map, QWidget* parent, const QString& title, ic4::PropVisibility startupVisibility)
	: PropertyDialog(map, nullptr, parent, title, startupVisibility)
{
}

PropertyDialog::PropertyDialog(ic4::Grabber& grabber, QWidget* parent, const QString& title, ic4::PropVisibility startupVisibility)
	: PropertyDialog(grabber.devicePropertyMap(), &grabber, parent, title, startupVisibility)
{
}

PropertyDialog::PropertyDialog(ic4::PropertyMap map, ic4::Grabber* grabber, QWidget* parent, const QString& title, ic4::PropVisibility startupVisibility)
	: QDialog(parent)
	, _map(map)
	, _grabber(grabber)
{
	setWindowTitle(title);
	createUI(startupVisibility);
}

void PropertyDialog::createUI(ic4::PropVisibility startupVisibility)
{
	setMinimumSize(500, 700);

	ic4::ui::PropertyTreeWidget::Settings treeSettings = { false, true, true, "", startupVisibility };
	auto tree = new ic4::ui::PropertyTreeWidget(_map.findCategory("Root"), _grabber, treeSettings, this);

	auto buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addWidget(tree);
	mainLayout->addWidget(buttons);
	setLayout(mainLayout);
}