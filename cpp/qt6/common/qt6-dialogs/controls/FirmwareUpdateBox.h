
#pragma once

#include "FormGroupBox.h"

#include <ic4/ic4.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qprogressbar.h>

enum class FirmwareUpdateState
{
    Inactive,
    Active,
    Success,
    ErrorGeneric,
    ErrorUnsupportedFirmwarePackage,
    ErrorModelNotSupportedByPackage,
    ErrorWritingToDevice,
};

struct RegisteredNotification
{
    ic4::Property property;
    ic4::Property::NotificationToken token;
};

class FirmwareUpdateBox : public FormGroupBox
{
    Q_OBJECT

public:
    explicit FirmwareUpdateBox(const QString& title)
    : FormGroupBox(title)
    {}

public:
    void update(const ic4::DeviceInfo& deviceInfo);
    auto getCurrentState() const -> FirmwareUpdateState
    {
        return _state;
    };

signals:
    void state_changed(FirmwareUpdateState state);

private:
    ic4::DeviceInfo _deviceInfo;
    ic4::PropertyMap _itfPropertyMap;

    QLineEdit* _file_path = nullptr;
    QPushButton* _file_dialog_button = nullptr;
    QPushButton* _write_fw_button = nullptr;
    QProgressBar* _progress_bar = nullptr;
    QLabel* _status_label = nullptr;

    FirmwareUpdateState _state = FirmwareUpdateState::Inactive;
    QString _last_firmware;

private:

    void setStatusText(const QString& new_text);
    void setProgressBarValue(int new_value);

    void onWriteFWButtonPressed();
    void onOpenFileDialog();
    void onFilePathChanged(const QString&);
    void onModelOverride(QAction*);

    bool selectDeviceOnInterface();
    bool eventFilter(QObject *obj, QEvent *event);

    // cleanup related things
    std::vector<RegisteredNotification> _registeredNotifications;
    void saveRegisteredEventNotification(ic4::Property, ic4::Property::NotificationToken);
    void unregisterAllEventNotifications();
};
