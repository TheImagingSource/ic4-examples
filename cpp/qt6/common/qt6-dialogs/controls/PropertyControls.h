
#pragma once

#include <ic4/ic4.h>

#include <QWidget>

namespace ic4::ui
{
	QWidget* create_prop_control(const ic4::Property& prop, QWidget* parent);
}