
// PickList.h
//
// Copyright (c) 2015 Michael Imamura.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.

#pragma once

#include "../Exception.h"
#include "BaseContainer.h"
#include "StateButton.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Display {
		class Display;
	}
}

namespace HoverRace {
namespace Display {

/**
 * A single list item.
 * @author Michael Imamura
 * @see PickList
 */
class PickListItem : public StateButton
{
	using SUPER = StateButton;

public:
	PickListItem(Display &display, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~PickListItem() { }

private:
	void Init();
	void InitIcon(bool enabled, bool checked);
};

/**
 * Scrollable list of selectable items.
 * @tparam T The type of the values held by the list items.
 * @todo Scrolling.
 * @todo Filtering.
 * @author Michael Imamura
 */
template<class T>
class PickList : public BaseContainer
{
	using SUPER = BaseContainer;

public:
	PickList(Display &display, const Vec2 &size,
		uiLayoutFlags_t layoutFlags = 0) :
		SUPER(display, size, true, layoutFlags), selItem() { }
	virtual ~PickList() { }

protected:
	class DefaultItem : public PickListItem
	{
		using SUPER = PickListItem;

	public:
		DefaultItem(PickList<T> &pickList, Display &display,
			size_t idx, const T &value, const std::string &text,
			uiLayoutFlags_t layoutFlags = 0) :
			SUPER(display, text, layoutFlags),
			pickList(pickList), idx(idx), value(value)
		{
			clickedConn = GetClickedSignal().connect([&](ClickRegion&) {
				pickList.SetSelectedItem(this);
			});
		}
		DefaultItem(const DefaultItem&) = delete;
		DefaultItem(DefaultItem&&) = delete;

		virtual ~DefaultItem() { }

		DefaultItem &operator=(const DefaultItem&) = delete;
		DefaultItem &operator=(DefaultItem&&) = delete;

	public:
		size_t GetIndex() const { return idx; }
		const T &GetValue() const { return value; }

	private:
		PickList<T> &pickList;
		const size_t idx;
		const T value;
		boost::signals2::scoped_connection clickedConn;
	};

	/// Maps the container child wrapper to each item.
	class ItemChild
	{
	public:
		ItemChild(Child &child, DefaultItem &item) :
			child(child), item(item) { }

		ItemChild &operator=(const ItemChild&) = delete;

	public:
		Child &child;
		DefaultItem &item;
	};

private:
	void SetSelectedItem(DefaultItem *sel)
	{
		boost::optional<size_t> newSel = sel ?
			boost::make_optional(sel->GetIndex()) :
			boost::none;

		if (selItem != newSel) {
			if (selItem) {
				items[*selItem].item.SetChecked(false);
			}
			selItem = newSel;
			if (sel) {
				sel->SetChecked(true);
			}

			valueChangedSignal();
		}
	}

public:
	/**
	 * Add a list item with a specific label.
	 * @param label The label.
	 * @param value The value.
	 */
	void Add(const std::string &label, const T &value)
	{
		size_t idx = items.size();
		auto item = NewChild<DefaultItem>(*this, display, idx, value, label);
		items.emplace_back(*(GetChildren().back()), *item);
		filteredItems.push_back(items.size() - 1);
		RequestLayout();
	}

	/**
	 * Add a list item with a default label.
	 * @param value The value.
	 */
	void Add(const T &value)
	{
		Add(boost::lexical_cast<std::string>(value), value);
	}

	/**
	 * Set the selected value.
	 *
	 * If none of the items in this list match the value, then no item will be
	 * selected.
	 *
	 * @param val The value.
	 */
	void SetValue(const T &val)
	{
		// Find the first item matching this value.
		// We assume that the list is small enough that we don't need a
		// lookup table.
		boost::optional<size_t> newSel;
		for (auto idx : filteredItems) {
			if (items[idx].item.GetValue() == val) {
				newSel = boost::make_optional(idx);
				break;
			}
		}

		if (selItem != newSel) {
			if (selItem) {
				items[*selItem].item.SetChecked(false);
				selItem = newSel;
			}
			if (newSel) {
				items[*newSel].item.SetChecked(true);
			}
		}
	}

	/**
	 * Check if this list has a selected item.
	 * @return @c true if an item is selected, @c false if not.
	 */
	bool HasSelected() const
	{
		return selItem != nullptr;
	}

	/**
	 * Get the value of the selected item.
	 * @return The value.
	 */
	boost::optional<const T&> GetValue() const
	{
		return selItem ?
			boost::make_optional<const T&>(items[*selItem].item.GetValue()) :
			boost::none;
	}

	/**
	 * Apply a filter to the list items.
	 *
	 * This filter is one-time; any items added later will not have the filter
	 * applied.
	 *
	 * @param fn The filter function.  Takes the value and returns either
	 *           @c true if the item is visible, @c false otherwise.
	 */
	template<class Fn>
	void ApplyFilter(Fn fn)
	{
		filteredItems.clear();
		size_t idx = 0;
		for (auto &item : items) {
			bool visible = item.child.visible = fn(item.item.GetValue());
			if (visible) {
				filteredItems.push_back(idx);
			}
			else if (selItem && *selItem == idx) {
				selItem = boost::none;
			}
			idx++;
		}

		RequestLayout();
	}

	void Reserve(size_t capacity) override
	{
		SUPER::Reserve(capacity);
		items.reserve(capacity);
		filteredItems.reserve(capacity);
	}

public:
	using valueChangedSignal_t = boost::signals2::signal<void()>;
	valueChangedSignal_t &GetValueChangedSignal() { return valueChangedSignal; }

protected:
	void Layout() override
	{
		double cx = 0;
		double w = GetSize().x;

		ForEachVisibleChild([&](const std::shared_ptr<UiViewModel> &model) {
			auto item = static_cast<DefaultItem*>(model.get());

			auto size = item->Measure();
			item->SetSize(w, size.y);
			item->SetPos(0, cx);

			cx += size.y;
		});
	}

private:
	std::vector<ItemChild> items;
	std::vector<size_t> filteredItems;  ///< Indexes of filtered items.
	boost::optional<size_t> selItem;  ///< Index of selected item.
	valueChangedSignal_t valueChangedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
