
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
		SUPER(display, size, true, layoutFlags), selItem(nullptr) { }
	virtual ~PickList() { }

protected:
	class DefaultItem : public PickListItem
	{
		using SUPER = PickListItem;

	public:
		DefaultItem(PickList<T> &pickList, Display &display,
			const T &value, const std::string &text,
			uiLayoutFlags_t layoutFlags = 0) :
			SUPER(display, text, layoutFlags),
			pickList(pickList), value(value)
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
		const T &GetValue() const { return value; }

	private:
		PickList<T> &pickList;
		T value;
		boost::signals2::scoped_connection clickedConn;
	};

private:
	void SetSelectedItem(DefaultItem *sel)
	{
		if (selItem != sel) {
			if (selItem) {
				selItem->SetChecked(false);
			}
			selItem = sel;
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
		//TODO: Check if matches current filter.
		auto item = NewChild<DefaultItem>(*this, display, value, label);
		items.push_back(item);
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
		DefaultItem *newSel = nullptr;
		for (const auto &item : items) {
			if (item->GetValue() == val) {
				newSel = item.get();
				break;
			}
		}

		if (selItem != newSel) {
			if (selItem) {
				selItem->SetChecked(false);
				selItem = newSel;
			}
			if (newSel) {
				newSel->SetChecked(true);
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
			boost::make_optional<const T&>(selItem->GetValue()) :
			boost::none;
	}

public:
	using valueChangedSignal_t = boost::signals2::signal<void()>;
	valueChangedSignal_t &GetValueChangedSignal() { return valueChangedSignal; }

protected:
	void Layout() override
	{
		double cx = 0;
		double w = GetSize().x;

		ForEachChild([&](const std::shared_ptr<UiViewModel> &model) {
			auto item = static_cast<DefaultItem*>(model.get());

			auto size = item->Measure();
			item->SetSize(w, size.y);
			item->SetPos(0, cx);

			cx += size.y;
		});
	}

private:
	std::vector<std::shared_ptr<DefaultItem>> items;
	DefaultItem *selItem;
	valueChangedSignal_t valueChangedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
