

#include "FirmwareUpdateBox.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <regex>
#include <QtCore>
#include <qfilesystemmodel.h>
#include <QtCore>
#include <QCompleter>
#include <QMouseEvent>
#include <QMenu>
#include <QDir>
#include <QStyle>


static const char* DEFAULT_STATUS_MSG = "Select firmware file to start firmware update.";

// TODO: replace with ic4::PropId once available
namespace ic4examples::PropId
{
    static ic4::PropId::PropIdInteger DeviceSelector = { "DeviceSelector" };

    static ic4::PropId::PropIdCommand FirmwareUpdateUpload = { "FirmwareUpdateUpload" };
    static ic4::PropId::PropIdString FirmwarePackageModels = { "FirmwarePackageModels" };
    static ic4::PropId::PropIdInteger FirmwarePackageModelsSelector = { "FirmwarePackageModelsSelector" };
    static ic4::PropId::PropIdEnumeration DeviceReachableStatus = { "DeviceReachableStatus" };
    static ic4::PropId::PropIdString FirmwareUpdateProgress = { "FirmwareUpdateProgress" };
    static ic4::PropId::PropIdInteger FirmwareUpdateProgressPercent = { "FirmwareUpdateProgressPercent" };
    static ic4::PropId::PropIdEnumeration FirmwareUpdateState = { "FirmwareUpdateState" };
    static ic4::PropId::PropIdString FirmwareUpdateModelOverride = { "FirmwareUpdateModelOverride" };
    static ic4::PropId::PropIdString FirmwareUpdatePackageFile = { "FirmwareUpdatePackageFile" };
}

void FirmwareUpdateBox::update(const ic4::DeviceInfo& deviceInfo)
{
	bool disableAccessibilityCheck = (QGuiApplication::queryKeyboardModifiers() & Qt::KeyboardModifier::ShiftModifier);

	// Partially programmed devices might be not accessible using ic4; still allow firmware upgrade if necessary
	if (!disableAccessibilityCheck)
	{
		ic4::Grabber g;
		if (!g.deviceOpen(deviceInfo, ic4::Error::Ignore()))
		{
			auto* label = new QLabel("The device is currently not accessible for firmware updates.");
			label->setWordWrap(true);
			_layout->addRow(label);
			return;
		}
	}

    _deviceInfo = deviceInfo;
	auto itf = _deviceInfo.getInterface();
    _itfPropertyMap = itf.interfacePropertyMap();

    ic4::Error err;

    if (!selectDeviceOnInterface())
    {
        // TODO: communicate error
        return;
    }

    auto has_all_required_properties = [] (ic4::PropertyMap& itf) -> bool
    {
        const char* prop_names [] = {
            ic4examples::PropId::FirmwareUpdateUpload.prop_name,
            ic4examples::PropId::DeviceReachableStatus.prop_name,
            ic4examples::PropId::FirmwareUpdateProgress.prop_name,
            ic4examples::PropId::FirmwareUpdateProgressPercent.prop_name,
            ic4examples::PropId::FirmwareUpdateState.prop_name,
            ic4examples::PropId::FirmwareUpdateModelOverride.prop_name,
            ic4examples::PropId::FirmwarePackageModels.prop_name,
            ic4examples::PropId::FirmwarePackageModelsSelector.prop_name,
            ic4examples::PropId::FirmwareUpdatePackageFile.prop_name
		};

		return std::all_of(std::begin(prop_names), std::end(prop_names),
			[&itf](auto& name)
			{
				return itf.find(name, ic4::Error::Ignore()).is_valid();
			}
		);
    };

    if (!has_all_required_properties(_itfPropertyMap))
    {
		auto tltype = itf.transportLayerType(ic4::Error::Ignore());

        if (tltype == ic4::TransportLayerType::GigEVision || tltype == ic4::TransportLayerType::USB3Vision)
        {
            QLabel* label = new QLabel(this);
            label->setText("The installed version of the GenTL Producer does not support firmware updates. Install the current version to enable firmware upgrades: <a href=\"https://www.theimagingsource.com/en-us/support/download/\"></a>");
            label->setTextFormat(Qt::RichText);
            label->setTextInteractionFlags(Qt::TextBrowserInteraction);
            label->setOpenExternalLinks(true);
            _layout->addRow(label);
			return;
        }
        else
        {
            _layout->addRow(new QLabel(tr("This GenTL Producer does not support firmware updates.")));
			return;
        }
    }
    
	if (_itfPropertyMap.getValueString(ic4examples::PropId::DeviceReachableStatus, ic4::Error::Ignore()) != "Reachable")
    {
        _layout->addRow(new QLabel(tr("The device is currently not accessible for firmware updates.")));
		return;
    }

	if (_itfPropertyMap.getValueString(ic4examples::PropId::FirmwareUpdateState, ic4::Error::Ignore()) == "Success")
	{
		_layout->addRow(new QLabel(tr("Firmware update SUCCESSFUL. Please reconnect the device.")));
		return;
	}

    _status_label = new QLabel();
	_status_label->setWordWrap(true);
    _status_label->setText(tr(DEFAULT_STATUS_MSG));
	_layout->addRow(_status_label);

    _progress_bar = new QProgressBar();
    _progress_bar->setVisible(false);
    _layout->addRow(_progress_bar);

    _file_path = new ReturnFocusNextLineEdit("", this);

    // Add file path completion to QLineEdit
    auto model = new QFileSystemModel(this);
    auto completer = new QCompleter(model, this);
    auto home = QDir::homePath();
    model->setRootPath(home);
    _file_path->setCompleter(completer);

    connect(_file_path, &QLineEdit::textChanged,
            this, &FirmwareUpdateBox::onFilePathChanged);

	_file_dialog_button = new QPushButton("…", this);
	connect(_file_dialog_button, &QPushButton::pressed,
			this, &FirmwareUpdateBox::onOpenFileDialog);

    QHBoxLayout* file_layout = new QHBoxLayout();
    file_layout->addWidget(_file_path);
    file_layout->addWidget(_file_dialog_button);

    _layout->addRow(file_layout);

    _write_fw_button = new QPushButton("Upload Firmware", this);
    _write_fw_button->installEventFilter(this);
    _write_fw_button->setEnabled(false);

    connect(_write_fw_button, &QPushButton::pressed,
            this, &FirmwareUpdateBox::onWriteFWButtonPressed);

    _layout->addRow(_write_fw_button);

    // write this last so everything is triggered correctly
    // by onFilePathChanged
    if (!_last_firmware.isEmpty())
    {
        _file_path->setText(_last_firmware);
    }
}

