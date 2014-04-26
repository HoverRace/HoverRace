
// FlexGrid.cpp
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

#include "StdAfx.h"

#include "../Exception.h"

#include "FlexGrid.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
FlexGrid::FlexGrid(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags),
	margin(display.styles.gridMargin), padding(display.styles.gridPadding),
	size(0, 0)
{
}

void FlexGrid::SetMargin(double width, double height)
{
	if (margin.x != width || margin.y != height) {
		margin.x = width;
		margin.y = height;

		FireModelUpdate(Props::MARGIN);
		RequestLayout();
	}
}

void FlexGrid::SetPadding(double width, double height)
{
	if (padding.x != width || padding.y != height) {
		padding.x = width;
		padding.y = height;

		FireModelUpdate(Props::MARGIN);
		RequestLayout();
	}
}

/**
 * Adjust the cell origin position to the position of the contents,
 * based on the contents' alignment.
 * @param x The upper-left corner of the cell, relative to the grid.
 * @param y The upper-left corner of the cell, relative to the grid.
 * @param w The width of the cell, not including padding.
 * @param h The height of the cell, not including padding.
 * @param alignment The alignment of the cell contents.
 * @return The adjusted position.
 */
Vec2 FlexGrid::AlignCellContents(double x, double y, double w, double h,
                                 Alignment alignment)
{
	switch (alignment) {
		case Alignment::NW: return Vec2(x, y);
		case Alignment::N: return Vec2(x + (w / 2.0), y);
		case Alignment::NE: return Vec2(x + w, y);
		case Alignment::E: return Vec2(x + w, y + (h / 2.0));
		case Alignment::SE: return Vec2(x + w, y + h);
		case Alignment::S: return Vec2(x + (w / 2.0), y + h);
		case Alignment::SW: return Vec2(x, y + h);
		case Alignment::W: return Vec2(x, y + (h / 2.0));
		case Alignment::CENTER: return Vec2(x + (w / 2.0), y + (h / 2.0));
		default:
			throw Exception("Unknown alignment: " +
				boost::lexical_cast<std::string>(static_cast<int>(alignment)));
	}
}

void FlexGrid::Clear()
{
	defaultCols.clear();
	rows.clear();

	SUPER::Clear();
}

void FlexGrid::Layout()
{
	std::vector<double> heights(rows.size(), 0.0);
	std::vector<double> widths(defaultCols.size(), 0.0);
	Vec2 totalSize(0, 0);

	Vec2 padding2x = padding;
	padding2x *= 2;

	// First, measure each of the cells to determine the height of each row
	// and width of each column.
	auto heightIter = heights.begin();
	auto widthIter = widths.begin();
	for (auto &cols : rows) {
		for (auto &cell : cols) {
			if (cell) {
				Vec3 size = cell->Measure();
				size += padding2x;

				if (size.x > *widthIter) {
					*widthIter = size.x;
				}
				if (size.y > *heightIter) {
					*heightIter = size.y;
				}
			}
			++widthIter;
		}

		++heightIter;
		widthIter = widths.begin();
	}

	// Move each cell contents into position.
	heightIter = heights.begin();
	widthIter = widths.begin();
	double x = 0;
	double y = 0;
	for (auto &cols : rows) {
		for (auto &cell : cols) {
			if (cell) {
				cell->SetExtents(x, y, *widthIter, *heightIter,
					padding.x, padding.y);
			}
			totalSize.x = x;
			x += *widthIter + padding2x.x + margin.x;
			++widthIter;
		}

		totalSize.y = y;
		y += *heightIter + padding2x.y + margin.y;
		++heightIter;
		x = 0;
		widthIter = widths.begin();
	}

	// Update the calculated size of the grid.
	size = totalSize;
}

Vec3 FlexGrid::Measure() const
{
	//FIXME: Measure() really should be non-const because we already document
	//       the caveat that this operation may change state.
	const_cast<FlexGrid*>(this)->PrepareRender();
	return size.Promote();
}

}  // namespace Display
}  // namespace HoverRace
