
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
 * Generic base for PickList.
 * @author Michael
 */
class BasePickList : public BaseContainer
{
	using SUPER = BaseContainer;

public:
	BasePickList(Display &display, const Vec2 &size,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~BasePickList() { }

protected:
	/**
	 * Get the focused child widget.
	 * @return The focused child, or @c nullptr if nothing is focused.
	 */
	virtual UiViewModel *GetFocusedChild() const = 0;

	/**
	 * Search for the index of the given child widget.
	 * @param child The child widget.
	 * @return The found child index, if the widget was found.
	 */
	virtual boost::optional<size_t> FindChildIndex(const UiViewModel &child) const = 0;

public:
	bool OnAction() override;
	bool OnNavigate(const Control::Nav &nav) override;

protected:
	void OnChildRequestedFocus(UiViewModel &child) override;
	void OnChildRelinquishedFocus(UiViewModel&,
		const Control::Nav &nav) override;

public:
	bool TryFocus(const Control::Nav &nav = Control::Nav::NEUTRAL) override;
	void DropFocus() override;

public:
	using valueChangedSignal_t = boost::signals2::signal<void()>;
	valueChangedSignal_t &GetValueChangedSignal() { return valueChangedSignal; }

protected:
	void Layout() override;

protected:
	std::vector<size_t> filteredItems;  ///< Indexes of filtered items.
	boost::optional<size_t> selItem;  ///< items index of selected.
	boost::optional<size_t> focusedItem;  ///< filteredItems index of focused.
	double listHeight;
	valueChangedSignal_t valueChangedSignal;
};

/**
 * Scrollable list of selectable items.
 * @tparam T The type of the values held by the list items.
 * @todo Scrolling.
 * @author Michael Imamura
 */
template<class T>
class PickList : public BasePickList
{
	using SUPER = BasePickList;

public:
	PickList(Display &display, const Vec2 &size,
		uiLayoutFlags_t layoutFlags = 0) :
		SUPER(display, size, layoutFlags) { }
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

protected:
	UiViewModel *GetFocusedChild() const override
	{
		return focusedItem ?
			items[filteredItems[*focusedItem]].child.child.get() :
			nullptr;
	}

	boost::optional<size_t> FindChildIndex(const UiViewModel &child) const override
	{
		// Currently using a simple linear search.
		// If this gets prohibitive later due to the number of child elements,
		// we'll need to rethink this.

		size_t fi = 0;
		for (auto i : filteredItems) {
			if (items[i].child.child.get() == &child) {
				return fi;
			}
			fi++;
		}

		return boost::none;
	}

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

private:
	void SetSelection(const boost::optional<size_t> &newSel)
	{
		if (selItem != newSel) {
			if (selItem) {
				items[*selItem].item.SetChecked(false);
			}
			if (newSel) {
				items[*newSel].item.SetChecked(true);
			}
			selItem = newSel;
			valueChangedSignal();
		}
	}

public:
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
				newSel = idx;
				break;
			}
		}

		SetSelection(newSel);
	}

	/**
	 * Select an item by index.
	 *
	 * If @p idx is out of range of the current list, then nothing will be
	 * selected.
	 *
	 * @param idx The index of the current (filtered) list.
	 */
	void SetIndex(size_t idx)
	{
		boost::optional<size_t> newSel;
		if (idx < filteredItems.size()) {
			newSel = filteredItems[idx];
		}

		SetSelection(newSel);
	}

	/**
	 * Check if this list has a selected item.
	 * @return @c true if an item is selected, @c false if not.
	 */
	bool HasSelected() const
	{
		return selItem;
	}

	/**
	 * If an item is selected, unset the selection so that nothing is selected.
	 */
	void ClearSelection()
	{
		if (selItem) {
			items[*selItem].item.SetChecked(false);
			selItem = boost::none;

			valueChangedSignal();
		}
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
	 * This filter is is applied immediately; it does not affect items added
	 * after this call is made.  It is up to the owner of this list to
	 * re-apply the filter whenever an item added.
	 *
	 * @param fn The filter function.  Takes the value and returns either
	 *           @c true if the item is visible, @c false otherwise.
	 */
	template<class Fn>
	void ApplyFilter(Fn fn)
	{
		auto prevFocusedChild = GetFocusedChild();
		auto prevFocusedIdx = focusedItem;

		bool deselected = false;
		bool foundFocusedChild = false;
		filteredItems.clear();
		size_t idx = 0;
		for (auto &item : items) {
			bool visible = item.child.visible = fn(item.item.GetValue());
			if (visible) {
				if (item.child.child.get() == prevFocusedChild) {
					// Found the previously-focused child widget in the
					// new filtered list; just update the index (the widget
					// was already focused, so no need to TryFocus() it).
					foundFocusedChild = true;
					focusedItem = filteredItems.size();
				}
				filteredItems.push_back(idx);
			}
			else if (selItem && *selItem == idx) {
				items[*selItem].item.SetChecked(false);
				selItem = boost::none;
				deselected = true;
			}
			idx++;
		}

		if (prevFocusedChild && !foundFocusedChild) {
			prevFocusedChild->DropFocus();
			focusedItem = boost::none;

			// An item was previously focused, but is no longer included
			// in the filtered list.
			if (filteredItems.empty()) {
				RelinquishFocus(Control::Nav::NEUTRAL);
			}
			else {
				// Move focus to another item.
				focusedItem = std::min(*focusedItem, filteredItems.size() - 1);
				if (!GetFocusedChild()->TryFocus()) {
					// Should never happen, but just in case...
					focusedItem = boost::none;
					SetFocused(false);
				}
			}
		}

		RequestLayout();

		if (deselected) {
			valueChangedSignal();
		}
	}

	/**
	 * Undo any applied filter (so all items are visible).
	 */
	void RemoveFilter()
	{
		ApplyFilter([](const T&){ return true; });
	}

	void Clear() override
	{
		if (focusedItem) {
			RelinquishFocus(Control::Nav::NEUTRAL);
		}

		filteredItems.clear();
		items.clear();

		SUPER::Clear();

		if (selItem) {
			// The selected item was removed, so no need to uncheck it :)
			selItem = boost::none;
			valueChangedSignal();
		}
	}

	void Reserve(size_t capacity) override
	{
		SUPER::Reserve(capacity);
		items.reserve(capacity);
		filteredItems.reserve(capacity);
	}

private:
	std::vector<ItemChild> items;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
