
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
			enum
			{
				MARGIN = SUPER::Props::NEXT_,
				PADDING,
				NEXT_,  ///< First index for subclasses.
			};
		};

	public:
		FlexGrid(Display &display, uiLayoutFlags_t layoutFlags = 0);
		virtual ~FlexGrid() { }

	public:
		const Vec2 &GetMargin() const { return margin; }
		void SetMargin(double width, double height);

		const Vec2 &GetPadding() const { return padding; }
		void SetPadding(double width, double height);

	public:
		static Vec2 AlignCellContents(double x, double y, double w, double h,
			Alignment alignment);

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
				virtual void SetAlignment(Alignment alignment) = 0;

				/**
				 * Set whether the widget is resized to fill the space of the
				 * cell.
				 *
				 * If the widget cannot be resized, then nothing will be done.
				 *
				 * @param fill @c true To resize the widget,
				 *             @c false to leave it alone.
				 */
				virtual void SetFill(bool fill) = 0;

			protected:
				/**
				 * Set the position and size of the cell.
				 * @param x The X position of the cell.
				 * @param y The Y position of the cell.
				 * @param w The width of the cell.
				 * @param h The height of the cell.
				 * @param paddingX The horizontal padding on the cell.
				 * @param paddingY The vertical padding on the cell.
				 */
				virtual void SetExtents(double x, double y,
					double w, double h, double paddingX, double paddingY) = 0;

				/**
				 * Measure the size of the cell contents.
				 * @return The size (may be zero).
				 */
				virtual Vec3 Measure() = 0;
		};

	protected:
		// Determine if a class as a SetSize(Vec2).
		template<class C>
		struct HasSetSize
		{
		private:
			template<class T>
			static auto check(T*) ->
				decltype(std::declval<T>().SetSize(std::declval<Vec2>()),
				std::true_type());

			template<class>
			static std::false_type check(...);

			typedef decltype(check<C>(nullptr)) type;

		public:
			static const bool value = type::value;
		};

		class DefaultCell : public Cell
		{
			typedef Cell SUPER;
			public:
				DefaultCell() : SUPER(),
					alignment(Alignment::NW), fill(false) { }
				DefaultCell(DefaultCell &&o) : SUPER(),
					alignment(o.alignment) { }
				virtual ~DefaultCell() { }

			public:
				DefaultCell &operator=(DefaultCell &&o) {
					alignment = o.alignment;
					return *this;
				}

			public:
				Alignment GetAlignment() const { return alignment; }

				void SetAlignment(Alignment alignment) override
				{
					this->alignment = alignment;
				}

				bool IsFill() const { return fill; }

				void SetFill(bool fill) override
				{
					this->fill = fill;
				}

			protected:
				void SetExtents(double, double, double, double,
					double, double) override { }
				Vec3 Measure() override { return Vec3(0, 0, 0);  }

			private:
				Alignment alignment;
				bool fill;
		};

		template<typename T>
		class BasicCell : public Cell
		{
			typedef Cell SUPER;
			public:
				BasicCell(FlexGrid *parent, const DefaultCell &defaultCell,
					std::shared_ptr<T> contents) :
					SUPER(), parent(parent), contents(std::move(contents)),
					fill(defaultCell.IsFill())
				{
					SetAlignment(defaultCell.GetAlignment());
				}

				virtual ~BasicCell()
				{
					parent->RemoveChild(contents);
				}

			public:
				virtual void SetAlignment(Alignment alignment)
				{
					// We track the alignment using the contents' own alignment
					// since the caller has implicitly given us control over
					// the position.
					if (alignment != contents->GetAlignment()) {
						contents->SetAlignment(alignment);
						parent->RequestLayout();
					}
				}

				virtual void SetFill(bool fill) override
				{
					this->fill = fill;
				}

			protected:
				template<class U>
				typename std::enable_if<HasSetSize<U>::value, void>::type
				SetSize(double w, double h)
				{
					contents->SetSize(Vec2(w, h));
				}

				template<class U>
				typename std::enable_if<!HasSetSize<U>::value, void>::type
				SetSize(double, double)
				{
					// Do nothing.
				}

				void SetExtents(double x, double y,
					double w, double h,
					double paddingX, double paddingY) override
				{
					Vec2 pos = FlexGrid::AlignCellContents(x, y,
						w - (paddingX * 2), h - (paddingY * 2),
						contents->GetAlignment());
					contents->SetPos(pos.x + paddingX, pos.y + paddingY);
					if (fill) {
						SetSize<T>(w, h);
					}
				}

				Vec3 Measure() override
				{
					return contents->Measure();
				}

			public:
				std::shared_ptr<T> &GetContents() { return contents; }

			private:
				FlexGrid *parent;
				std::shared_ptr<T> contents;
				bool fill;
		};

	public:
		/**
		 * Set the default cell settings for a column.
		 *
		 * This does not retro-actively set the defaults for cells already
		 * added.
		 *
		 * @param col The column coordinate (starting at zero).
		 * @return A mutable cell that will be used as the default template.
		 */
		Cell &GetColumnDefault(size_t col)
		{
			if (col >= defaultCols.size()) {
				defaultCols.resize(col + 1);
			}

			return defaultCols[col];
		}

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

			// Resize the grid to accomodate the cell.
			if (row >= rows.size()) {
				rows.resize(row + 1);
			}
			auto &cols = rows[row];
			if (col >= cols.size()) {
				cols.resize(col + 1);
				if (col >= defaultCols.size()) {
					defaultCols.resize(col + 1);
				}
			}

			std::shared_ptr<BasicCell<T>> cell =
				std::make_shared<BasicCell<T>>(this, defaultCols[col],
					sharedChild);

			// Add the new cell, replacing the old one if necessary.
			cols[col] = cell;

			RequestLayout();

			return cell;
		}

		virtual void Clear();

	protected:
		virtual void Layout();

	public:
		virtual Vec3 Measure();

	private:
		Vec2 margin;
		Vec2 padding;
		Vec2 size;
		typedef std::vector<std::shared_ptr<Cell>> cells_t;
		std::vector<DefaultCell> defaultCols;
		std::vector<cells_t> rows;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
