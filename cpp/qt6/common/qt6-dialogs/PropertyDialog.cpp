
#include "PropertyDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

PropertyDialog::PropertyDialog(ic4::PropertyMap map, QWidget* parent, const QString& title)
	: PropertyDialog(map, nullptr, parent, title)
{
}

PropertyDialog::PropertyDialog(ic4::Grabber& grabber, QWidget* parent, const QString& title)
	: PropertyDialog(grabber.devicePropertyMap(), &grabber, parent, title)
{
}

PropertyDialog::PropertyDialog(ic4::PropertyMap map, ic4::Grabber* grabber, QWidget* parent, const QString& title)
	: QDialog(parent)
	, _map(map)
	, _grabber(grabber)
{
	setWindowTitle(title);
	createUI();
}

void PropertyDialog::createUI()
{
	setMinimumSize(500, 700);

	ic4::ui::PropertyTreeWidget::Settings treeSettings = { false, true, true };
	auto tree = new ic4::ui::PropertyTreeWidget(_map.findCategory("Root"), _grabber, treeSettings, this);

	auto buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addWidget(tree);
	mainLayout->addWidget(buttons);
	setLayout(mainLayout);
}