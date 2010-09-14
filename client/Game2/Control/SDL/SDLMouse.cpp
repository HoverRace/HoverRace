/*
The zlib/libpng License

Copyright (c) 2005-2007 Phillip Castaneda (pjcast -- www.wreckedgames.com)

This software is provided 'as-is', without any express or implied warranty. In no event will
the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following
restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that 
		you wrote the original software. If you use this software in a product, 
		an acknowledgment in the product documentation would be appreciated but is 
		not required.

    2. Altered source versions must be plainly marked as such, and must not be 
		misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

/*
This file has been altered from the original OIS 1.2.0 distribution in order
to integrate directly with the Linux build of HoverRace.

The original distribution may be found at:
  http://sourceforge.net/projects/wgois/
*/

#include "SDLMouse.h"
#include "SDLInputManager.h"
#include "OISException.h"
#include "OISEvents.h"

namespace HoverRace {
namespace Client {
namespace Control {
namespace SDL {

using namespace OIS;

//-------------------------------------------------------------------//
SDLMouse::SDLMouse( OIS::InputManager *creator, bool buffered, bool grab ) :
	Mouse(creator->inputSystemName(), buffered, 0, creator),
	grab(grab), mGrabbed(false), mRegainFocus(false)
{
	mBuffered = buffered;
	mType = OISMouse;
	mListener = 0;

	static_cast<SDLInputManager*>(creator)->setMouseUsed(true);
}

//-------------------------------------------------------------------//
void SDLMouse::_initialize()
{
	//Clear old state
	mState.clear();
	mRegainFocus = false;

	_setVisible(false);

	if (grab) {
		_setGrab(true);
		static_cast<SDLInputManager*>(mCreator)->_setGrabMode(true);
	}
}

//-------------------------------------------------------------------//
SDLMouse::~SDLMouse()
{
	if (grab) {
		_setGrab(false);
		static_cast<SDLInputManager*>(mCreator)->_setGrabMode(false);
	}

	_setVisible(true);

	static_cast<SDLInputManager*>(mCreator)->setMouseUsed(false);
}

//-------------------------------------------------------------------//
void SDLMouse::capture()
{
	//Used for going from SDL Button to OIS button
	static const MouseButtonID ButtonMask[4] = {MB_Left, MB_Left, MB_Middle, MB_Right};

	//Clear old relative values
	mState.X.rel = mState.Y.rel = mState.Z.rel = 0;

	SDL_Event events[OIS_SDL_MOUSE_BUFF];
	int count = SDL_PeepEvents(events, OIS_SDL_MOUSE_BUFF, SDL_GETEVENT, SDL_MOUSEEVENTMASK);

	bool mouseXYMoved = false;
	bool mouseZMoved = false;
	for( int i = 0; i < count; ++i )
	{
		switch( events[i].type )
		{
			case SDL_MOUSEMOTION: mouseXYMoved = true; break;				
			case SDL_MOUSEBUTTONDOWN:
			{
				mRegainFocus = true;
				int sdlButton = events[i].button.button;
				if( sdlButton <= SDL_BUTTON_RIGHT )
				{	//Left, Right, or Middle
					mState.buttons |= (1 << ButtonMask[sdlButton]);
					if( mBuffered && mListener )
						if( mListener->mousePressed(MouseEvent(this, mState), ButtonMask[sdlButton]) == false )
							return;
				}
				else
				{	//mouse Wheel
					mouseZMoved = true;
					if( sdlButton == SDL_BUTTON_WHEELUP )
						mState.Z.rel += 120;
					else if( sdlButton == SDL_BUTTON_WHEELDOWN )
						mState.Z.rel -= 120;
				}
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				int sdlButton = events[i].button.button;
				if( sdlButton <= SDL_BUTTON_RIGHT )
				{	//Left, Right, or Middle
					mState.buttons &= ~(1 << ButtonMask[sdlButton]);
					if( mBuffered && mListener )
						if( mListener->mouseReleased(MouseEvent(this, mState), ButtonMask[sdlButton]) == false )
							return;
				}
				break;
			}
		}
	}

	//Handle X/Y axis move
	if( mouseXYMoved )
	{
		SDL_GetMouseState( &mState.X.abs, &mState.Y.abs );
		SDL_GetRelativeMouseState( &mState.X.rel, &mState.Y.rel );

		if( mBuffered && mListener )
			mListener->mouseMoved(MouseEvent(this, mState));
	}
	//Handle Z Motion
	if( mouseZMoved )
	{
		mState.Z.abs += mState.Z.rel;
		if( mBuffered && mListener )
			mListener->mouseMoved(MouseEvent(this, mState));
	}

	//Handle Alt-Tabbing
	SDLInputManager* man = static_cast<SDLInputManager*>(mCreator);
	if( man->_getGrabMode() == false )
	{
		if( mRegainFocus == false && mGrabbed == true )
		{	//We had focus, but must release it now
			_setGrab(false);
			_setVisible(true);
		}
		else if( mRegainFocus == true && mGrabbed == false )
		{	//We are gaining focus back (mouse clicked in window)
			_setGrab(true);
			_setVisible(false);
			man->_setGrabMode(true);	//Notify manager
		}
	}
}

//-------------------------------------------------------------------//
void SDLMouse::setBuffered(bool buffered)
{
	mBuffered = buffered;
}

//-------------------------------------------------------------------//
void SDLMouse::_setGrab(bool grabbed)
{
	if( grabbed )
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);

	mGrabbed = grabbed;
}

//-------------------------------------------------------------------//
void SDLMouse::_setVisible(bool visible)
{

	if( visible )
		SDL_ShowCursor(SDL_ENABLE);
	else
		SDL_ShowCursor(SDL_DISABLE);
}

}  // namespace SDL
}  // namespace Control
}  // namespace Client
}  // namespace HoverRace
