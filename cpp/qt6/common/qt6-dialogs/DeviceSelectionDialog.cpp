
#include "DeviceSelectionDialog.h"
#include "controls/PropertyControls.h"
#include "ResourceSelector.h"

#include <iostream>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QEvent>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QtAlgorithms>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QShortcut>


const QEvent::Type EVENT_DEVICE_LIST_CHANGED = static_cast<QEvent::Type>(QEvent::User + 3);
const Qt::ItemDataRole ROLE_ITEM_DATA = static_cast<Qt::ItemDataRole>(Qt::UserRole + 1);

DeviceSelectionDialog::DeviceSelectionDialog(QWidget* parent, ic4::Grabber* pgrabber, std::function<bool(const ic4::DeviceInfo&)> filter)
	: QDialog(parent)
	, _filter_func(filter)
	, _grabber(pgrabber)
{
	createUI();
	onUpdateButton();

	_enumerator.eventAddDeviceListChanged(
		[this](auto&)
		{
			QApplication::postEvent(this, new QEvent(EVENT_DEVICE_LIST_CHANGED));
		}
	);
}

void DeviceSelectionDialog::customEvent(QEvent* event)
{
	if (event->type() == EVENT_DEVICE_LIST_CHANGED)
	{
		onUpdateButton();
	}
}

struct InterfaceDeviceItemData
{
	ic4::Interface itf;
	ic4::PropertyMap itfPropertyMap;
	ic4::DeviceInfo device;
	int64_t deviceIndex = 0;

	bool isDevice() const
	{
		return device.is_valid();
	}
};
Q_DECLARE_METATYPE(InterfaceDeviceItemData)

