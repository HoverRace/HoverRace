
// FlexGrid.h
//
// Copyright (c) 2014 Michael Imamura.
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

#include "Container.h"

#ifdef _WIN32
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
 * A container that arranges components into a grid that is automatically
 * sized to the contents.
 * @author Michael Imamura
 */
class MR_DllDeclare FlexGrid : public Container
{
	typedef Container SUPER;

	public:
		struct Props
		{
			enum {
				MARGIN = SUPER::Props::NEXT_,
				PADDING,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		FlexGrid(Display &display, uiLayoutFlags_t layoutFlags=0);
		virtual ~FlexGrid() { }

	public:
		const Vec2 &GetMargin() const { return margin; }
		void SetMargin(double width, double height);

		const Vec2 &GetPadding() const { return padding; }
		void SetPadding(double width, double height);

	public:
		static Vec2 AlignCellContents(double x, double y, double w, double h,
			Alignment::type alignment);

		class Cell
		{
			friend class FlexGrid;
			public:
				Cell() { }
				virtual ~Cell() { }

			public:
				/**
				 * Set the how the widget is aligned inside the cell when
				 * the cell is larger than the widget.
				 * @param alignment The alignment (default is @c NW).
				 */
				virtual void SetAlignment(Alignment::type alignment) = 0;

			protected:
				/**
				 * Set the position and size of the cell.
				 * @param x The X position of the cell.
				 * @param y The Y position of the cell.
				 * @param w The width of the cell.
				 * @param h The height of the cell.
				 */
				virtual void SetExtents(double x, double y,
					double w, double h, double paddingX, double paddingY) = 0;

				/**
				 * Measure the size of the cell contents.
				 * @return The size (may be zero).
				 */
				virtual Vec3 Measure() = 0;
		};

		template<typename T>
		class BasicCell : public Cell
		{
			typedef Cell SUPER;
			public:
				BasicCell(FlexGrid *parent, std::shared_ptr<T> contents) :
					SUPER(), parent(parent), contents(std::move(contents)) { }
				virtual ~BasicCell() { }

			public:
				virtual void SetAlignment(Alignment::type alignment)
				{
					// We track the alignment using the contents' own alignment
					// since the caller has implicitly given us control over
					// the position.
					if (alignment != contents->GetAlignment()) {
						contents->SetAlignment(alignment);
						parent->RequestLayout();
					}
				}

			protected:
				virtual void SetExtents(double x, double y,
					double w, double h, double paddingX, double paddingY)
				{
					Vec2 pos = FlexGrid::AlignCellContents(x, y,
						w - (paddingX * 2), h - (paddingY * 2),
						contents->GetAlignment());
					contents->SetPos(pos.x + paddingX, pos.y + paddingY);
				}

				virtual Vec3 Measure()
				{
					return contents->Measure();
				}

			public:
				std::shared_ptr<T> &GetContents() { return contents; }

			private:
				FlexGrid *parent;
				std::shared_ptr<T> contents;
		};

	public:
		/**
		 * Append a child element to the grid.
		 *
		 * If the cell coordinates are outside of the current grid size, then
		 * the grid is automatically resized to fit the cell.
		 *
		 * @param row The row coordinate (starting at zero).
		 * @param col The column coordinate (starting at zero).
		 * @param child The child element; must be a subclass of UiViewModel.
		 * @return The child element, wrapped in a @c std::shared_ptr, wrapped
		 *         in the table cell.
		 */
		template<typename T>
		typename std::enable_if<
			std::is_base_of<UiViewModel, T>::value,
			std::shared_ptr<BasicCell<T>>
			>::type
		AddGridCell(size_t row, size_t col, T *child)
		{
			std::shared_ptr<T> sharedChild = AddChild(child);
			std::shared_ptr<BasicCell<T>> cell =
				std::make_shared<BasicCell<T>>(this, sharedChild);

			// Resize the grid to accomodate the cell.
			if (row >= rows.size()) {
				rows.resize(row + 1);
			}
			auto &cols = rows[row];
			if (col >= cols.size()) {
				cols.resize(col + 1);
				if (col + 1 > numCols) {
					numCols = col + 1;
				}
			}
			cols[col] = cell;

			RequestLayout();

			return cell;
		}

		virtual void Clear();

	protected:
		virtual void Layout();

	public:
		virtual Vec3 Measure() const;

	private:
		Vec2 margin;
		Vec2 padding;
		size_t numCols;
		typedef std::vector<std::shared_ptr<Cell>> cells_t;
		std::vector<cells_t> rows;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
