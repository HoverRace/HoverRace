// GDIResources.cpp

#include "stdafx.h"
#include "GDIResources.h"
#include "resource.h"


CPen     gNormalPen;
CPen     gSelectionPen;
CPen     gTempWallPen;
CPen     gWallBrush;
CPen     gSelectWallBrush;
CBrush   gFocusBrush;
CBrush   gNodeBrush;
CBrush   gSelectNodeBrush;
CBrush   gAnchorBrush;
CBrush   gSelectAnchorBrush;
CBrush   gRoomBrush;
CBrush   gSelectRoomBrush;
CBrush   gFeatureBrush;
CBrush   gSelectFeatureBrush;
CBrush   gItemBrush;
CBrush   gSelectItemBrush;
CBitmap  gItemBitmap;
CBitmap  gSelectItemBitmap;


void InitGDIResources()
{
   gNormalPen.CreatePen( PS_SOLID, 0, RGB( 0, 0, 0 ));
   gSelectionPen.CreatePen( PS_SOLID, 0, RGB( 0, 0, 0 ));
   gTempWallPen.CreatePen( PS_SOLID, 5, RGB( 0, 0, 0 ));
   gWallBrush.CreatePen( PS_INSIDEFRAME, 3, RGB(0,0,128));
   gSelectWallBrush.CreatePen( PS_INSIDEFRAME, 3, RGB( 0, 0, 255 ));

   gFocusBrush.CreateSolidBrush( RGB(255,0,255));
   gNodeBrush.CreateSolidBrush( RGB(128,0,0));
   gSelectNodeBrush.CreateSolidBrush( RGB(255,0,0));
   gAnchorBrush.CreateSolidBrush( RGB(0,0,128));
   gSelectAnchorBrush.CreateSolidBrush( RGB(0,0,255));
   gRoomBrush.CreateSolidBrush( RGB(128,128,128));
   gSelectRoomBrush.CreateSolidBrush( RGB(192,192,192));
   gFeatureBrush.CreateSolidBrush( RGB(128,128,0));
   gSelectFeatureBrush.CreateSolidBrush( RGB(255,255,0));
   gItemBrush.CreateSolidBrush( RGB(0,128,0));
   gSelectItemBrush.CreateSolidBrush( RGB(0,255,0));
   gItemBitmap.LoadBitmap( IDB_ITEM );
   gSelectItemBitmap.LoadBitmap( IDB_SELECTITEM );
}
