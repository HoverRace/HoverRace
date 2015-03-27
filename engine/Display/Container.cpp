
// Container.cpp
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

#include "../Exception.h"

#include "Container.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

void Container::OnChildRequestedFocus(UiViewModel &child)
{
	if (!IsFocused()) {
		RequestFocus();
	}

	if (IsFocused()) {
		// Switch focus to the new child, if possible.
		if (focusedChild) {
			focusedChild->DropFocus();
			focusedChild = nullptr;
		}
		if (child.TryFocus()) {
			focusedChild = &child;
		}
		else {
			// The child that requested focus refused to take the focus.
			RelinquishFocus(Control::Nav::NEUTRAL);
		}
	}
}

void Container::OnChildRelinquishedFocus(
	UiViewModel &child, const Control::Nav &nav)
{
	using Nav = Control::Nav;

	if (focusedChild) {
		focusedChild->DropFocus();
		focusedChild = nullptr;
	}

	// Find the child in the list of children.
	// We assume that there won't be hundreds of children in each container,
	// so a simple linear search will do.
	auto &children = GetChildren();
	auto iter = children.begin();
	for (; iter != children.end(); ++iter) {
		if ((*iter)->child.get() == &child) break;
	}
	if (iter == children.end()) {
		RelinquishFocus(Control::Nav::NEUTRAL);
		return;
	}

	auto dir = nav.AsDigital();
	switch (dir) {
		case Nav::NEUTRAL:
			RelinquishFocus(nav);
			break;

		case Nav::UP:
		case Nav::LEFT:
		case Nav::PREV:
			FocusPrevFrom(iter, nav);
			break;

		case Nav::DOWN:
		case Nav::RIGHT:
		case Nav::NEXT:
			FocusNextFrom(iter, nav);
			break;

		default:
			throw UnimplementedExn(boost::str(boost::format(
				"Container::OnChildRelinquishedFocus(%s)") % nav));
	}
}

/**
 * Shift focus to the first focusable widget previous to the child.
 * @param startingPoint
 * @param nav The current navigation direction (either Nav::UP or Nav::LEFT).
 */
void Container::FocusPrevFrom(children_t::iterator startingPoint,
	const Control::Nav &nav)
{
	auto fin = GetChildren().begin();

	for (auto iter = startingPoint;;) {
		if (iter == fin) {
			break;
		}
		--iter;

		if ((*iter)->child->TryFocus(nav)) {
			focusedChild = (*iter)->child.get();
			return;
		}
	}

	// No widget could be focused; relinquish our own focus.
	RelinquishFocus(nav);
}

/**
 * Shift focus to the first focusable widget after the child.
 * @param startingPoint
 * @param nav The current navigation direction (either Nav::RIGHT or Nav::DOWN).
 */
void Container::FocusNextFrom(children_t::iterator startingPoint,
	const Control::Nav &nav)
{
	auto fin = GetChildren().end();

	for (auto iter = startingPoint;;) {
		++iter;
		if (iter == fin) {
			break;
		}

		if ((*iter)->child->TryFocus(nav)) {
			focusedChild = (*iter)->child.get();
			return;
		}
	}

	// No widget could be focused; relinquish our own focus.
	RelinquishFocus(nav);
}

bool Container::TryFocus(const Control::Nav &nav)
{
	using Nav = Control::Nav;

	if (IsFocused()) return true;
	if (!IsVisible()) return false;

	// Attempt to focus the first focusable widget.
	auto &children = GetChildren();
	auto dir = nav.AsDigital();
	switch (dir) {
		case Nav::NEUTRAL:
		case Nav::RIGHT:
		case Nav::DOWN:
		case Nav::NEXT:
			// Search forward.
			for (auto &child : children) {
				if (child->child->TryFocus(nav)) {
					focusedChild = child->child.get();
					SetFocused(true);
					return true;
				}
			}
			return false;

		case Nav::LEFT:
		case Nav::UP:
		case Nav::PREV:
			// Search backwards.
			for (auto iter = children.rbegin();
				iter != children.rend(); ++iter)
			{
				auto &child = (*iter)->child;
				if (child->TryFocus(nav)) {
					focusedChild = child.get();
					SetFocused(true);
					return true;
				}
			}
			return false;

		default:
			throw UnimplementedExn(boost::str(
				boost::format("Container::TryFocus: Unhandled: %s") % nav));
	}
}

void Container::DropFocus()
{
	if (focusedChild) {
		focusedChild->DropFocus();
		focusedChild = nullptr;
	}
	SUPER::DropFocus();
}

void Container::Clear()
{
	if (focusedChild) {
		focusedChild->DropFocus();
		focusedChild = nullptr;
		RelinquishFocus(Control::Nav::NEUTRAL);
	}

	SUPER::Clear();
}

}  // namespace Display
}  // namespace HoverRace