void DeviceSelectionDialog::createUI()
{
	Q_INIT_RESOURCE(qt6dialogs);

	setWindowTitle("Select Device");

	setMinimumSize(
		ic4::ui::CustomStyle.DeviceSelectionDlgMinWidth, 
		ic4::ui::CustomStyle.DeviceSelectionDlgMinHeight);

	if (ic4::ui::CustomStyle.DeviceSelectionDlgResizeEnabled)
	{
		resize(
			ic4::ui::CustomStyle.DeviceSelectionDlgMinWidth, 
			ic4::ui::CustomStyle.DeviceSelectionDlgMinHeight);
	}

	setSizeGripEnabled(ic4::ui::CustomStyle.DeviceSelectionDlgSizeGripEnabled);

	auto topLayout = new QHBoxLayout();
	auto leftLayout = new QVBoxLayout();

	_cameraTree = new QTreeWidget();
	_cameraTree->setIconSize(QSize(24, 24));
	_cameraTree->setIndentation(16);
	_cameraTree->setStyleSheet(ic4::ui::CustomStyle.DeviceSelectionDlgCameraTreeStyle);
	_cameraTree->setRootIsDecorated(false);
	_cameraTree->setMinimumWidth(450);
	_cameraTree->setItemsExpandable(false);
	QStringList headerLabels = { tr("Device"), tr("Serial Number"), tr("IP Address"), tr("Device User ID") };
	_cameraTree->setColumnCount(headerLabels.count());
	_cameraTree->setHeaderLabels(headerLabels);
	_cameraTree->setColumnWidth(0, 160);
	_cameraTree->setColumnWidth(1, 100);
	_cameraTree->setColumnWidth(2, 100);
	_cameraTree->setColumnWidth(3, 80);
	_cameraTree->setHeaderHidden(false);

	connect(_cameraTree, &QTreeWidget::currentItemChanged, this, &DeviceSelectionDialog::onCurrentItemChanged);
	if (_grabber)
	{
		connect(_cameraTree, &QTreeWidget::itemDoubleClicked, [&](QTreeWidgetItem* item, int column) { onOK(); });
	}

	leftLayout->addWidget(_cameraTree);

	QHBoxLayout* buttons = new QHBoxLayout();

	//////////////////////////////////////////////////////////

	auto systemInfoButton = new QPushButton(tr("System Info"));
	connect(systemInfoButton, &QPushButton::pressed, this, &DeviceSelectionDialog::onSystemInfoButton);
	buttons->addWidget(systemInfoButton);

	auto UpdateButton = new QPushButton(tr("Update (F5)"));
	connect(UpdateButton, &QPushButton::pressed, this, &DeviceSelectionDialog::onUpdateButton);
    UpdateButton->setShortcut(QKeySequence::Refresh);

	buttons->addWidget(UpdateButton);

	if (_grabber)
	{
		auto cancelButton = new QPushButton(tr("Cancel"));
		connect(cancelButton, &QPushButton::pressed, this, &QDialog::reject);
		buttons->addWidget(cancelButton);

		_okButton = new QPushButton(tr("OK"));
		_okButton->setDefault(true);
		connect(_okButton, &QPushButton::pressed, this, &DeviceSelectionDialog::onOK);
		buttons->addWidget(_okButton);
	}
	else
	{
		buttons->addSpacing(150);

		auto closeButton = new QPushButton(tr("Close"));
		closeButton->setDefault(true);
		connect(closeButton, &QPushButton::pressed, this, &QDialog::reject);
		buttons->addWidget(closeButton);
	}

	leftLayout->addLayout(buttons);
	topLayout->addLayout(leftLayout, 1);

	_itfInfoGroup = new FormGroupBox(tr("Interface Information"));
	_itfInfoGroup->setVisible(false);
	_devInfoGroup = new FormGroupBox(tr("Device Information"));
	_devInfoGroup->setVisible(false);
	_ipConfigGroup = new IPConfigGroupBox(tr("IP Configuration"));
	_ipConfigGroup->setVisible(false);
	_switchDriverGroup = new SwitchDriverGroupBox(tr("Kernel Driver"));
	_switchDriverGroup->setVisible(false);

	_rightScroll = new QScrollArea();
	_rightScroll->setObjectName("rightScroll");
	_rightScroll->setStyleSheet(ic4::ui::CustomStyle.DeviceSelectionDlgRightScrollStyle);

	auto rightBox = new QFrame();
	rightBox->setObjectName("rightBox");
	rightBox->setStyleSheet(ic4::ui::CustomStyle.DeviceSelectionDlgRightBox);

	auto rightLayout = new QVBoxLayout();
	rightLayout->setContentsMargins(0, 0, 0, 0);

	rightLayout->addWidget(_itfInfoGroup, 0);
	rightLayout->addWidget(_switchDriverGroup, 0);
	rightLayout->addWidget(_devInfoGroup, 0);
	rightLayout->addWidget(_ipConfigGroup, 0);
	rightLayout->addStretch(1);

	rightBox->setLayout(rightLayout);

	_rightScroll->setWidget(rightBox);
	_rightScroll->setWidgetResizable(true);

	topLayout->addWidget(_rightScroll, 2);

	setLayout(topLayout);
}

