
// FlexGrid.h
//
// Copyright (c) 2014, 2015 Michael Imamura.
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
#include "MPL.h"

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
class MR_DllDeclare FlexGrid : public BaseContainer
{
	using SUPER = BaseContainer;

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
	bool OnAction() override
	{
		if (focusedCell) {
			return rows[focusedCell->first][focusedCell->second]->OnAction();
		}
		return false;
	}

	bool OnNavigate(const Control::Nav &nav) override
	{
		if (focusedCell) {
			return rows[focusedCell->first][focusedCell->second]->
				OnNavigate(nav);
		}
		return false;
	}

protected:
	void OnChildRequestedFocus(UiViewModel &child) override;
	void OnChildRelinquishedFocus(UiViewModel &child,
		const Control::Nav &nav) override;

private:
	void SetFocusedCell(size_t row, size_t col);

	template<class Fn>
	bool FocusFrom(size_t row, size_t col, const Control::Nav &nav, Fn nextFn)
	{
		while (nextFn(row, col)) {
			auto &cols = rows[row];
			if (col >= cols.size()) break;

			auto &cell = cols[col];
			if (cell && cell->TryFocus(nav)) {
				focusedCell = boost::make_optional(std::make_pair(row, col));
				SetFocused(true);
				return true;
			}
		}

		return false;
	}

	bool FocusUpFrom(size_t row, size_t col, const Control::Nav &nav);
	bool FocusDownFrom(size_t row, size_t col, const Control::Nav &nav);
	bool FocusLeftFrom(size_t row, size_t col, const Control::Nav &nav);
	bool FocusRightFrom(size_t row, size_t col, const Control::Nav &nav);

public:
	bool TryFocus(const Control::Nav &nav = Control::Nav::NEUTRAL) override;
	void DropFocus() override;

public:
	const Vec2 &GetMargin() const { return margin; }
	void SetMargin(double width, double height);

	const Vec2 &GetPadding() const { return padding; }
	void SetPadding(double width, double height);

	static const double AUTOSIZE;

	bool IsFixedWidth() const;
	bool IsFixedHeight() const;
	void SetFixedSize(double w, double h);
	void SetFixedWidth(double w);
	void SetFixedHeight(double h);

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

		/// Trigger TryFocus() on the contents.
		virtual bool TryFocus(const Control::Nav &nav) = 0;

		/// Trigger DropFocus() on the contents.
		virtual void DropFocus() = 0;

		/// Trigger OnAction() on the contents.
		virtual bool OnAction() = 0;

		/// Trigger OnNavigate() on the contents.
		virtual bool OnNavigate(const Control::Nav&) = 0;

		/**
		 * Check if this contains the specified widget. 
		 * @param child The child to check (may be @c nullptr).
		 * @return @c true if the cell contains the widget, @c false otherwise.
		 */
		virtual bool Contains(const UiViewModel *child) = 0;
	};

