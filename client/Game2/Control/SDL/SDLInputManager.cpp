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

#include "SDLInputManager.h"
#include "SDLKeyboard.h"
#include "SDLMouse.h"
#include "OISException.h"
#include "OISObject.h"

namespace HoverRace {
namespace Client {
namespace Control {
namespace SDL {

using namespace OIS;

const std::string SDLInputManager::iName = "SDL Input Wrapper";

//--------------------------------------------------------------------------------//
SDLInputManager::SDLInputManager() : InputManager("SDLInputManager"), mGrabbed(false), keyboardUsed(false), mouseUsed(false)
{
	mFactories.push_back(this);
}

//--------------------------------------------------------------------------------//
SDLInputManager::~SDLInputManager()
{
}

//--------------------------------------------------------------------------------//
void SDLInputManager::_initialize( ParamList &paramList )
{
	Uint32 flags = SDL_WasInit(0);
	if( flags == 0 )
		OIS_EXCEPT( E_General, "SDLInputManager::SDLInputManager >> SDL Not Initialized already!");

	_parseConfigSettings( paramList );
	_enumerateDevices();
}

//--------------------------------------------------------------------------------//
void SDLInputManager::_parseConfigSettings( ParamList &paramList )
{
}

//--------------------------------------------------------------------------------//
void SDLInputManager::_enumerateDevices()
{
}

//--------------------------------------------------------------------------------//
int SDLInputManager::numJoySticks()
{
	return 0;
}

//--------------------------------------------------------------------------------//
int SDLInputManager::numMice()
{
	return 1;
}

//--------------------------------------------------------------------------------//
int SDLInputManager::numKeyboards()
{
	return 1;
}

//----------------------------------------------------------------------------//
Object* SDLInputManager::createObject( OIS::InputManager *creator, Type iType, bool bufferMode, const std::string &vendor )
{
	Object* obj = 0;
	
	switch( iType )
	{
		case OISKeyboard: obj = new SDLKeyboard( this, bufferMode ); break;
		case OISMouse: obj = new SDLMouse( this, bufferMode, false ); break;
		case OISJoyStick: 
		default: OIS_EXCEPT( E_InputDeviceNotSupported, "Type not implemented");
	}

	try	{
		obj->_initialize();
	}
	catch(...) {
		delete obj;
		throw; //rethrow
	}

	return obj;
}

//----------------------------------------------------------------------------//
void SDLInputManager::destroyObject( Object* obj )
{
	if( obj == 0 ) return;

	delete obj;
}

bool SDLInputManager::vendorExist(Type iType, const std::string &vendor)
{
	return (iType == OISKeyboard || iType == OISMouse) && vendor == mInputSystemName;
}

DeviceList SDLInputManager::freeDeviceList()
{
	DeviceList retv;

	if (!keyboardUsed)
		retv.insert(DeviceList::value_type(OISKeyboard, mInputSystemName));

	if (!mouseUsed)
		retv.insert(DeviceList::value_type(OISMouse, mInputSystemName));

	return retv;
}

int SDLInputManager::totalDevices(Type iType)
{
	return (iType == OISKeyboard || iType == OISMouse) ? 1 : 0;
}

int SDLInputManager::freeDevices(Type iType)
{
	switch (iType) {
		case OISKeyboard: return keyboardUsed ? 0 : 1;
		case OISMouse: return mouseUsed ? 0 : 1;
		default: return 0;
	}
}

}  // namespace SDL
}  // namespace Control
}  // namespace Client
}  // namespace HoverRace