void DeviceSelectionDialog::enumerateDevices()
{
	_cameraTree->clear();

	auto interfaces = ic4::DeviceEnum::enumInterfaces();

	for (auto&& itf : interfaces)
	{
		auto itf_devices = itf.enumDevices();

		std::vector<ic4::DeviceInfo> filtered_itf_devices;
		if (_filter_func)
		{
			std::copy_if(itf_devices.begin(), itf_devices.end(), std::back_inserter(filtered_itf_devices), _filter_func);
		}
		else
		{
			filtered_itf_devices = itf_devices;
		}

		if (filtered_itf_devices.empty())
			continue;

		ic4::Error err;

		auto map = itf.interfacePropertyMap(err);
		if (err.isError())
			continue;

		auto* itf_item = new QTreeWidgetItem(_cameraTree);
		itf_item->setText(0, QString::fromStdString(itf.interfaceDisplayName()));
		itf_item->setForeground(0, QPalette().windowText());
		itf_item->setData(0, ROLE_ITEM_DATA, QVariant::fromValue(InterfaceDeviceItemData{ itf, map }));
		itf_item->setFirstColumnSpanned(true);

		bool isGigEVisionInterface = itf.transportLayerType(ic4::Error::Ignore()) == ic4::TransportLayerType::GigEVision;

		for (auto&& dev : filtered_itf_devices)
		{
			QString strIPAddress;

			int index = std::distance(itf_devices.begin(), std::find(itf_devices.begin(), itf_devices.end(), dev));

			if (isGigEVisionInterface)
			{
				ic4::Error err;
				if (map.setValue("DeviceSelector", index, err))
				{
					auto ip = map.getValueInt64("GevDeviceIPAddress", err);
					if (err.isSuccess())
					{
						strIPAddress = QString("%1.%2.%3.%4")
							.arg((ip >> 24) & 0xFF)
							.arg((ip >> 16) & 0xFF)
							.arg((ip >> 8) & 0xFF)
							.arg((ip >> 0) & 0xFF);
					}
				}
			}

			auto device = dev.modelName(ic4::Error::Ignore());
			auto serial = dev.serial(ic4::Error::Ignore());
			auto deviceUserID = dev.userID(ic4::Error::Ignore());

			auto* node = new QTreeWidgetItem();

			auto variant = QVariant::fromValue(InterfaceDeviceItemData{ itf, map, dev, index });
			node->setData(0, ROLE_ITEM_DATA, variant);

			node->setText(0, QString::fromStdString(device));
			node->setText(1, QString::fromStdString(serial));
			node->setText(2, strIPAddress);
			node->setText(3, QString::fromStdString(deviceUserID));

			auto hasDeviceReachableStatus = map.findEnumeration("DeviceReachableStatus", ic4::Error::Ignore()).is_valid();

			if (!hasDeviceReachableStatus || map.getValueString("DeviceReachableStatus", ic4::Error::Ignore()) == "Reachable")
			{
				auto cam = ResourceSelector::instance().loadIcon(":/images/camera_icon_usb3.png");
				node->setIcon(0, cam);
			}
			else
			{
				auto warning = style()->standardIcon(QStyle::SP_MessageBoxWarning);
				node->setIcon(0, warning);
			}

			itf_item->addChild(node);
		}
	}

	if (interfaces.empty())
	{
		auto* err_item = new QTreeWidgetItem(_cameraTree);
		err_item->setText(0, "No interfaces found.");
		err_item->setForeground(0, QPalette().windowText());
		err_item->setFirstColumnSpanned(true);
		err_item->setDisabled(true);

		auto* err_sub = new QTreeWidgetItem(_cameraTree);
		err_sub->setText(0, "This is likely caused by no ic4 GenTL Producers being installed.");
		err_sub->setForeground(0, QPalette().windowText());
		err_sub->setFirstColumnSpanned(true);
		err_sub->setDisabled(true);
	}

	_cameraTree->expandAll();
}

static QString buildIPAddress(const ic4::PropertyMap& map, const char* ip_feature, const char* subnet_feature)
{
	ic4::Error err;

	auto ip = map.getValueString(ip_feature, err);
	if (err.isError())
		return {};

	auto subnet = map.getValueInt64(subnet_feature, err);
	if (err.isError())
		return {};

	int leadingOnes = qCountLeadingZeroBits(static_cast<uint32_t>(~subnet));
	int trailingZeros = qCountTrailingZeroBits(static_cast<uint32_t>(subnet));

	if (leadingOnes + trailingZeros == 32)
	{
		return QString("%1/%2")
			.arg(ip.c_str())
			.arg(leadingOnes);
	}
	else
	{
		auto subnetString = map.getValueString(subnet_feature, err);
		if (err.isError())
			return {};

		return QString("%1/%2")
			.arg(ip.c_str())
			.arg(subnetString.c_str());
	}
}

static QStringList buildInterfaceIPAddressList(const ic4::PropertyMap& map)
{
	ic4::Error err;
	auto selector = map.findInteger("GevInterfaceSubnetSelector", err);
	if (err.isError())
		return {};

	int64_t max = selector.maximum(ic4::Error::Ignore());
	QStringList result;

	for (int64_t i = 0; i <= max; ++i)
	{
		if (!selector.setValue(i, err))
			continue;

		auto addr = buildIPAddress(map, "GevInterfaceSubnetIPAddress", "GevInterfaceSubnetMask");
		if (addr.isEmpty())
			continue;

		result.push_back(addr);
	}

	return result;
}

