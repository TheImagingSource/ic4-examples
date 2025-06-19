// added { outline: none; } to QTreeView::item in QSS, to remove dottet line in WIN32

#pragma once

#include "PropertyControls.h"
#include "PropertyInfoBox.h"

#include <ic4/ic4.h>

#include <QString>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <QDockWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QTreeView>
#include <QHeaderView>
#include <QSplitter>
#include <QPainter>


#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>

#include <vector>
#include <memory>

namespace ic4::ui
{
	struct PropertyTreeNode
	{
	protected:
		PropertyTreeNode* parent_;

		ic4::Property prop_;
		ic4::PropType prop_type_;
		int row_;

		QString prop_name_;
		QString display_name_;
		std::vector<std::unique_ptr<PropertyTreeNode>> children_;
		ic4::Property::NotificationToken notification_token_ = {};
		bool prev_available_ = false;
	public:
		PropertyTreeNode(PropertyTreeNode* parent_, const ic4::Property& prop, ic4::PropType type, int row, QString pn, QString dn);
		~PropertyTreeNode();

		void populate();

		int num_children() const;

		PropertyTreeNode* child(int n) const;

		void register_notification_once(std::function<void(PropertyTreeNode*)> item_changed);

		bool is_category() const noexcept
		{
			return prop_type_ == ic4::PropType::Category;
		}

		auto prop() noexcept -> ic4::Property& { return prop_; }
		auto children() noexcept -> std::vector<std::unique_ptr<PropertyTreeNode>>& { return children_; }

		auto get_row() const noexcept { return row_; }
		auto get_parent() const noexcept { return parent_; }
		auto get_display_name() const noexcept { return display_name_; }
		auto get_prop_name() const noexcept { return prop_name_; }
	};

	class PropertyTreeModel : public QAbstractItemModel
	{
	private:
		PropertyTreeNode tree_root_;
		PropertyTreeNode* prop_root_ = nullptr;

	public:
		PropertyTreeModel(ic4::PropCategory root);
		~PropertyTreeModel() = default;