void FirmwareUpdateBox::setStatusText(const QString& new_text)
{
    // setText should only be invoked from the main thread
    // invokeMethod ensures that we update in the correct thread
    QMetaObject::invokeMethod(_status_label, "setText",
                              Qt::QueuedConnection,
                              Q_ARG(QString, new_text));
}

void FirmwareUpdateBox::setProgressBarValue(int new_value)
{
    QMetaObject::invokeMethod(_progress_bar, "setValue",
                              Qt::QueuedConnection,
                              Q_ARG(int, new_value));
}

void FirmwareUpdateBox::onWriteFWButtonPressed()
{
    auto write_confirm = QMessageBox::question(this, tr("Confirm Firmware Update"),
                                                     tr("Really update firmware?\nAny interruption of this process may render the device unusable."),
                                                    QMessageBox::Yes, QMessageBox::Cancel);

    if (write_confirm != QMessageBox::Yes)
    {
        return;
    }

    _progress_bar->reset();

    auto cb_status = [this](const ic4::Property& prop)
    {
        ic4::Error err;
        std::string value = prop.asString().getValue(err);

        if (!value.empty())
        {
            setStatusText(QString::fromStdString(value));
        }
    };

    selectDeviceOnInterface();

    ic4::Error err;

    auto status_prop = _itfPropertyMap.find(ic4examples::PropId::FirmwareUpdateProgress, err);
    auto status_token = status_prop.eventAddNotification(cb_status, err);
    saveRegisteredEventNotification(status_prop, status_token);

    auto cb_percentage = [this](const ic4::Property& prop)
    {
        ic4::Error err;
        int value = prop.asInteger().getValue(err);

        setProgressBarValue(value);
    };

    auto percentage_prop = _itfPropertyMap.find(ic4examples::PropId::FirmwareUpdateProgressPercent, err);
    auto percentage_token = percentage_prop.eventAddNotification(cb_percentage, err);
    saveRegisteredEventNotification(percentage_prop, percentage_token);

    _progress_bar->setEnabled(true);
    _progress_bar->setVisible(true);
    _file_path->setEnabled(false);
	_file_dialog_button->setEnabled(false);
    _write_fw_button->setEnabled(false);

    auto state_cb = [this](ic4::Property& prop) -> void
    {
        auto value = prop.asEnumeration().getValue();

        if (value == "NotStarted")
        {
		}
        else if (value == "Running")
        {
            emit state_changed(FirmwareUpdateState::Active);
        }
		else if (QString::fromStdString(value).startsWith("Error"))
        {
			if (value == "ErrorUnsupportedFirmwarePackage")
			{
				emit state_changed(FirmwareUpdateState::ErrorUnsupportedFirmwarePackage);
			}
			else if (value == "ErrorModelNotSupportedByPackage")
			{
				emit state_changed(FirmwareUpdateState::ErrorModelNotSupportedByPackage);
			}
			else if (value == "ErrorWritingToDevice")
			{
				emit state_changed(FirmwareUpdateState::ErrorWritingToDevice);
			}
			else
			{
				emit state_changed(FirmwareUpdateState::ErrorGeneric);
			}

			unregisterAllEventNotifications();

			QMetaObject::invokeMethod(_file_path, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
			QMetaObject::invokeMethod(_file_dialog_button, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, true));
        }
		else if (QString::fromStdString(value).startsWith("Success"))
		{
			if (value == "Success")
			{
				emit state_changed(FirmwareUpdateState::Success);
			}

			unregisterAllEventNotifications();
		}
    };

    auto update_state = _itfPropertyMap.find(ic4examples::PropId::FirmwareUpdateState, err);
    auto state_token = update_state.eventAddNotification(state_cb, err);
    saveRegisteredEventNotification(update_state, state_token);
    
	if (!_itfPropertyMap.executeCommand(ic4examples::PropId::FirmwareUpdateUpload, err))
	{
		setStatusText(QString("Failed to start firmware update: %1").arg(QString::fromStdString(err.message())));
		unregisterAllEventNotifications();
	}

	// Make sure to reset override so that the next update can auto-select again
	_itfPropertyMap.setValue(ic4examples::PropId::FirmwareUpdateModelOverride, "", ic4::Error::Ignore());
}

