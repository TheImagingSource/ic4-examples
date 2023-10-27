
#include <QGroupBox>

#include <ic4/ic4.h>

#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>

class FormGroupBox : public QFrame
{
public:
	FormGroupBox(const QString& title)
	{
		auto vbox = new QVBoxLayout();
		vbox->setContentsMargins(0, 0, 0, 8);

		_layout = new QFormLayout();
		_layout->setContentsMargins(7, 0, 7, 0);
		_layout->setLabelAlignment(Qt::AlignRight);

		auto label = new QLabel(title);
		label->setStyleSheet("QLabel { margin: 0px; background-color: palette(base); padding: 4px }");

		vbox->addWidget(label);
		vbox->addLayout(_layout);

		setLayout(vbox);
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

protected:
	QFormLayout* _layout;
};

class IPConfigGroupBox : public FormGroupBox
{
public:
	IPConfigGroupBox(const QString& title)
		: FormGroupBox(title)
	{
	}

public:
	void update(const ic4::DeviceInfo& deviceInfo);
	void updateUnreachable(ic4::PropertyMap itfPropertyMap);

private:
	ic4::DeviceInfo _deviceInfo;
	ic4::PropertyMap _itfPropertyMap;

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