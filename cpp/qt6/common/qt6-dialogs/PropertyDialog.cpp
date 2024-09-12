
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
	, _grabber(grabber)
	, _map(map)
{
	setWindowTitle(title);
	createUI();
}

void PropertyDialog::createUI()
{
	setMinimumSize(500, 700);

	ic4::ui::PropertyTreeWidget::Settings treeSettings = { false, true, true, "", ic4::PropVisibility::Beginner };
	_tree = new ic4::ui::PropertyTreeWidget(_map.findCategory("Root"), _grabber, treeSettings, this);

	auto buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addWidget(_tree);
	mainLayout->addWidget(buttons);
	setLayout(mainLayout);
}

void PropertyDialog::updateGrabber(ic4::Grabber& grabber)
{
	_map = grabber.devicePropertyMap();
	_grabber = &grabber;

	_tree->updateGrabber(_grabber);
}

void PropertyDialog::updatePropertyMap(ic4::PropertyMap map)
{
	_map = map;
	_grabber = nullptr;

	_tree->updateModel(_map.findCategory("Root"));
}

void PropertyDialog::setPropVisibility(ic4::PropVisibility visibility)
{
	_tree->setPropVisibility(visibility);
}

void PropertyDialog::setFilterText(const QString& filterText)
{
	_tree->setFilterText(filterText);
}