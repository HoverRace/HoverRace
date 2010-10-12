// Controller.h
//
// This file contains the definition for the HoverRace::Client::Controller
// class.  It contains all the code to control input devices; joysticks, 
// keyboards, and the like.

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>
#include <string>

#include "OIS/OIS.h"
#include "OIS/OISInputManager.h"
#include "OIS/OISException.h"
#include "OIS/OISKeyboard.h"
#include "OIS/OISMouse.h"
#include "OIS/OISJoyStick.h"
#include "OIS/OISEvents.h"
#include "../../../engine/Util/Config.h"
#include "../../../engine/Util/OS.h"
#include "../../../engine/MainCharacter/MainCharacter.h"
#include "../HoverScript/HighConsole.h"
#include "../Observer.h"

#include "ControlAction.h"

#define	CTL_MOTOR_ON	1
#define CTL_LEFT		2
#define CTL_RIGHT		3
#define CTL_JUMP		4
#define CTL_BRAKE		5
#define CTL_FIRE		6
#define CTL_WEAPON		7
#define CTL_LOOKBACK	8

#define AXIS_X			1
#define AXIS_Y			2
#define AXIS_Z			3

#define UNASSIGNED		-1

#define SET_CONTROL		(WM_USER + 1)

// maybe later... TODO
//#include "OIS/OISForceFeedback.h"

namespace HoverRace {
namespace Client {
namespace Control {

class InputHandler;
typedef boost::shared_ptr<InputHandler> InputHandlerPtr;
class UiHandler;
typedef boost::shared_ptr<UiHandler> UiHandlerPtr;

using namespace OIS;

/***
 * Contains information on the current control state.  Eventually, its members should
 * be made analog instead of digital (well, the ones that can, at least).
 */
struct ControlState {
	// TODO: make these inputs analog, not digital
	bool motorOn;
	bool jump;
	bool brake;
	bool fire;
	bool weapon;
	bool lookBack;
	bool right;
	bool left;
};

class InputEventController : public KeyListener, public MouseListener, public JoyStickListener {
	public:
		InputEventController(Util::OS::wnd_t mainWindow, UiHandlerPtr uiHandler);
		~InputEventController();

		// Typedef for the maps of hashes to controls
		typedef std::map<int, ControlAction*> ActionMap;

		// event handlers
		bool keyPressed(const KeyEvent &arg);
		bool keyReleased(const KeyEvent &arg);
		bool mouseMoved(const MouseEvent &arg);
		bool mousePressed(const MouseEvent &arg, MouseButtonID id);
		bool mouseReleased(const MouseEvent &arg, MouseButtonID id);
		bool buttonPressed(const JoyStickEvent &arg, int button);
		bool buttonReleased(const JoyStickEvent &arg, int button);
		bool axisMoved(const JoyStickEvent &arg, int axis);
		bool povMoved(const JoyStickEvent &arg, int pov);

		void Poll();
		void HandleEvent(int hash, int value);

		/***
		 * This function tells the InputEventController to capture the next user input
		 * event and assign the action currently residing at 'oldhash' to the hash of
		 * the new input.  Behavior is undefined if there is no action assigned to the
		 * old hash, so don't screw it up!  This is meant to be called by the control
		 * assignment dialog box.
		 *
		 * @param oldhash Old hash.
		 * @param mapname String representing the name of the map.
		 */
		void CaptureNextInput(int oldhash, std::string mapname);

		/***
		 * This function indicates whether or not the controller is capturing an input.
		 * It can be used to check whether or not an input has been captured.
		 */
		bool IsCapturing();

		/***
		 * This function stops a capture.
		 */
		void StopCapture();

		/***
		 * This function assigns the next disabled hash to the current capture control.
		 * Nothing will be done if the InputEventController is not in capture mode.  This
		 * function will also disable capture mode.
		 */
		void DisableCaptureInput();

		/***
		 * Clears all of the active control bindings.  Does not delete the bindings
		 * but simply removes them from the active action map.
		 */
		void ClearActionMap();

		/***
		 * Add an action map into the current action map.  The available maps are
		 * referenced by string.  Maps include:
		 *
		 * "player1" ... "player4"
		 *
		 * @return false if the map is not found
		 */
		bool AddActionMap(std::string mapname);

