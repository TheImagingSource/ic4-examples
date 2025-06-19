
#include "PropertyTreeWidget.h"

ic4::ui::PropertyTreeNode::PropertyTreeNode(PropertyTreeNode* parent, const ic4::Property& prop_, ic4::PropType type, int row, QString pn, QString dn)
: parent_(parent)
, prop_(prop_)
, prop_type_(type)
, row_(row)
, prop_name_(std::move(pn))
, display_name_(std::move(dn))
{
}

ic4::ui::PropertyTreeNode::~PropertyTreeNode()
{
	if (notification_token_)
		prop_.eventRemoveNotification(notification_token_, ic4::Error::Ignore());
}

void ic4::ui::PropertyTreeNode::populate()
{
	if (!children_.empty())
		return;

	if (prop_.type(ic4::Error::Ignore()) == ic4::PropType::Category)
	{
		int index = 0;
		for (auto&& feature : prop_.asCategory().features())
		{
			auto child_name = QString::fromStdString(feature.name());
			auto child_display_name = QString::fromStdString(feature.displayName());

			auto tmp_prop_type = feature.type(ic4::Error::Ignore());

			switch (tmp_prop_type)
			{
				// only show valid properties
			case ic4::PropType::Integer:
			case ic4::PropType::Command:
			case ic4::PropType::String:
			case ic4::PropType::Enumeration:
			case ic4::PropType::Boolean:
			case ic4::PropType::Float:
			case ic4::PropType::Category:
				children_.push_back(std::make_unique<PropertyTreeNode>(this, feature, tmp_prop_type, index++, child_name, child_display_name));
				break;
			default:
				break;
			}
		}
	}
}

int ic4::ui::PropertyTreeNode::num_children() const
{
	const_cast<PropertyTreeNode*>(this)->populate();

	return static_cast<int>(children_.size());
}

ic4::ui::PropertyTreeNode* ic4::ui::PropertyTreeNode::child(int n) const
{
	const_cast<PropertyTreeNode*>(this)->populate();

	if (n < static_cast<int>(children_.size()))
		return children_[n].get();

	return nullptr;
}

void ic4::ui::PropertyTreeNode::register_notification_once(std::function<void(PropertyTreeNode*)> item_changed)
{
	if (notification_token_)
		return;

	prev_available_ = prop_.isAvailable(ic4::Error::Ignore());

	notification_token_ = prop_.eventAddNotification(
		[this, item_changed](ic4::Property& local_prop)
		{
			bool new_available = local_prop.isAvailable(ic4::Error::Ignore());
			if (prev_available_ != new_available)
			{
				item_changed(this);
				prev_available_ = new_available;
			}
		},
		ic4::Error::Ignore()
	);
}

ic4::ui::PropertyTreeModel::PropertyTreeModel(ic4::PropCategory root) : QAbstractItemModel(Q_NULLPTR)
, tree_root_(nullptr, root, ic4::PropType::Category, 0, "", "")
{
	// Add extra layer to make sure root element has a parent
	// QSortFilterProxyModel::filterAcceptsRow works with parent index
	auto root_name = root.name(ic4::Error::Ignore());
	auto root_display_name = root.displayName(ic4::Error::Ignore());
	tree_root_.children().push_back(std::make_unique<PropertyTreeNode>(&tree_root_, root, ic4::PropType::Category, 0, QString::fromStdString(root_name), QString::fromStdString(root_display_name)));
	prop_root_ = tree_root_.children().front().get();
}

const ic4::ui::PropertyTreeNode* ic4::ui::PropertyTreeModel::parent_item(const QModelIndex& parent) const
{
	if (!parent.isValid())
		return &tree_root_;

	return static_cast<const PropertyTreeNode*>(parent.internalPointer());
}

QModelIndex ic4::ui::PropertyTreeModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	auto* parentItem = parent_item(parent);

	auto* childItem = parentItem->child(row);
	if (childItem)
	{
		childItem->register_notification_once(
			[this](PropertyTreeNode* item)
			{
				auto item_index = createIndex(item->get_row(), 0, item);

				const_cast<PropertyTreeModel*>(this)->dataChanged(item_index, item_index);
			}
		);

		return createIndex(row, column, childItem);
	}

	return QModelIndex();
}

