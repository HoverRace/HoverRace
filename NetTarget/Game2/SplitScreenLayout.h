#ifndef MR_SPLIT_SCREEN_LAYOUT_H
#define MR_SPLIT_SCREEN_LAYOUT_H

#include "../../include/LocalPlayer.h"

struct MR_SplitScreenViewport
{
	int mX;
	int mY;
	int mWidth;
	int mHeight;
};

struct MR_SplitScreenGrid
{
	int mColumns;
	int mRows;
};

inline MR_SplitScreenGrid MR_GetSplitScreenGrid(int pPlayerCount)
{
	MR_SplitScreenGrid lGrid = { 1, 1 };

	if(pPlayerCount <= 1) {
		return lGrid;
	}
	if(pPlayerCount == 2) {
		lGrid.mColumns = 1;
		lGrid.mRows = 2;
	}
	else if(pPlayerCount <= 4) {
		lGrid.mColumns = 2;
		lGrid.mRows = 2;
	}
	else if(pPlayerCount <= 6) {
		lGrid.mColumns = 2;
		lGrid.mRows = 3;
	}
	else if(pPlayerCount <= 8) {
		lGrid.mColumns = 2;
		lGrid.mRows = 4;
	}
	else if(pPlayerCount == 9) {
		lGrid.mColumns = 3;
		lGrid.mRows = 3;
	}
	else {
		lGrid.mColumns = 4;
		lGrid.mRows = 3;
	}

	return lGrid;
}

inline BOOL MR_GetSplitScreenViewport(int pPlayerCount, int pPlayerIndex,
	int pWidth, int pHeight, MR_SplitScreenViewport &pViewport)
{
	if((pPlayerCount < 1) || (pPlayerCount > MR_MAX_LOCAL_PLAYER) ||
		(pPlayerIndex < 0) || (pPlayerIndex >= pPlayerCount) ||
		(pWidth <= 0) || (pHeight <= 0))
	{
		return FALSE;
	}

	if(pPlayerCount == 10) {
		int lRow;
		int lColumn;
		int lRowColumns;

		if(pPlayerIndex < 3) {
			lRow = 0;
			lColumn = pPlayerIndex;
			lRowColumns = 3;
		}
		else if(pPlayerIndex < 7) {
			lRow = 1;
			lColumn = pPlayerIndex - 3;
			lRowColumns = 4;
		}
		else {
			lRow = 2;
			lColumn = pPlayerIndex - 7;
			lRowColumns = 3;
		}

		const int lY0 = pHeight * lRow / 3;
		const int lY1 = pHeight * (lRow + 1) / 3;

		if(lRowColumns == 4) {
			pViewport.mX = pWidth * lColumn / 4;
			pViewport.mWidth = pWidth * (lColumn + 1) / 4 - pViewport.mX;
		}
		else {
			const int lX0 = pWidth * lColumn / 3;
			const int lX1 = pWidth * (lColumn + 1) / 3;
			pViewport.mX = lX0;
			pViewport.mWidth = lX1 - lX0;
		}

		pViewport.mY = lY0;
		pViewport.mHeight = lY1 - lY0;
		return TRUE;
	}

	MR_SplitScreenGrid lGrid = MR_GetSplitScreenGrid(pPlayerCount);
	const int lColumn = pPlayerIndex % lGrid.mColumns;
	const int lRow = pPlayerIndex / lGrid.mColumns;

	if(lRow >= lGrid.mRows) {
		return FALSE;
	}

	const int lX0 = pWidth * lColumn / lGrid.mColumns;
	const int lX1 = pWidth * (lColumn + 1) / lGrid.mColumns;
	const int lY0 = pHeight * lRow / lGrid.mRows;
	const int lY1 = pHeight * (lRow + 1) / lGrid.mRows;

	pViewport.mX = lX0;
	pViewport.mY = lY0;
	pViewport.mWidth = lX1 - lX0;
	pViewport.mHeight = lY1 - lY0;
	return TRUE;
}

#endif