		/***
		 * Return a vector containing the current maps.
		 */
		const std::vector<std::string>& GetActiveMaps();

		/***
		 * Return the map with the given key.
		 */
		ActionMap& GetActionMap(std::string key);

		/***
		 * Return a vector containing the names of all the available maps.
		 */
		std::vector<std::string> GetAvailableMaps();

		/***
		 * Update player mappings to point to correct MainCharacter objects, then
		 * add them to the active action map.  If NULL is passed as any of the pointers
		 * that map will not be added.
		 * The "console-toggle" map will also be added.
		 */
		void AddPlayerMaps(int numPlayers, MainCharacter::MainCharacter** mcs);

		/***
		 * Update pointers to Observer objects and add "Camera" map to the active
		 * action maps.
		 */
		void AddObserverMaps(Observer** obs, int numObs);

		/***
		 * Convert a hash into an internationalized string.
		 */
		std::string HashToString(int hash);

		/***
		 * Set up controls for the console.
		 */
		void LoadConsoleMap();

		/***
		 * Update the pointer to the Console.
		 */
		void SetConsole(HoverScript::HighConsole* hc);

		/***
		 * Save the controller configuration to the Config object.
		 */
		void SaveConfig();

		/***
		 * Clear and reload entire configuration.
		 */
		void ReloadConfig();

		/***
		 * Load the configuration from the Config object.
		 */
		void LoadConfig();

	private:
		void InitInputManager(Util::OS::wnd_t mainWindow);

		// Auxiliary functions
		void RebindKey(std::string mapname, int oldhash, int newhash);

		// Hashing scheme (we have 32 bits but won't always use them):
		// disabled control
		// [000000000000000000][aaaaaaaaaaaa]
		//   a: next available disabled id
		// keyboard event
		// [00000000][00][000000][aaaaaaaa][00000000]
		//	 a: int keycode
		// mouse event
		// [00000000][01][00][aaaaaaaa][000000000000]: button press
		//   a: button id
		// [00000000][01][01][aaaa][bbbb][000000000000]: axis move
		//	 a: axis id
		//   b: direction
		// joystick event
		// [00000000][10][00][aaaaaaaa][bbbbbbbb][0000]: button press
		//   a: joystick id
		//   b: button id
		// [00000000][10][01][aaaaaaaa][bbbbbbbb][0000]: slider move
		//   a: joystick id
		//   b: slider id
		// [00000000][10][10][aaaaaaaa][bbbb][cccc][0000]: pov move
		//   a: joystick id
		//   b: direction
		//   c: pov id
		// [00000000][10][11][aaaaaaaa][bbbb][cccc][0000]: axis move
		//   a: joystick id
		//   b: axis id
		//   c: direction
		int GetNextAvailableDisabledHash();
		int HashKeyboardEvent(const KeyCode& arg);
		int HashMouseButtonEvent(const MouseEvent& arg, MouseButtonID id);
		int HashMouseAxisEvent(const MouseEvent& arg, int axis, int direction);
		int HashJoystickAxisEvent(const JoyStickEvent& arg, int axis, int direction);
		int HashJoystickSliderEvent(const JoyStickEvent& arg, int slider);
		int HashJoystickButtonEvent(const JoyStickEvent& arg, int button);
		int HashJoystickPovEvent(const JoyStickEvent& arg, int pov, int direction);

		/***
		 * We store several different action maps which we can choose from.
		 * They are referenced by string.  See ClearActionMap(), AddActionMap().
		 */
		ActionMap actionMap;
		std::vector<std::string> activeMaps;
		std::map<std::string, ActionMap> allActionMaps;

		/// OIS input manager does most of the work for us
		InputManager *mgr;

		UiHandlerPtr uiHandler;

		// now the input devices
		Keyboard *kbd;
		Mouse *mouse;
		int numJoys;
		JoyStick **joys;
		int *joyIds;

		int nextAvailableDisabledHash;

		bool captureNextInput;
		int  captureOldHash; /// stores the value of the hash we will be replacing when capturing input
		std::string captureMap; /// name of the map we are capturing for
};

} // namespace Control
} // namespace Client
} // namespace HoverRace

#endif
