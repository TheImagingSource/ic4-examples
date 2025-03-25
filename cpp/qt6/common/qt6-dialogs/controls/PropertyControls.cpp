
#include "PropertyControls.h"

#include "props/PropIntControl.h"
#include "props/PropEnumerationControl.h"
#include "props/PropStringControl.h"
#include "props/PropCommandControl.h"
#include "props/PropBooleanControl.h"
#include "props/PropFloatControl.h"
#include "props/PropCategoryControl.h"


ic4::ui::CustomStyleDef ic4::ui::CustomStyle;


QWidget* ic4::ui::create_prop_control(const ic4::Property& prop, QWidget* parent, ic4::Grabber* grabber, StreamRestartFilterFunction func)
{
	switch (prop.type())
	{
	case ic4::PropType::Integer:
		return new ic4::ui::PropIntControl(prop.asInteger(), parent, grabber, func);
	case ic4::PropType::Command:
		return new ic4::ui::PropCommandControl(prop.asCommand(), parent, grabber, func);
	case ic4::PropType::String:
		return new ic4::ui::PropStringControl(prop.asString(), parent, grabber, func);
	case ic4::PropType::Enumeration:
		return new ic4::ui::PropEnumerationControl(prop.asEnumeration(), parent, grabber, func);
	case ic4::PropType::Boolean:
		return new ic4::ui::PropBooleanControl(prop.asBoolean(), parent, grabber, func);
	case ic4::PropType::Float:
		return new ic4::ui::PropFloatControl(prop.asFloat(), parent, grabber, func);
	case ic4::PropType::Category:
		return new ic4::ui::PropCategoryControl(prop.asCategory(), parent, func);
	default:
		return nullptr;
	}
}