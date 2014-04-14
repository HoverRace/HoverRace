
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

#include "FlexGrid.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
FlexGrid::FlexGrid(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags), numCols(0)
{
}

void FlexGrid::Clear()
{
	numCols = 0;
	rows.clear();

	SUPER::Clear();
}

void FlexGrid::Layout()
{
	std::vector<double> heights(rows.size(), 0.0);
	std::vector<double> widths(numCols, 0.0);

	// First, measure each of the cells to determine the height of each row
	// and width of each column.
	auto heightIter = heights.begin();
	auto widthIter = widths.begin();
	BOOST_FOREACH(auto &cols, rows) {
		BOOST_FOREACH(auto &cell, cols) {
			if (cell) {
				Vec3 size = cell->Measure();

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
	BOOST_FOREACH(auto &cols, rows) {
		BOOST_FOREACH(auto &cell, cols) {
			if (cell) {
				cell->SetExtents(x, y, *widthIter, *heightIter);
			}
			x += *widthIter;
			++widthIter;
		}

		y += *heightIter;
		++heightIter;
		x = 0;
		widthIter = widths.begin();
	}
}

Vec3 FlexGrid::Measure() const
{
	//TODO: Perform layout then measure.
	return Vec3(0, 0, 0);
}

}  // namespace Display
}  // namespace HoverRace