protected:
	class DefaultCell : public Cell
	{
		using SUPER = Cell;

	public:
		DefaultCell() : SUPER(),
			alignment(Alignment::NW), fill(false) { }
		DefaultCell(DefaultCell &&o) : SUPER(),
			alignment(o.alignment), fill(o.fill) { }
		virtual ~DefaultCell() { }

	public:
		DefaultCell &operator=(DefaultCell &&o) {
			alignment = o.alignment;
			fill = o.fill;
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
		bool TryFocus(const Control::Nav&) override { return false; }
		void DropFocus() override { }
		bool OnAction() override { return false; }
		bool OnNavigate(const Control::Nav&) override { return false; }
		bool Contains(const UiViewModel*) override { return false; }

	private:
		Alignment alignment;
		bool fill;
	};

	template<typename T>
	class BasicCell : public Cell
	{
		using SUPER = Cell;

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
		void SetAlignment(Alignment alignment) override
		{
			// We track the alignment using the contents' own alignment
			// since the caller has implicitly given us control over
			// the position.
			if (alignment != contents->GetAlignment()) {
				contents->SetAlignment(alignment);
				parent->RequestLayout();
			}
		}

		void SetFill(bool fill) override
		{
			this->fill = fill;
		}

	protected:
		void SetSize(double w, double h)
		{
			MPL::SetSize(*contents, w, h);
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
				SetSize(w, h);
			}
		}

		Vec3 Measure() override
		{
			return contents->Measure();
		}

		bool TryFocus(const Control::Nav &nav) override
		{
			return contents->TryFocus(nav);
		}

		void DropFocus() override
		{
			contents->DropFocus();
		}

		bool OnAction() override
		{
			return contents->OnAction();
		}

		bool OnNavigate(const Control::Nav &nav) override
		{
			return contents->OnNavigate(nav);
		}

		bool Contains(const UiViewModel *child) override
		{
			return contents.get() == child;
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
	 * This is used to reference a cell of the grid without directly accessing
	 * it; useful for adding new widgets to the grid.
	 */
	class CellProxy : public Cell
	{
		using SUPER = Cell;

	public:
		CellProxy(size_t row, size_t col, FlexGrid &grid, Cell *cell) :
			SUPER(), row(row), col(col), grid(grid), cell(cell) { }
		CellProxy(const CellProxy&) = default;
		CellProxy(CellProxy&&) = default;
		virtual ~CellProxy() { }

	public:
		CellProxy &operator=(const CellProxy&) = delete;
		CellProxy &operator=(CellProxy&&) = delete;

	private:
		Cell *CheckCell()
		{
			if (!cell) {
				std::ostringstream oss;
				oss << "Cell at row=" << row << ", col=" << col <<
					" is empty.";
				throw Exception(oss.str());
			}
			else {
				return cell;
			}
		}

	public:
		void SetAlignment(Alignment alignment) override
		{
			CheckCell()->SetAlignment(alignment);
		}

		void SetFill(bool fill) override
		{
			CheckCell()->SetFill(fill);
		}

	protected:
		void SetExtents(double x, double y,
			double w, double h,
			double paddingX, double paddingY) override
		{
			CheckCell()->SetExtents(x, y, w, h, paddingX, paddingY);
		}

		Vec3 Measure() override
		{
			return CheckCell()->Measure();
		}

		bool TryFocus(const Control::Nav &nav) override
		{
			return CheckCell()->TryFocus(nav);
		}

		void DropFocus() override
		{
			CheckCell()->DropFocus();
		}

		bool OnAction() override
		{
			return CheckCell()->OnAction();
		}

		bool OnNavigate(const Control::Nav &nav) override
		{
			return CheckCell()->OnNavigate(nav);
		}

		bool Contains(const UiViewModel *child) override
		{
			return CheckCell()->Contains(child);
		}

	public:
		/**
		 * Creates a new child widget in this cell.
		 *
		 * If the cell coordinates are outside of the current grid size, then
		 * the grid is automatically resized to fit the cell.
		 *
		 * @tparam T The type of the child widget.
		 * @tparam Args The types of the arguments.
		 * @param args The arguments to pass to the child widget's constructor.
		 * @return The actual grid cell 
		 */
		template<class T, class... Args>
		typename std::enable_if<
			std::is_base_of<UiViewModel, T>::value,
			std::shared_ptr<BasicCell<T>>
			>::type
		NewChild(Args&&... args)
		{
			return grid.NewGridCell<T>(row, col, std::forward<Args>(args)...);
		}

	private:
		size_t row;
		size_t col;
		FlexGrid &grid;
		Cell *cell;
	};

public:
	/**
	 * Access a cell of the grid.
	 *
	 * This is usually used to add new widgets to the grid, but can also be
	 * used to change attributes of the cell without accessing the contents.
	 * If adding new widgets, the row and column don't need to already exist.
	 *
	 * @param row The row index.
	 * @param col The column index.
	 * @return A proxy to the cell.
	 */
	CellProxy At(size_t row, size_t col)
	{
		Cell *cell = nullptr;
		if (row < rows.size()) {
			if (col < rows[row].size()) {
				cell = rows[row][col].get();
			}
		}
		return { row, col, *this, cell };
	}

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

private:
	template<class T>
	std::shared_ptr<BasicCell<T>> InitCell(size_t row, size_t col,
		std::shared_ptr<T> sharedChild)
	{
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

protected:
	template<class T, class... Args>
	typename std::enable_if<
		std::is_base_of<UiViewModel, T>::value,
		std::shared_ptr<BasicCell<T>>
		>::type
	NewGridCell(size_t row, size_t col, Args&&... args)
	{
		return InitCell(row, col, NewChild<T>(std::forward<Args>(args)...));
	}

	boost::optional<std::pair<size_t, size_t>> FindChild(UiViewModel *child);

public:
	void Clear() override;

protected:
	void Layout() override;

public:
	Vec3 Measure() override;

private:
	Vec2 margin;
	Vec2 padding;
	Vec2 size;
	Vec2 fixedSize;
	using cells_t = std::vector<std::shared_ptr<Cell>>;
	std::vector<DefaultCell> defaultCols;
	std::vector<cells_t> rows;
	boost::optional<std::pair<size_t, size_t>> focusedCell;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