static void synchronizeColumnWidths(std::vector<QFormLayout*> layouts)
{
	int maxWidth = 0;

	for (auto&& layout : layouts)
	{
		for (int i = 0; i < layout->rowCount(); ++i)
		{
			auto* labelItem = layout->itemAt(i, QFormLayout::ItemRole::LabelRole);
			if (labelItem)
			{
				auto* lbl = qobject_cast<QLabel*>(labelItem->widget());
				if (lbl)
				{
					auto width = lbl->fontMetrics().size(0, lbl->text()).width();
					maxWidth = std::max(maxWidth, width);
				}
			}
		}
	}

	for (auto&& layout : layouts)
	{
		for (int i = 0; i < layout->rowCount(); ++i)
		{
			auto* label = layout->itemAt(i, QFormLayout::ItemRole::LabelRole);
			if (label != nullptr)
			{
				label->widget()->setMinimumWidth(maxWidth);
			}
		}
	}
}

void DeviceSelectionDialog::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{	
	_rightScroll->hide();

	if (_okButton)
		_okButton->setEnabled(false);

	_itfInfoGroup->hide();
	_itfInfoGroup->clear();
	_devInfoGroup->hide();
	_devInfoGroup->clear();
	_ipConfigGroup->hide();
	_ipConfigGroup->clear();
	_switchDriverGroup->hide();
	_switchDriverGroup->clear();

	if (current == nullptr)
	{
		_rightScroll->show();
		return;
	}

	auto variant = current->data(0, ROLE_ITEM_DATA);
	auto itemData = variant.value<InterfaceDeviceItemData>();

	bool isGigEVisionInterface = itemData.itf.transportLayerType(ic4::Error::Ignore()) == ic4::TransportLayerType::GigEVision;
	bool isUSB3VisionInterface = itemData.itf.transportLayerType(ic4::Error::Ignore()) == ic4::TransportLayerType::USB3Vision;
	ic4::PropertyMap map = itemData.itfPropertyMap;

	if (itemData.isDevice())
	{
		if (map.setValue("DeviceSelector", itemData.deviceIndex, ic4::Error::Ignore()))
		{
			if (_okButton)
				_okButton->setEnabled(true);
		}
	}

	auto addStringItem = [](const char* label, const QString& value, QFormLayout& layout) -> QLineEdit*
		{
			auto edit = new QLineEdit(value);
			edit->setReadOnly(true);
			edit->setCursorPosition(0);
			layout.addRow(tr(label), edit);
			return edit;
		};

	auto buildStringItemIfExists = [&addStringItem](const ic4::PropertyMap& map, const char* prop_item, const char* label, QFormLayout& layout) -> QLineEdit*
		{
			ic4::Error err;
			auto value = map.getValueString(prop_item, err);
			if (err.isSuccess())
			{
				return addStringItem(label, QString::fromStdString(value), layout);
			}
			return nullptr;
		};

	_itfInfoGroup->show();

	buildStringItemIfExists(map, "InterfaceDisplayName", "Interface Name", *_itfInfoGroup->formLayout());

	if (isGigEVisionInterface)
	{
		auto interfaceIPAddresses = buildInterfaceIPAddressList(map);
		if (interfaceIPAddresses.count() == 1)
		{
			auto edit = new QLineEdit(interfaceIPAddresses.at(0));
			edit->setReadOnly(true);
			_itfInfoGroup->formLayout()->addRow(tr("IP Address"), edit);
		}
		else if (!interfaceIPAddresses.isEmpty())
		{
			auto txt = interfaceIPAddresses.join("\r\n");
			auto edit = new QPlainTextEdit(txt);
			edit->document()->setDocumentMargin(2);
			edit->setReadOnly(true);
			auto docmargin = edit->document()->documentMargin();
			auto margins = edit->contentsMargins();
			auto frameWidth = edit->frameWidth();
			auto fontHeight = edit->fontMetrics().height();
			edit->setFixedHeight(fontHeight * interfaceIPAddresses.count() + frameWidth * 2 + margins.top() + margins.bottom() + (int)docmargin * 2);
			edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
			_itfInfoGroup->formLayout()->addRow("IP Addresses", edit);
		}

		buildStringItemIfExists(map, "MaximumTransmissionUnit", "Maximum Transmission Unit", *_itfInfoGroup->formLayout());
	}

	auto itfTLName = itemData.itf.transportLayerName(ic4::Error::Ignore());
	if (!itfTLName.empty())
	{
		addStringItem("Driver Name", QString::fromStdString(itfTLName), *_itfInfoGroup->formLayout());
	}

	auto itfTLVersion = itemData.itf.transportLayerVersion(ic4::Error::Ignore());
	if (!itfTLVersion.empty())
	{
		addStringItem("Driver Version", QString::fromStdString(itfTLVersion), *_itfInfoGroup->formLayout());
	}

	if (itemData.isDevice())
	{
		_devInfoGroup->show();

		buildStringItemIfExists(map, "DeviceModelName", "Model Name", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceVendorName", "Vendor Name", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceSerialNumber", "Serial Number", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceVersion", "Device Version", *_devInfoGroup->formLayout());
		buildStringItemIfExists(map, "DeviceUserID", "Device User ID", *_devInfoGroup->formLayout());

		if (isGigEVisionInterface)
		{
			auto devIPAddress = buildIPAddress(map, "GevDeviceIPAddress", "GevDeviceSubnetMask");
			if (!devIPAddress.isEmpty())
				addStringItem("Device IP Address", devIPAddress, *_devInfoGroup->formLayout());

			buildStringItemIfExists(map, "GevDeviceGateway", "Device Gateway", *_devInfoGroup->formLayout());
			buildStringItemIfExists(map, "GevDeviceMACAddress", "Device MAC Address", *_devInfoGroup->formLayout());

			auto reachableStatus = map.getValueString("DeviceReachableStatus", ic4::Error::Ignore());
			if (reachableStatus.empty() || reachableStatus == "Reachable")
			{
				_ipConfigGroup->update(itemData.device);
			}
			else
			{
				_ipConfigGroup->updateUnreachable(map);

				if (_okButton)
					_okButton->setEnabled(false);
			}

			_ipConfigGroup->show();
		}
		else if (isUSB3VisionInterface)
		{
			auto reachableStatus = map.getValueString("DeviceReachableStatus", ic4::Error::Ignore());
			if (!reachableStatus.empty() && reachableStatus != "Reachable")
			{
				_switchDriverGroup->update(itemData.device);
				_switchDriverGroup->show();

				if (_okButton)
					_okButton->setEnabled(false);
			}
		}
	}

	synchronizeColumnWidths({ _devInfoGroup->formLayout(), _itfInfoGroup->formLayout(), _ipConfigGroup->formLayout()});

	_rightScroll->verticalScrollBar()->setValue(0);
	_rightScroll->show();
}

bool DeviceSelectionDialog::selectPreviousItem(QVariant itemVariant)
{
	auto itemData = itemVariant.value<InterfaceDeviceItemData>();

	for (int i = 0; i < _cameraTree->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* itfItem = _cameraTree->topLevelItem(i);

		if (!itemData.isDevice())
		{
			auto itfVariant = itfItem->data(0, ROLE_ITEM_DATA);
			if (itfVariant.value<InterfaceDeviceItemData>().itf == itemData.itf)
			{
				_cameraTree->setCurrentItem(itfItem);
				return true;
			}
		}
		else
		{
			for (int j = 0; j < itfItem->childCount(); ++j)
			{
				QTreeWidgetItem* devItem = itfItem->child(j);
				auto devVariant = devItem->data(0, ROLE_ITEM_DATA);

				if (devVariant.value<InterfaceDeviceItemData>().device == itemData.device)
				{
					_cameraTree->setCurrentItem(devItem);
					return true;
				}
			}
		}
	}

	return false;
}

static QString buildSystemInfoString()
{
	QString text;
	text += "<pre>";
	text += QString::fromStdString(ic4::getVersionInfo(ic4::VersionInfoFlags::Default, ic4::Error::Ignore()));
	text += "\n\n";

	ic4::Error err;
	auto interfaceList = ic4::DeviceEnum::enumInterfaces(err);
	if (!err)
	{
		text += "Detected Interfaces/Devices:\n";

		for (auto&& itf : interfaceList)
		{
			text += "    " + QString::fromStdString(itf.interfaceDisplayName(ic4::Error::Ignore())) + "\n";

			auto deviceList = itf.enumDevices(err);
			if (!err)
			{
				if (deviceList.empty())
				{
					text += "        (No devices found)\n";
				}
				else
				{
					for (auto&& dev : deviceList)
					{
						text += "        " + QString::fromStdString(dev.modelName(ic4::Error::Ignore()))
							+ " (" + QString::fromStdString(dev.serial(ic4::Error::Ignore())) + ", "
							+ QString::fromStdString(dev.version(ic4::Error::Ignore())) + ")\n";
					}
				}
			}
			else
			{
				text += "        (Failed to enumerate devices: " + QString::fromStdString(err.message()) + ")\n";
			}

			text += "\n";
		}
	}
	else
	{
		text += "    (Failed to enumerate device interfaces: " + QString::fromStdString(err.message()) + ")\n";
	}

	text += "</pre>";

	return text;
}

void DeviceSelectionDialog::onSystemInfoButton()
{
	QDialog infoDlg(this);
	infoDlg.setWindowTitle("System Info");
	infoDlg.setMinimumSize(640, 320);
	QVBoxLayout infoLayout;

	QTextEdit infoEdit;
	infoEdit.setReadOnly(true);
	infoEdit.setHtml(buildSystemInfoString());
	infoLayout.addWidget(&infoEdit);

	QHBoxLayout buttonsLayout;

	QPushButton copyButton(tr("Copy to Clipboard"));
	connect(&copyButton, &QPushButton::released, this,
		[&infoEdit, &copyButton]()
		{
			auto prev = infoEdit.textCursor();
			infoEdit.selectAll();
			infoEdit.copy();
			infoEdit.setTextCursor(prev);

			// change the button for 1 second
			// so that users know something happened
			copyButton.setText(tr("Copied!"));

			QTimer::singleShot(1000, &copyButton, [&copyButton]()
			{
				copyButton.setText(tr("Copy to Clipboard"));
			});
		});
	buttonsLayout.addWidget(&copyButton);

	QPushButton closeButton(tr("Close"));
	connect(&closeButton, &QPushButton::pressed, &infoDlg, &QDialog::close);
	buttonsLayout.addWidget(&closeButton);

	infoLayout.addLayout(&buttonsLayout);

	infoDlg.setLayout(&infoLayout);
	infoDlg.exec();
}

void DeviceSelectionDialog::onUpdateButton()
{
	QVariant previousData;

	auto* item = _cameraTree->currentItem();
	if (item != nullptr)
	{
		previousData = item->data(0, ROLE_ITEM_DATA);
	}

	enumerateDevices();

	if (previousData.isValid())
	{
		if (!selectPreviousItem(previousData))
		{
			if (_okButton)
				_okButton->setEnabled(false);
		}
	}
	else
	{
		_cameraTree->setCurrentItem(nullptr);

		if (_okButton)
			_okButton->setEnabled(false);
	}
}

void DeviceSelectionDialog::onOK()
{
	if (!_okButton->isEnabled())
		return;

	auto* item = _cameraTree->currentItem();
	if (item == nullptr)
		return;

	auto variant = item->data(0, ROLE_ITEM_DATA);
	auto itemData = variant.value<InterfaceDeviceItemData>();

	if (itemData.isDevice())
	{
		try
		{
			_grabber->deviceClose(ic4::Error::Throw());
			_grabber->deviceOpen(itemData.device, ic4::Error::Throw());
			accept();
		}
		catch (ic4::IC4Exception ex)
		{
			QMessageBox::critical(this, {}, ex.what());
		}
	}
}