void FirmwareUpdateBox::onOpenFileDialog()
{
    QString path;
    if (_last_firmware.isEmpty())
    {
        path = QDir::homePath();
    }
    else
    {
        path = QFileInfo(_last_firmware).dir().absolutePath();
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Firmware File"), 
                                                    path, tr("Firmware File - fwpack (*.fwpack)"));

    if (!fileName.isEmpty())
    {
        _file_path->setText(QDir::toNativeSeparators(fileName));
    }
}

void FirmwareUpdateBox::onFilePathChanged(const QString& new_file_path)
{
    if (new_file_path.endsWith(".fwpack") && QFileInfo::exists(new_file_path))
    {
        ic4::Error err;
        if (!_itfPropertyMap.setValue(ic4examples::PropId::FirmwareUpdatePackageFile, _file_path->text().toStdString().c_str(), err))
        {
            setStatusText("Firmware file is not usable. Select a different one.");
            return;
        }

        _last_firmware = new_file_path;
        // Reset status label in case we displayed an error message
        setStatusText(tr(DEFAULT_STATUS_MSG));
        _write_fw_button->setEnabled(true);
    }
    else
    {
        // Reset status label in case we displayed an error message
        setStatusText(tr(DEFAULT_STATUS_MSG));
        _write_fw_button->setEnabled(false);
    }
}

void FirmwareUpdateBox::onModelOverride(QAction* action)
{
    _itfPropertyMap.setValue(ic4examples::PropId::FirmwareUpdateModelOverride, action->text().toStdString().c_str(), ic4::Error::Ignore());
    // immediately start firmware update
    QMetaObject::invokeMethod(_write_fw_button, "click",
                              Qt::QueuedConnection);
}

bool FirmwareUpdateBox::selectDeviceOnInterface()
{
    ic4::Error err;
    // ensure we write to the correct device
	auto selector = _itfPropertyMap.find(ic4examples::PropId::DeviceSelector, err);
	if (!selector.is_valid())
		return false;

    for(int index = selector.minimum(); index <= selector.maximum(); index += selector.increment())
    {
		if (selector.setValue(index, err))
		{
			if (_itfPropertyMap.getValueString(ic4::PropId::DeviceSerialNumber, err) == _deviceInfo.serial())
			{
				return true;
			}
		}
    }

    return false;
}

bool FirmwareUpdateBox::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _write_fw_button && event->type() == QEvent::MouseButtonPress)
    {
         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

         if (mouseEvent->modifiers() == Qt::ShiftModifier)
         {
            ic4::Error err;

            QMenu* model_selection_menu = new QMenu();
            
			auto prop_model_selector = _itfPropertyMap.find(ic4examples::PropId::FirmwarePackageModelsSelector, err);

            for (int index = prop_model_selector.minimum(err);
                 index <= prop_model_selector.maximum(err);
                 index += prop_model_selector.increment(err))
            {
				if (prop_model_selector.setValue(index, err))
				{
					std::string modelName = _itfPropertyMap.getValueString(ic4examples::PropId::FirmwarePackageModels, err);
					if (err.isSuccess())
					{
						model_selection_menu->addAction(new QAction(QString::fromStdString(modelName)));
					}
				}
            }

            connect(model_selection_menu, &QMenu::triggered, this, &FirmwareUpdateBox::onModelOverride);

            model_selection_menu->popup(QWidget::mapToGlobal(_write_fw_button->pos()));

            return true;
         }
    }

    return QWidget::eventFilter(obj, event);
}

void FirmwareUpdateBox::saveRegisteredEventNotification(ic4::Property p, ic4::Property::NotificationToken t)
{
    _registeredNotifications.push_back({p, t});
}

void FirmwareUpdateBox::unregisterAllEventNotifications()
{
    for (auto&& t : _registeredNotifications)
    {
        t.property.eventRemoveNotification(t.token, ic4::Error::Ignore());
    }

    _registeredNotifications.clear();
}