QModelIndex ic4::ui::PropertyTreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	auto* childItem = static_cast<PropertyTreeNode*>(index.internalPointer());
	auto* parentItem = childItem->get_parent();

	if (parentItem == &tree_root_)
		return QModelIndex();

	return createIndex(parentItem->get_row(), 0, parentItem);
}

int ic4::ui::PropertyTreeModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	if (parent.column() > 0)
		return 0;

	auto* parentItem = parent_item(parent);

	return parentItem->num_children();
}

int ic4::ui::PropertyTreeModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant ic4::ui::PropertyTreeModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	if (!index.isValid())
		return QVariant();

	auto* tree = static_cast<PropertyTreeNode*>(index.internalPointer());
	switch (role)
	{
	case Qt::TextAlignmentRole:
		if (tree->children().size() == 0)
			return  static_cast<Qt::Alignment::Int>(Qt::AlignVCenter | Qt::AlignRight);
		else
			return  static_cast<Qt::Alignment::Int>(Qt::AlignVCenter | Qt::AlignLeft);
		break;
	case Qt::DisplayRole:
		if (index.column() == 0)
			return tree->get_display_name();
		return {};
	case Qt::ToolTipRole:
	{
		auto tt = tree->prop().tooltip();
		auto desc = tree->prop().description();

		if (!tt.empty()) return QString::fromStdString(tt);
		if (!desc.empty()) return QString::fromStdString(desc);
		return tree->get_display_name();
	}
	default:
		return {};
	}
}

ic4::ui::PropertyTreeItemDelegate::PropertyTreeItemDelegate(QSortFilterProxyModel& proxy, ic4::Grabber* grabber, ic4::ui::StreamRestartFilterFunction restart_filter, ic4::ui::PropSelectedFunction prop_selected) : proxy_(proxy)
, grabber_(grabber)
, restart_filter_(restart_filter)
, prop_selected_(prop_selected)
{

}

QWidget* ic4::ui::PropertyTreeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& index) const
{
	auto source_index = proxy_.mapToSource(index);

	auto* tree = static_cast<PropertyTreeNode*>(source_index.internalPointer());

	if (!tree)
	{
		return nullptr;
	}

	auto* widget = create_prop_control(tree->prop(), parent, grabber_, restart_filter_, prop_selected_);
	if (widget)
	{
		widget->setContentsMargins(0, 0, 8, 0);
	}

	return widget;
}

ic4::ui::TestItemDelegateForPaint::TestItemDelegateForPaint(QSortFilterProxyModel& proxy, QWidget* parent) : proxy_(proxy), parent_(parent)
{

}

void ic4::ui::TestItemDelegateForPaint::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{

	auto source_index = proxy_.mapToSource(index);
	auto* tree = static_cast<PropertyTreeNode*>(source_index.internalPointer());

	if (tree->children().size() > 0) {
		// This paints the left half of the category title and background
		painter->save();
		if (CustomStyle.ItemDelegateForPaintTextColorEnabled)
		{
			painter->setPen(CustomStyle.ItemDelegateForPaintTextColor);
		}
		else
		{
			painter->setPen(parent_->palette().color(QPalette::Text));
		}
		auto r = option.rect;
		if (CustomStyle.ItemDelegateForPaintBackgroundColorEnabled)
		{
			painter->fillRect(r, QBrush(CustomStyle.ItemDelegateForPaintBackgroundColor));
		}
		else
		{
			painter->fillRect(r, QBrush(parent_->palette().color(QPalette::Mid)));
		}
		painter->drawText(r, option.displayAlignment, index.data().toString());
		painter->restore();
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);

		/*
		painter->save();
		painter->setPen(parent_->palette().color(QPalette::Text));
		auto r = option.rect;
		painter->fillRect(r, QBrush(parent_->palette().color(QPalette::Background)));
		painter->drawText(r, option.displayAlignment, index.data().toString());
		painter->restore();
		*/
	}
}

