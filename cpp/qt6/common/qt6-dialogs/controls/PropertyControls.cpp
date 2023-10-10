
#include "PropertyControls.h"

#include "props/PropIntControl.h"
#include "props/PropEnumerationControl.h"
#include "props/PropStringControl.h"
#include "props/PropCommandControl.h"
#include "props/PropBooleanControl.h"
#include "props/PropFloatControl.h"
#include "props/PropCategoryControl.h"



QWidget* ic4::ui::create_prop_control(const ic4::Property& prop, QWidget* parent)
{
	switch (prop.getType())
	{
	case ic4::PropType::Integer:
		return new ic4::ui::PropIntControl(prop.asInteger(), parent);
	case ic4::PropType::Command:
		return new ic4::ui::PropCommandControl(prop.asCommand(), parent);
	case ic4::PropType::String:
		return new ic4::ui::PropStringControl(prop.asString(), parent);
	case ic4::PropType::Enumeration:
		return new ic4::ui::PropEnumerationControl(prop.asEnumeration(), parent);
	case ic4::PropType::Boolean:
		return new ic4::ui::PropBooleanControl(prop.asBoolean(), parent);
	case ic4::PropType::Float:
		return new ic4::ui::PropFloatControl(prop.asFloat(), parent);
	case ic4::PropType::Category:
		return new ic4::ui::PropCategoryControl(prop.asCategory(), parent);
	default:
		return nullptr;
	}
}