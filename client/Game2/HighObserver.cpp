
// HighObserver.cpp
// Global HUD.
//
// Copyright (c) 2009 Michael Imamura.
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

#include <boost/format.hpp>

#include "../../engine/Util/Config.h"
#include "../../engine/VideoServices/2DViewPort.h"
#include "../../engine/VideoServices/Font.h"
#include "../../engine/VideoServices/MultipartText.h"
#include "../../engine/VideoServices/NumericGlyphs.h"
#include "../../engine/VideoServices/StaticText.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "ClientSession.h"

#include "HighObserver.h"

using namespace HoverRace;
using HoverRace::Util::Config;
using HoverRace::Util::OS;
using namespace HoverRace::VideoServices;

#define STATS_COLOR 0x47

HighObserver::HighObserver()
{
	const Config *cfg = cfg->GetInstance();

	viewport = new MR_2DViewPort();

	statsFont = new Font("Arial", 12);
	statsNumGlyphs = new NumericGlyphs(*statsFont, STATS_COLOR);

	fpsTxt = new MultipartText(*statsFont, statsNumGlyphs, STATS_COLOR);
	*fpsTxt << pgettext("HUD|Frames Per Second","FPS") << ": " <<
		boost::format("%0.2f", OS::locale);
}

HighObserver::~HighObserver()
{
	delete fpsTxt;

	delete statsNumGlyphs;
	delete statsFont;

	delete viewport;
}

/**
 * Render the stats layer.
 * @param session The current client session.
 */
void HighObserver::RenderStats(const MR_ClientSession *session) const
{
	Config *cfg = Config::GetInstance();

	if (cfg->runtime.showFramerate)
		fpsTxt->Blt(2, 2, viewport, session->GetCurrentFramerate());
}

/**
 * Render into the video buffer.
 * @param dest The target video buffer.
 * @param session The current client session.
 */
void HighObserver::Render(MR_VideoBuffer *dest, const MR_ClientSession *session)
{
	viewport->Setup(dest, 0, 0, dest->GetXRes(), dest->GetYRes());

	RenderStats(session);
}
