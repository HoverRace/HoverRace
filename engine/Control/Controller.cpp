
// Controller.cpp
//
// Copyright (c) 2010 Ryan Curtin.
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../Util/Config.h"
#include "../Util/Log.h"

#include "Controller.h"

#include "ActionPerformers.h"
#include "ControlAction.h"

#include <sstream>

using namespace HoverRace;
using namespace HoverRace::Util;
using namespace std;

namespace HoverRace {
namespace Control {

InputEventController::actions_t::ui_t::ui_t() :
	menuOk(std::make_shared<Action<voidSignal_t>>(_("OK"), 0)),
	menuCancel(std::make_shared<Action<voidSignal_t>>(_("Cancel"), 0)),
	menuExtra(std::make_shared<Action<voidSignal_t>>(_("Extra"), 0)),
	menuUp(std::make_shared<Action<voidSignal_t>>(_("Up"), 0)),
	menuDown(std::make_shared<Action<voidSignal_t>>(_("Down"), 0)),
	menuLeft(std::make_shared<Action<voidSignal_t>>(_("Left"), 0)),
	menuRight(std::make_shared<Action<voidSignal_t>>(_("Right"), 0)),
	menuNext(std::make_shared<Action<voidSignal_t>>(_("Next"), 0)),
	menuPrev(std::make_shared<Action<voidSignal_t>>(_("Previous"), 0)),
	consoleUp(std::make_shared<Action<voidSignal_t>>(_("Page Up"), 0)),
	consoleDown(std::make_shared<Action<voidSignal_t>>(_("Page Down"), 1)),
	consoleTop(std::make_shared<Action<voidSignal_t>>(_("Top"), 2)),
	consoleBottom(std::make_shared<Action<voidSignal_t>>(_("Bottom"), 3)),
	consolePrevCmd(std::make_shared<Action<voidSignal_t>>(_("Prev Cmd"), 4)),
	consoleNextCmd(std::make_shared<Action<voidSignal_t>>(_("Next Cmd"), 5)),
	text(std::make_shared<Action<stringSignal_t, const std::string&>>("", 0)),
	control(std::make_shared<Action<textControlSignal_t, TextControl::key_t>>("", 0)),
	mouseMoved(std::make_shared<Action<vec2Signal_t, const Vec2&>>("", 0)),
	mousePressed(std::make_shared<Action<mouseClickSignal_t, const Mouse::Click&>>("", 0)),
	mouseReleased(std::make_shared<Action<mouseClickSignal_t, const Mouse::Click&>>("", 0)),
	mouseScrolled(std::make_shared<Action<mouseScrollSignal_t, const Mouse::Scroll&>>("", 0))
	{ }

InputEventController::actions_t::sys_t::sys_t() :
	consoleToggle(std::make_shared<Action<voidSignal_t>>(_("Toggle Console"), 0))
	{ }

InputEventController::actions_t::camera_t::camera_t() :
	zoomIn(std::make_shared<Action<voidSignal_t>>(_("Zoom In"), 0)),
	zoomOut(std::make_shared<Action<voidSignal_t>>(_("Zoom Out"), 1)),
	panUp(std::make_shared<Action<voidSignal_t>>(_("Pan Up"), 2)),
	panDown(std::make_shared<Action<voidSignal_t>>(_("Pan Down"), 3)),
	reset(std::make_shared<Action<voidSignal_t>>(_("Reset Camera"), 4))
	{ }

InputEventController::InputEventController() :
	activeMaps(0), nextAvailableDisabledHash(0),
	captureNextInput(false), captureOldHash(0), captureMap(),
	mousePos(0, 0)
{
	LoadConfig();
	LoadCameraMap();
	LoadMenuMap();
	LoadConsoleToggleMap();
	LoadConsoleMap();
}

InputEventController::~InputEventController()
{
}

void InputEventController::ProcessInputEvent(const SDL_Event &evt)
{
	switch (evt.type) {
		case SDL_KEYDOWN: OnKeyPressed(evt.key); break;
		case SDL_KEYUP: OnKeyReleased(evt.key); break;
		case SDL_TEXTINPUT: OnTextInput(evt.text); break;

		case SDL_MOUSEMOTION: OnMouseMoved(evt.motion); break;
		case SDL_MOUSEBUTTONDOWN: OnMousePressed(evt.button); break;
		case SDL_MOUSEBUTTONUP: OnMouseReleased(evt.button); break;
		case SDL_MOUSEWHEEL: OnMouseWheel(evt.wheel); break;

		default:
			Log::Info("Unhandled input event type: %d", evt.type);
	}
}

bool InputEventController::OnKeyPressed(const SDL_KeyboardEvent& arg)
{
	SDL_Keycode kc = arg.keysym.sym;
	auto textInputActive = SDL_IsTextInputActive();

	if (textInputActive) {
		SDL_Keycode nkc = kc;

		// Map numpad enter to the "normal" enter so action subscribers don't need to
		// check for both.
		if (nkc == SDLK_KP_ENTER)
			nkc = SDLK_RETURN;

		switch (nkc) {
			case SDLK_BACKSPACE:
			case SDLK_RETURN:
				// For now, key_t values are mapped directly to SDL keycodes for
				// convenience.
				(*actions.ui.control)(static_cast<TextControl::key_t>(nkc));
				break;
		}
	}

	// make left and right modifiers equal
	if (kc == SDLK_RSHIFT)
		kc = SDLK_LSHIFT;
	else if (kc == SDLK_RCTRL)
		kc = SDLK_LCTRL;
	else if (kc == SDLK_RGUI)
		kc = SDLK_LGUI;

	// Hotkeys are fired in addition to the normal bound action, but we don't
	// want to interfere with text input.
	if (!textInputActive) {
		auto iter = hotkeys.find(kc);
		if (iter != hotkeys.end()) {
			(*(iter->second))(1);
		}
	}

	auto hash = HashKeyboardEvent(kc);

	// Flip the meaning of menuNext if shift is held down.
	if (IsMapActive(ActionMapId::MENU) &&
		hash == actions.ui.menuNext->GetPrimaryTrigger() &&
		(SDL_GetModState() & KMOD_SHIFT))
	{
		hash = actions.ui.menuPrev->GetPrimaryTrigger();
	}

	HandleEvent(hash, 1);
	return true;
}

bool InputEventController::OnKeyReleased(const SDL_KeyboardEvent& arg)
{
	if (SDL_IsTextInputActive()) return true;

	// make left and right modifiers equal
	SDL_Keycode kc = arg.keysym.sym;

	if (kc == SDLK_RSHIFT)
		kc = SDLK_LSHIFT;
	else if (kc == SDLK_RCTRL)
		kc = SDLK_LCTRL;
	else if (kc == SDLK_RGUI)
		kc = SDLK_LGUI;

	// value will be 0 (since the key was released)
	HandleEvent(HashKeyboardEvent(kc), 0);
	return true;
}

bool InputEventController::OnTextInput(const SDL_TextInputEvent &evt)
{
	(*actions.ui.text)(evt.text);

	return true;
}

bool InputEventController::OnMouseMoved(const SDL_MouseMotionEvent& evt)
{
	int x = evt.xrel;
	int y = evt.yrel;

	int ax = abs(x);
	int ay = abs(y);

	if (ax > 0)
		HandleEvent(HashMouseAxisEvent(AXIS_X, (x > 0) ? 1 : 0), ax);
	if (ay > 0)
		HandleEvent(HashMouseAxisEvent(AXIS_Y, (y > 0) ? 1 : 0), ay);

	// For some events (e.g. mouse wheel), we want to pass the mouse position
	// but the SDL event doesn't pass it along.  So, we just keep track of the
	// last recorded mouse position and use that.
	mousePos.x = evt.x;
	mousePos.y = evt.y;

	(*actions.ui.mouseMoved)(mousePos);

	return true;
}

bool InputEventController::OnMousePressed(const SDL_MouseButtonEvent& evt)
{
	HandleEvent(HashMouseButtonEvent(evt), 1);
	(*actions.ui.mousePressed)(Mouse::Click(evt.x, evt.y,
		static_cast<Mouse::button_t>(evt.button)));
	return true;
}

bool InputEventController::OnMouseReleased(const SDL_MouseButtonEvent& evt)
{
	HandleEvent(HashMouseButtonEvent(evt), 0);
	(*actions.ui.mouseReleased)(Mouse::Click(evt.x, evt.y,
		static_cast<Mouse::button_t>(evt.button)));
	return true;
}

bool InputEventController::OnMouseWheel(const SDL_MouseWheelEvent &evt)
{
	int x = evt.x;
	int y = evt.y;

	int ax = abs(x);
	int ay = abs(y);

	if (ax > 0)
		HandleEvent(HashMouseAxisEvent(AXIS_WHEEL_X, (x > 0) ? 1 : 0), ax);
	if (ay > 0)
		HandleEvent(HashMouseAxisEvent(AXIS_WHEEL_Y, (y > 0) ? 1 : 0), ay);

	(*actions.ui.mouseScrolled)(Mouse::Scroll(mousePos.x, mousePos.y, x, y));

	return true;
}

/*TODO

bool InputEventController::buttonPressed(const JoyStickEvent& arg, int button)
{
	HandleEvent(HashJoystickButtonEvent(arg, button), 1);
	return true;
}

bool InputEventController::buttonReleased(const JoyStickEvent& arg, int button)
{
	HandleEvent(HashJoystickButtonEvent(arg, button), 0);
	return true;
}

bool InputEventController::axisMoved(const JoyStickEvent& arg, int axis)
{
	int value;
	if(arg.state.mAxes[axis].absOnly)
		value = arg.state.mAxes[axis].abs;
	else
		value = arg.state.mAxes[axis].rel;

	HandleEvent(HashJoystickAxisEvent(arg, axis, (value > 0) ? 1 : 0), value);
	return true;
}

bool InputEventController::povMoved(const JoyStickEvent& arg, int pov)
{
	HandleEvent(HashJoystickPovEvent(arg, pov, arg.state.mPOV[pov].direction), 0);
	return true;
}

*/

int InputEventController::GetNextAvailableDisabledHash()
{
	while(actionMap.count(++nextAvailableDisabledHash) == 1)
	{
		if(nextAvailableDisabledHash >= 16384)
			nextAvailableDisabledHash = 0; // only allowed to use 14 bits for hash
	}

	return nextAvailableDisabledHash;
}

void InputEventController::HandleEvent(int hash, int value)
{
	// if we are setting controls, we need to update our action map
	if(captureNextInput) {
		RebindKey(captureMap, captureOldHash, hash);

		captureNextInput = false; // don't do this again
		captureMap = "";
		captureOldHash = 0;

		return;
	}

	// fire the action bound to the given input hash code
	if(actionMap.count(hash) == 1)
		(*actionMap[hash])(value);
}

void InputEventController::CaptureNextInput(int oldhash, string mapname)
{
	// the next time the user gives any input, it will be bound to the control
	// that is currently bound to oldhash
	captureNextInput = true;
	captureOldHash = oldhash;
	captureMap = mapname;
}

bool InputEventController::IsCapturing()
{
	return captureNextInput;
}

void InputEventController::StopCapture()
{
	captureNextInput = false;
	captureOldHash = 0;
}

void InputEventController::DisableCaptureInput()
{
	if(!captureNextInput)
		return;

	RebindKey(captureMap, captureOldHash, GetNextAvailableDisabledHash());

	captureOldHash = 0;
	captureMap = "";
	captureNextInput = false;
}

void InputEventController::ClearActionMap()
{
	// remove all active bindings
	actionMap.clear();
	activeMaps = 0;
}

bool InputEventController::AddActionMap(
	const string &mapname, ActionMapId mapId)
{
	auto iter = allActionMaps.find(mapname);
	if (iter == allActionMaps.end()) return false;

	for (auto &map : iter->second) {
		actionMap.insert(map);
	}

	activeMaps |= static_cast<size_t>(mapId);

	return true;
}

InputEventController::ActionMap& InputEventController::GetActionMap(string key)
{
	return allActionMaps[key];
}

vector<string> InputEventController::GetAvailableMaps()
{
	vector<string> maps;
	for(map<string, ActionMap>::iterator it = allActionMaps.begin(); it != allActionMaps.end(); it++)
		maps.push_back(it->first);

	return maps;
}

void InputEventController::AddPlayerMaps(int numPlayers, MainCharacter::MainCharacter** mcs)
{
	// iterate over each actionMap
	// start with the highest player, so the lowest-numbered player's controls override any
	// potentially conflicting controls (this should not happen)
	for(int i = numPlayers - 1; i >= 0; i--) {
		if(mcs[i] == NULL) // player controls do not exist for this player; do not add it
			continue;

		stringstream str;
		str << _("Player") << " " << (i + 1);
		string mapname = str.str();
		if(allActionMaps.count(mapname) != 1)
			continue; // was not loaded for some reason...

		for(ActionMap::iterator it = allActionMaps[mapname].begin(); it != allActionMaps[mapname].end(); it++) {
			PlayerEffectAction* perf = dynamic_cast<PlayerEffectAction*>(it->second.get());
			if(perf != NULL)
				perf->SetMainCharacter(mcs[i]);
			actionMap[it->first] = it->second; // add to active controls
		}
		activeMaps |= static_cast<size_t>(ActionMapId::PLAYER);
	}
}

/// Enable camera controls.
void InputEventController::AddCameraMaps()
{
	AddActionMap(_("Camera"), ActionMapId::CAMERA);
}

/// Enable menu controls.
void InputEventController::AddMenuMaps()
{
	AddActionMap(_("Menu"), ActionMapId::MENU);
}

/// Enable console toggle.
void InputEventController::AddConsoleToggleMaps()
{
	AddActionMap(_("ConsoleToggle"), ActionMapId::CONSOLE_TOGGLE);
}

/// Enable console scroll controls.
void InputEventController::AddConsoleMaps()
{
	AddActionMap(_("Console"), ActionMapId::CONSOLE);
}

/**
 * Assign an action to a hotkey.
 * Hotkey actions are activated in *addition* to any normal action assigned
 * to the key.
 * @param key The string representation of the key.
 * @return The action for the hotkey, or @c nullptr if the key is an invalid
 *         key for a hotkey.
 */
InputEventController::VoidActionPtr InputEventController::Hotkey(const std::string &key)
{
	SDL_Keycode code = StringToKey(key);

	if (code != SDLK_UNKNOWN) {
		auto action = std::make_shared<Action<voidSignal_t>>("", 0);
		hotkeys[code] = action;
		return action;
	}
	else {
		return VoidActionPtr();
	}
}

/// Save the controller configuration to the Config object.
void InputEventController::SaveConfig()
{
	Config *cfg = Config::GetInstance();

	/* We must save each functor's hash */
	for(int i = 0; i < cfg->MAX_PLAYERS; i++) {
		// use map playerX
		stringstream str;
		str << _("Player") << " " << (i + 1);
		ActionMap& playerMap = allActionMaps[str.str()];

		for(ActionMap::iterator it = playerMap.begin(); it != playerMap.end(); it++) {
			// tedious, unfortunately
			switch(it->second->GetListOrder()) {
				case 0: // throttle
					cfg->controlsHash[i].motorOn = it->first;
					break;
				case 1: // brake
					cfg->controlsHash[i].brake = it->first;
					break;
				case 2: // turn left
					cfg->controlsHash[i].left = it->first;
					break;
				case 3: // turn right
					cfg->controlsHash[i].right = it->first;
					break;
				case 4: // jump
					cfg->controlsHash[i].jump = it->first;
					break;
				case 5: // powerup
					cfg->controlsHash[i].fire = it->first;
					break;
				case 6: // change item
					cfg->controlsHash[i].weapon = it->first;
					break;
				case 7: // look back
					cfg->controlsHash[i].lookBack = it->first;
					break;
			}
		}
	}

	// now save console map
	for(ActionMap::iterator it = allActionMaps[_("Console")].begin(); it != allActionMaps[_("Console")].end(); it++) {
		switch(it->second->GetListOrder()) {
			case 0: // toggle console
				cfg->ui.consoleToggle = it->first;
				break;
			case 1: // page up
				cfg->ui.consoleUp = it->first;
				break;
			case 2: // page down
				cfg->ui.consoleDown = it->first;
				break;
			case 3: // top
				cfg->ui.consoleTop = it->first;
				break;
			case 4: // bottom
				cfg->ui.consoleBottom = it->first;
				break;
			case 5: // help
				cfg->ui.consoleHelp = it->first;
				break;
		}
	}

	// now save camera map
	for(ActionMap::iterator it = allActionMaps[_("Camera")].begin(); it != allActionMaps[_("Camera")].end(); it++) {
		switch(it->second->GetListOrder()) {
			case 0: // zoom in
				cfg->cameraHash.zoomIn = it->first;
				break;
			case 1: // zoom out
				cfg->cameraHash.zoomOut = it->first;
				break;
			case 2: // pan up
				cfg->cameraHash.panUp = it->first;
				break;
			case 3: // pan down
				cfg->cameraHash.panDown = it->first;
				break;
			case 4: // reset
				cfg->cameraHash.reset = it->first;
				break;
		}
	}
}

/// Clear and reload entire configuration.
void InputEventController::ReloadConfig()
{
	allActionMaps.clear();
	activeMaps = 0;
	actionMap.clear();
	LoadConfig();
	LoadConsoleMap();
}

/// Load the configuration from the Config object.
void InputEventController::LoadConfig()
{
	// use the cfg_controls_t structure to load functors
	Config *cfg = Config::GetInstance();

	/* now we need to load the values */
	for(int i = 0; i < cfg->MAX_PLAYERS; i++) {
		// use map playerX
		stringstream str;
		str << _("Player") << " " << (i + 1);
		ActionMap& playerMap = allActionMaps[str.str()];

		playerMap[cfg->controlsHash[i].motorOn].reset(new EngineAction(_("Throttle"), 0, NULL));
		playerMap[cfg->controlsHash[i].brake].reset(new BrakeAction(_("Brake"), 1, NULL));
		playerMap[cfg->controlsHash[i].left].reset(new TurnLeftAction(_("Turn Left"), 2, NULL));
		playerMap[cfg->controlsHash[i].right].reset(new TurnRightAction(_("Turn Right"), 3, NULL));
		playerMap[cfg->controlsHash[i].jump].reset(new JumpAction(_("Jump"), 4, NULL));
		playerMap[cfg->controlsHash[i].fire].reset(new PowerupAction(_("Fire"), 5, NULL));
		playerMap[cfg->controlsHash[i].weapon].reset(new ChangeItemAction(_("Item"), 6, NULL));
		playerMap[cfg->controlsHash[i].lookBack].reset(new LookBackAction(_("Look Back"), 7, NULL));
	}

	//TODO: Load camera map
	/*
	ActionMap& cameraMap = allActionMaps[_("Camera")];
	*/
}

void InputEventController::RebindKey(string mapname, int oldhash, int newhash)
{
	// check map exists
	if(allActionMaps.count(mapname) > 0) {
		ActionMap& map = allActionMaps[mapname];
		// check anything exists at old hash
		if(map.count(oldhash) > 0) {
			ControlActionPtr tmp = map[oldhash];
			map.erase(oldhash);
			// check if we need to disable
			if(map.count(newhash) > 0) {
				ControlActionPtr tmp2 = map[newhash];
				map.erase(newhash);
				map[GetNextAvailableDisabledHash()] = tmp2;
			}
			map[newhash] = tmp; // bind old action to new control
		}
	}
}

/// Set up camera controls.
void InputEventController::LoadCameraMap()
{
	ActionMap& cmap = allActionMaps[_("Camera")];
	cmap.clear();

	Config* config = Config::GetInstance();

	AssignAction(cmap, config->cameraHash.zoomIn, actions.camera.zoomIn);
	AssignAction(cmap, config->cameraHash.zoomOut, actions.camera.zoomOut);
	AssignAction(cmap, config->cameraHash.panUp, actions.camera.panUp);
	AssignAction(cmap, config->cameraHash.panDown, actions.camera.panDown);
	AssignAction(cmap, config->cameraHash.reset, actions.camera.reset);
}

/// Set up menu controls.
void InputEventController::LoadMenuMap()
{
	ActionMap& cmap = allActionMaps[_("Menu")];
	cmap.clear();

	Config* config = Config::GetInstance();

	AssignAction(cmap, config->ui.menuOk, actions.ui.menuOk);
	AssignAction(cmap, config->ui.menuCancel, actions.ui.menuCancel);
	AssignAction(cmap, config->ui.menuExtra, actions.ui.menuExtra);
	AssignAction(cmap, config->ui.menuUp, actions.ui.menuUp);
	AssignAction(cmap, config->ui.menuDown, actions.ui.menuDown);
	AssignAction(cmap, config->ui.menuLeft, actions.ui.menuLeft);
	AssignAction(cmap, config->ui.menuRight, actions.ui.menuRight);
	AssignAction(cmap, config->ui.menuNext, actions.ui.menuNext);
	AssignAction(cmap, config->ui.menuPrev, actions.ui.menuPrev);
}

/// Set up the console toggle control.
void InputEventController::LoadConsoleToggleMap()
{
	ActionMap& cmap = allActionMaps[_("ConsoleToggle")];
	cmap.clear();

	Config* config = Config::GetInstance();

	AssignAction(cmap, config->ui.consoleToggle, actions.sys.consoleToggle);
}

/// Set up the console navigation controls.
void InputEventController::LoadConsoleMap()
{
	ActionMap& cmap = allActionMaps[_("Console")];
	cmap.clear();

	Config* config = Config::GetInstance();

	AssignAction(cmap, config->ui.consoleUp, actions.ui.consoleUp);
	AssignAction(cmap, config->ui.consoleDown, actions.ui.consoleDown);
	AssignAction(cmap, config->ui.consoleTop, actions.ui.consoleTop);
	AssignAction(cmap, config->ui.consoleBottom, actions.ui.consoleBottom);
	AssignAction(cmap, config->ui.consolePrevCmd, actions.ui.consolePrevCmd);
	AssignAction(cmap, config->ui.consoleNextCmd, actions.ui.consoleNextCmd);
}

// 0x0000xx00; xx = keycode
// 0x001yyy00; yyy = scancode as keycode
int InputEventController::HashKeyboardEvent(const SDL_Keycode& key)
{
	if (key & SDLK_SCANCODE_MASK) {
		ASSERT((key ^ SDLK_SCANCODE_MASK) < 0xfff);
		return (0x00100000 | (key & 0xfff)) << 8;
	}
	else {
		ASSERT(key <= 0xff);
		return (key % 0xff) << 8;
	}
}

// 0x004xx000; xx = mouse button
int InputEventController::HashMouseButtonEvent(const SDL_MouseButtonEvent& arg)
{
	return (0x00400000 | ((arg.button % 256) << 12));
}

// 0x005xy000; x = axis id, y = direction
int InputEventController::HashMouseAxisEvent(axis_t axis, int direction)
{
	return (0x00500000 | ((axis % 16) << 16) | ((direction % 16) << 12));
}

/*TODO
// 0x008xxyy0; x = joystick id, y = button id
int InputEventController::HashJoystickButtonEvent(const JoyStickEvent& arg, int button)
{
	return (0x00800000 | ((arg.device->getID() % 256) << 12) | ((button % 256) << 4));
}

// 0x009xxyy0; x = joystick id, y = slider id
int InputEventController::HashJoystickSliderEvent(const JoyStickEvent& arg, int slider)
{
	return (0x00900000 | ((arg.device->getID() % 256) << 12) | ((slider % 256) << 4));
}

// 0x00Axxyz0; x = joystick id, y = direction, z = pov
int InputEventController::HashJoystickPovEvent(const JoyStickEvent& arg, int pov, int direction)
{
	return (0x00A00000 | ((arg.device->getID() % 256) << 12) |
						 ((direction % 16) << 8) | ((pov % 16) << 4));
}

// 0x00Bxxyz0; x = joystick id, y = axis id, z = direction
int InputEventController::HashJoystickAxisEvent(const JoyStickEvent& arg, int axis, int direction)
{
	return (0x00B00000 | ((arg.device->getID() % 256) << 12) |
						 ((axis % 16) << 8) | ((direction % 16) << 4));
}
*/

} // namespace Control
} // namespace HoverRace
