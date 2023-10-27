
#include <QGroupBox>

#include <ic4/ic4.h>

#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class IPConfigGroupBox : public QGroupBox
{
public:
	IPConfigGroupBox(const QString& title)
		: QGroupBox(title)
	{
		_layout = new QFormLayout();
		_layout->setContentsMargins(4, 4, 4, 4);
		setLayout(_layout);
	}

public:
	QFormLayout* formLayout() const
	{
		return _layout;
	}

public:
	void clear()
	{
		while (_layout->count() != 0)
		{
			QLayoutItem* forDeletion = _layout->takeAt(0);
			delete forDeletion->widget();
			delete forDeletion;
		}
	}

	void update(const ic4::DeviceInfo& deviceInfo);
	void updateUnreachable(ic4::PropertyMap itfPropertyMap);

private:
	ic4::DeviceInfo _deviceInfo;
	ic4::PropertyMap _itfPropertyMap;

	QFormLayout* _layout;

	QCheckBox* _chkPersistentIP;
	QCheckBox* _chkDHCP;
	QLineEdit* _persistentIPAddress;
	QLineEdit* _persistentSubnetMask;
	QLineEdit* _persistentDefaultGateway;
	QPushButton* _applyButton;

	QLineEdit* _forceIPAddress;
	QLineEdit* _forceSubnetMask;
	QLineEdit* _forceDefaultGateway;
	QPushButton* _forceButton;

private:
	void onStatusChangedCheckPersistentIP(int checkState);
	void onApplyButtonPressed();

	void onForceButtonPressed();
};