		QModelIndex rootIndex() const
		{
			return createIndex(0, 0, prop_root_);
		}
	private:
		const PropertyTreeNode* parent_item(const QModelIndex& parent) const;
	protected:
		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex& index) const override;
		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	};

	class PropertyTreeItemDelegate : public QStyledItemDelegate
	{
		QSortFilterProxyModel& proxy_;
		ic4::Grabber* grabber_;
		ic4::ui::StreamRestartFilterFunction restart_filter_;
		ic4::ui::PropSelectedFunction prop_selected_;
	public:
		PropertyTreeItemDelegate(QSortFilterProxyModel& proxy, ic4::Grabber* grabber, ic4::ui::StreamRestartFilterFunction restart_filter, ic4::ui::PropSelectedFunction prop_selected);

		void updateGrabber(ic4::Grabber* grabber)
		{
			grabber_ = grabber;
		}

		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& index) const override;
	};

	class TestItemDelegateForPaint : public QStyledItemDelegate
	{
		QSortFilterProxyModel& proxy_;
		QWidget* parent_ = nullptr;
	public:
		TestItemDelegateForPaint(QSortFilterProxyModel& proxy, QWidget* parent);
	
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	};


	class FilterPropertiesProxy : public QSortFilterProxyModel
	{
		QRegularExpression filter_regex_;
		ic4::PropVisibility visibility_ = ic4::PropVisibility::Expert;

		std::function<bool(const ic4::Property&)> filter_func_;
	public:
		FilterPropertiesProxy();

		void filter(QString text, ic4::PropVisibility vis);

		void filter_func(std::function<bool(const ic4::Property&)> accept_prop);

	protected:

		// don't show blue selection
		Qt::ItemFlags flags(const QModelIndex& index) const;

		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
	};

	class PropertyTreeView : public QTreeView
	{
		Q_OBJECT

		QSortFilterProxyModel& proxy_;
	public:
		PropertyTreeView(QSortFilterProxyModel& proxy);

	protected:

		void mousePressEvent(QMouseEvent* event) override;

		void drawBranches(QPainter* painter,
			const QRect& rect,
			const QModelIndex& index) const;

	};

	template<class T>
	class PropertyTreeWidgetBase : public T, public app::IViewBase
	{
		static_assert(std::is_base_of<QWidget, T>::value, "T must be derived from QWidget");

	public:
		struct Settings
		{
			bool showRootItem = false;
			bool showInfoBox = true;
			bool showFilter = true;

			QString initialFilter = "";
			ic4::PropVisibility initialVisibility = ic4::PropVisibility::Beginner;

			StreamRestartFilterFunction streamRestartFilter;

			static Settings Default() { return {}; };
		};

	protected:
		QComboBox* visibility_combo_ = nullptr;
		QLineEdit* filter_text_ = nullptr;

		PropertyInfoBox* info_text_ = nullptr;

		PropertyTreeView* view_ = nullptr;
		PropertyTreeModel* source_ = nullptr;
		FilterPropertiesProxy proxy_;

		PropertyTreeItemDelegate delegate_;
		TestItemDelegateForPaint branchPaintDelegate_;

		Settings settings_ = {};

	private:
		void update_visibility()
		{
			auto vis = static_cast<ic4::PropVisibility>(visibility_combo_->currentData().toInt());
			proxy_.filter(filter_text_->text(), vis);
		}

		void create_all_editors(QAbstractItemModel& model, QModelIndex parent)
		{
			int rows = model.rowCount(parent);

			for (int row = 0; row < rows; ++row)
			{
				auto index1 = model.index(row, 1, parent);
				view_->openPersistentEditor(index1);

				auto index0 = model.index(row, 0, parent);
				create_all_editors(model, index0);
			}
		}

		void propSelectionChanged(const QItemSelection& selected, const QItemSelection& /* deselected */)
		{
			if (!selected.isEmpty() && !selected.front().isEmpty())
			{
				auto item = selected.front().indexes().front();
				propSelected(item);
			}
			else
			{
				propSelected({});
			}
		}

		void propSelected(const QModelIndex& index)
		{
			if (!info_text_)
				return;

			auto source_index = proxy_.mapToSource(index);

			auto* tree = static_cast<PropertyTreeNode*>(source_index.internalPointer());
			if (!tree)
			{
				info_text_->clear();
				return;
			}

			info_text_->update(tree->prop());
		}

		void update_view()
		{
			if (!settings_.showRootItem && source_)
				view_->setRootIndex(proxy_.mapFromSource(source_->rootIndex()));

			create_all_editors(proxy_, {});
			view_->expandAll();

			bool source_available = source_ != nullptr;

			if (info_text_)
			{
				info_text_->setEnabled(source_available);
			}
			if (visibility_combo_)
			{
				visibility_combo_->setEnabled(source_available);
			}
			if (filter_text_)
			{
				filter_text_->setEnabled(source_available);
			}
			view_->setEnabled(source_available);

			if (source_available)
			{
				view_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
				view_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
				view_->header()->setStretchLastSection(true);
				view_->setItemDelegateForColumn(0, &branchPaintDelegate_);
				view_->setItemDelegateForColumn(1, &delegate_);
				view_->resizeColumnToContents(0);
			}
		}

		void proxyDataChanged(const QModelIndex& /* topLeft */, const QModelIndex& /* bottomRight */, const QVector<int>& /* roles = QVector<int>() */)
		{
			update_view();
		}

		void proxyLayoutChanged(const QList<QPersistentModelIndex>& /*parents = QList<QPersistentModelIndex>() */, QAbstractItemModel::LayoutChangeHint /* hint = QAbstractItemModel::NoLayoutChangeHint */)
		{
			update_view();
		}

	private:
		void updateModel(PropertyTreeModel* model)
		{
			auto* old_model = source_;

			source_ = model;
			proxy_.setSourceModel(source_);

			update_view();

			delete old_model;
		}

	public:
		void clearModel()
		{
			updateModel(nullptr);
		}
		void updateModel(ic4::PropCategory cat)
		{
			updateModel(new PropertyTreeModel(cat));
		}		
		void updateGrabber(ic4::Grabber* grabber)
		{
			ic4::Error err;
			auto propMap = grabber->devicePropertyMap(err);
			if (err.isSuccess())
			{
				auto cat = propMap.findCategory("Root", err);
				if (err.isSuccess())
				{
					delegate_.updateGrabber(grabber); // !
					updateModel(new PropertyTreeModel(cat));
				}
			}
		} 
		void setPropertyFilter(std::function<bool(const ic4::Property&)> accept_prop)
		{
			proxy_.filter_func(accept_prop);
		}

	public:
		PropertyTreeWidgetBase(ic4::PropCategory cat, ic4::Grabber* grabber, Settings settings = Settings::Default(), QWidget* parent = nullptr)
			: PropertyTreeWidgetBase(new PropertyTreeModel(cat), grabber, settings, parent)
		{
		}

		PropertyTreeWidgetBase(PropertyTreeModel* model, ic4::Grabber* grabber, Settings settings = Settings::Default(), QWidget* parent = nullptr)
			: T(parent)
			, source_(model)
			, delegate_(proxy_, grabber, settings.streamRestartFilter, [this](auto& p) { info_text_->update(p); })
			, branchPaintDelegate_(proxy_, this)
			, settings_(settings)
		{
			auto frame = new QFrame(this);
			auto* layout = new QVBoxLayout(frame);

			if (settings.showFilter)
			{
				auto* top = new QHBoxLayout;

				visibility_combo_ = new QComboBox;
				visibility_combo_->addItem("Beginner", (int)ic4::PropVisibility::Beginner);
				visibility_combo_->addItem("Expert", (int)ic4::PropVisibility::Expert);
				visibility_combo_->addItem("Guru", (int)ic4::PropVisibility::Guru);
				visibility_combo_->setCurrentIndex((int)settings.initialVisibility);
				visibility_combo_->setMinimumWidth(200);
				visibility_combo_->setStyleSheet("QComboBox {"
					"font-size: 13px;"
					"}");

				T::connect(visibility_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int)
					{
						update_visibility();
					});

				top->addWidget(visibility_combo_);

				filter_text_ = new QLineEdit;
				filter_text_->setStyleSheet("QLineEdit {"
					"font-size: 13px;"
					"}");
				filter_text_->setText(settings.initialFilter);
                filter_text_->setPlaceholderText("Search Properties (Ctrl-F)");
                filter_text_->setClearButtonEnabled(true);

                QShortcut *sC = new QShortcut(QKeySequence::Find, this);

                QObject::connect(sC,  &QShortcut::activated,
                                 [this] ()
                                 {
                                     this->filter_text_->setFocus(Qt::FocusReason::ShortcutFocusReason);
                                 });

				T::connect(filter_text_, &QLineEdit::textChanged, this, [this](const QString&) { update_visibility(); });
				top->addWidget(filter_text_);
	
				layout->addLayout(top);
				update_visibility();
			}

			view_ = new PropertyTreeView(proxy_);
			view_->setStyleSheet(CustomStyle.PropertyTreeViewStyle);
			proxy_.setSourceModel(source_);
			proxy_.filter(settings.initialFilter, settings.initialVisibility);

			view_->setModel(&proxy_);
			view_->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
			view_->header()->setHidden(true);
			view_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
			view_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
			view_->header()->setStretchLastSection(true);
			view_->setItemDelegateForColumn(0, &branchPaintDelegate_);
			view_->setItemDelegateForColumn(1, &delegate_);

			T::connect(view_, &QTreeView::clicked, this, &PropertyTreeWidgetBase::propSelected);
			T::connect(view_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertyTreeWidgetBase::propSelectionChanged);
			T::connect(&proxy_, &QSortFilterProxyModel::dataChanged, this, &PropertyTreeWidgetBase::proxyDataChanged);
			T::connect(&proxy_, &QSortFilterProxyModel::layoutChanged, this, &PropertyTreeWidgetBase::proxyLayoutChanged);

			if (settings_.showInfoBox)
			{
				info_text_ = new PropertyInfoBox(this);

				auto* splitter = new QSplitter(Qt::Orientation::Vertical, this);
				layout->addWidget(splitter);

				splitter->addWidget(view_);
				splitter->addWidget(info_text_);
				splitter->setStretchFactor(0, 3);
			}
			else
			{
				layout->addWidget(view_);
			}

			layout->setSpacing(0);
			layout->setContentsMargins(0, 0, 0, 0);

			frame->setLayout(layout);

			if constexpr (std::is_same_v<T, QDockWidget>) 
			{
				T::setWidget(frame);
			}
			else
			{
				T::setLayout(layout);
			}

			update_view();
		}

		~PropertyTreeWidgetBase()
		{
			delete source_;
		}

	public:
		void setPropVisibility(ic4::PropVisibility visibility)
		{
			visibility_combo_->setCurrentIndex(static_cast<int>(visibility));
			update_visibility();
		}
		void setFilterText(const QString& filterText)
		{
			filter_text_->setText(filterText);
			update_visibility();
		}
	};

	class PropertyTreeWidget : public PropertyTreeWidgetBase<QWidget>
	{
	public:
		PropertyTreeWidget(ic4::PropCategory cat, ic4::Grabber* grabber, Settings settings = Settings::Default(), QWidget* parent = nullptr);
		void closeEvent(QCloseEvent* event) override;
	};
}
