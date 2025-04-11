
#pragma once

#include "props/PropControlBase.h"

#include <ic4/ic4.h>

#include <QWidget>

namespace ic4::ui
{
	QWidget* create_prop_control(const ic4::Property& prop, QWidget* parent, ic4::Grabber* grabber, StreamRestartFilterFunction func, PropSelectedFunction propSelected);


	struct CustomStyleDef
	{
		bool PropertyTreeViewBranchBackgroundEnabled = false;
		QColor PropertyTreeViewBranchBackground = QColor(0xFFFF0000);

		bool PropertyTreeViewBranchTextColorEnabled = false;
		QColor PropertyTreeViewBranchTextColor = QColor(0xFFdcdcdc);

		QString PropertyTreeViewStyle = "QTreeView::branch, QTreeView::item, QTreeView { "
			"outline: none; "
			"show-decoration-selected: 0;"
			"color: palette(text);"
			"background: palette(window);" // needs to be set,
			// in order that dottet line is not visible?!
			"font-size: 13px;"
			"}"
			"QTreeView::branch:open:adjoins-item:has-children{"
			"background: transparent;" // needs to be transparent, 
			//in order that QTReeView::drawBranches works?!
			"margin : 0;"
			" }"
			"QTreeView::branch:closed:adjoins-item:has-children{"
			"background: transparent;"// needs to be transparent, 
			//in order that QTReeView::drawBranches works?!
			"margin : 0;"
			" }";

		QString PropCategoryControlStyle = "QWidget { "
			"background-color: palette(mid);"
			"}";

		QString PropCategoryControlLabelStyle = "QWidget { "
			"background-color: palette(mid);"
			"}";

		bool ItemDelegateForPaintTextColorEnabled = false;
		QColor ItemDelegateForPaintTextColor = QColor(0xFFffffff);

		bool ItemDelegateForPaintBackgroundColorEnabled = false;
		QColor ItemDelegateForPaintBackgroundColor = QColor(0x252526);

		QString FormGroupBoxStyle = "QLabel {"
			"margin: 0px; background-color: palette(base); padding: 4px }";


		QString IPConfigGroupBoxLineEditBackgroundDefault = "background-color: palette(base)";
		QString IPConfigGroupBoxLineEditBackgroundDark = "background-color: darkred";
		QString IPConfigGroupBoxLineEditBackgroundLight = "background-color: #FF4040";

		QString IPConfigGroupBoxUnreachableFrame = "QFrame#WarningFrame { "
			"border: 1px solid red; background-color: palette(base); color: red; padding: 4px }";

		QString DeviceSelectionDlgRightScrollStyle = "QScrollArea#rightScroll "
			"{ border-width: 1; border-style: solid; border-color: palette(base); }";

		QString DeviceSelectionDlgRightBox =
			"QFrame#rightBox { padding: 0px; }\n"
			"QLineEdit[readOnly=\"true\"] { background: palette(window) }\n"
			"QPlainTextEdit[readOnly=\"true\"] { background: palette(window) }\n";

		QString DeviceSelectionDlgCameraTreeStyle = "QTreeView::item { padding: 4px; } ";

		bool DeviceSelectionDlgSizeGripEnabled = true;

		bool DeviceSelectionDlgResizeEnabled = false;

		int DeviceSelectionDlgMinWidth = 900;

		int DeviceSelectionDlgMinHeight = 400;


	}; 
	extern CustomStyleDef CustomStyle;
}