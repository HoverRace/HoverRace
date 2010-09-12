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

#pragma once

#include "OISInputManager.h"
#include "OISFactoryCreator.h"
#include "SDLPrereqs.h"

namespace HoverRace {
namespace Client {
namespace Control {
namespace SDL {

	/**
		SDL Input Manager wrapper
	*/
	class SDLInputManager : public OIS::InputManager, OIS::FactoryCreator
	{
	public:
		SDLInputManager();
		virtual ~SDLInputManager();

		/** @copydoc InputManager::inputSystemName */
		virtual const std::string& inputSystemName() { return iName; }
		
		/** @copydoc InputManager::numJoysticks */
		virtual int numJoySticks();
		/** @copydoc InputManager::numMice */
		virtual int numMice();
		/** @copydoc InputManager::numKeyBoards */
		virtual int numKeyboards();
		
		/** @copydoc InputManager::createInputObject */
		OIS::Object* createObject( OIS::InputManager*, OIS::Type iType, bool bufferMode, const std::string &vendor="" );
		/** @copydoc InputManager::destroyInputObject */
		void destroyObject( OIS::Object* obj );

		bool vendorExist(OIS::Type iType, const std::string &vendor);

		OIS::DeviceList freeDeviceList();
		int totalDevices(OIS::Type iType);
		int freeDevices(OIS::Type iType);

		void setKeyboardUsed(bool used) { keyboardUsed = used; }
		void setMouseUsed(bool used) { mouseUsed = used; }

		/** @copydoc InputManager::_initialize */
		void _initialize( OIS::ParamList &paramList );

		//Utility methods to coordinate between mouse and keyboard grabbing
		bool _getGrabMode() {return mGrabbed;};
		void _setGrabMode(bool grabbed) {mGrabbed = grabbed;}

	protected:
		//! internal class method for dealing with param list
		void _parseConfigSettings( OIS::ParamList &paramList );
		//! internal class method for finding attached devices
		void _enumerateDevices();

		static const std::string iName;

		bool mGrabbed;

		bool keyboardUsed;
		bool mouseUsed;
	};

}  // namespace SDL
}  // namespace Control
}  // namespace Client
}  // namespace HoverRace