ic4::ui::FilterPropertiesProxy::FilterPropertiesProxy()
{
	setRecursiveFilteringEnabled(true);
}

void ic4::ui::FilterPropertiesProxy::filter(QString text, ic4::PropVisibility vis)
{
	filter_regex_ = QRegularExpression(text, QRegularExpression::PatternOption::CaseInsensitiveOption);
	visibility_ = vis;
	invalidate();
}

void ic4::ui::FilterPropertiesProxy::filter_func(std::function<bool(const ic4::Property&)> accept_prop)
{
	filter_func_ = accept_prop;
	invalidate();
}

Qt::ItemFlags ic4::ui::FilterPropertiesProxy::flags(const QModelIndex& index) const
{
	const auto flags = QSortFilterProxyModel::flags(index);
	return flags & ~Qt::ItemIsSelectable;
}

bool ic4::ui::FilterPropertiesProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	auto* tree = static_cast<PropertyTreeNode*>(sourceParent.internalPointer());
	if (!tree)
		return false;

	auto& child = *tree->children()[sourceRow];

	// Set all categories as hidden.
	// Qt will still show them if they have visible children.
	if (child.is_category())
		return false;

	if (!child.prop().isAvailable())
		return false;

	if (child.prop().visibility() > visibility_)
		return false;

	if (!filter_regex_.match(child.get_display_name()).hasMatch() && !filter_regex_.match(child.get_prop_name()).hasMatch())
		return false;

	if (!filter_func_)
		return true;

	return filter_func_(child.prop());
}

ic4::ui::PropertyTreeView::PropertyTreeView(QSortFilterProxyModel& proxy) : proxy_(proxy)
{

}

void ic4::ui::PropertyTreeView::mousePressEvent(QMouseEvent* event)
{
	QModelIndex index = indexAt(event->pos());
	bool last_state = isExpanded(index);
	QTreeView::mousePressEvent(event);
	if (index.isValid() && last_state == isExpanded(index))
		setExpanded(index, !last_state);
}

void ic4::ui::PropertyTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
	auto source_index = proxy_.mapToSource(index);
	auto* tree = static_cast<PropertyTreeNode*>(source_index.internalPointer());

	if (tree->children().size() > 0)
	{
		if (CustomStyle.PropertyTreeViewBranchBackgroundEnabled)
		{
			painter->fillRect(rect, CustomStyle.PropertyTreeViewBranchBackground);
		}
		else
		{
			painter->fillRect(rect, QWidget::palette().color(QPalette::Mid));
		}

		// 
		int offset = (rect.width() - indentation()) / 2;
		auto c = CustomStyle.PropertyTreeViewBranchTextColorEnabled ?
			CustomStyle.PropertyTreeViewBranchTextColor : QWidget::palette().color(QPalette::Text);
		int x = rect.x() + rect.width() / 2 + offset;
		int y = rect.y() + rect.height() / 2;
		int length = 9;

		if (isExpanded(index))
		{
			x = x - 5;
			y = y - 2;
			for (int i = 0; i < 5; ++i)
			{
				QRect arrow(x + i, y + i, length - (i * 2), 1);
				painter->fillRect(arrow, c);
			}
		}
		else
		{
			x = x - 2;
			y = y - 5;
			for (int i = 0; i < 5; ++i)
			{
				QRect arrow(x + i, y + i, 1, length - (i * 2));
				painter->fillRect(arrow, c);
			}
		}
	}
	else
	{
		QTreeView::drawBranches(painter, rect, index);
	}
}

ic4::ui::PropertyTreeWidget::PropertyTreeWidget(ic4::PropCategory cat, ic4::Grabber* grabber, Settings settings /*= Settings::Default()*/, QWidget* parent /*= nullptr*/) :
	PropertyTreeWidgetBase(cat, grabber, settings, parent)
{

}

void ic4::ui::PropertyTreeWidget::closeEvent(QCloseEvent* event)
{
	clearModel();
	onClose((app::IViewBase*)this);
	QWidget::closeEvent(event);
}
