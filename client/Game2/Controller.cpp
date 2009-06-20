// Controller.cpp
//
// This file contains the definition for the HoverRace::Client::Controller
// class.  It contains all the code to control input devices; joysticks, 
// keyboards, and the like.

#include "StdAfx.h"

#include "Controller.h"
#include "../../engine/Util/Config.h"

using namespace HoverRace::Client::Control;
using namespace HoverRace::Util;
using namespace OIS;

/***
 * Initialize the controller for player 1, loading configuration values.
 *
 * @param mainWindow Handle to the main HR window
 */
Controller::Controller(HWND mainWindow) {
	kbd = NULL;
	mouse = NULL;
	joys = NULL;

	InitInputManager(mainWindow);
	LoadControllerConfig();
}

/***
 * Clean up the Controller and everything in it.
 */
Controller::~Controller() {
	// simple cleanup... I like OIS
	InputManager::destroyInputSystem(mgr);
	delete[] joys;
}

/***
 * Save any control bindings that may have changed.
 */
void Controller::saveControls() {
	handler.saveControls();
}

/***
 * Poll the inputs for new input.  If there is input, it will call the correct handler.
 */
void Controller::poll() {
	try {
		if(kbd)
			kbd->capture();
		if(mouse)
			mouse->capture();
		if(joys) {
			for(int i = 0; i < numJoys; i++)
				joys[i]->capture();
		}
	} catch(Exception&) { // this should not happen, hence our error message
		MessageBox(NULL, "EXCEPTION YO SHIT", "SHIT SHIT", MB_OK);
	}
}

/***
 * Return the current control state.
 */
ControlState Controller::getState(int player) const {
	return handler.getControlState(player);
}

/***
 * Tell the Controller to take the next input and assign it to a control.
 *
 * @param control ID of the control (CTL_MOTOR_ON, ...)
 * @param player ID of the player; 0-3
 * @param hwnd Handle to the window we should contact when we are done
 */
void Controller::captureNextInput(int control, int player, HWND hwnd) {
	handler.captureNextInput(control, player, hwnd);
}

/***
 * Stop waiting to capture the next input.
 */
void Controller::stopCapture() {
	handler.stopCapture();
}

/***
 * Tell the Controller to disable the given control for the given player.
 *
 * @param control ID of the control (CTL_MOTOR_ON, ...)
 * @param player ID of the player; 0-3
 */
void Controller::disableInput(int control, int player) {
	handler.disableControl(control, player);
}

/***
 * Return a string representation of the input control.
 */
std::string Controller::toString(HoverRace::Util::Config::cfg_controls_t::cfg_control_t control) {
	if(control.inputType == OISKeyboard)
		return kbd->getAsString((KeyCode) control.kbdBinding);
	else if(control.inputType == OISUnknown) // cfg_control_t is set entirely to 0: disabled
		return "Disabled";
	else
		return "Something crazy"; // a mouse or joystick: TODO
}

/***
 * Initialize the InputManager object and set up all input devices.
 *
 * @param mainWindow Handle to the main HR window
 */
void Controller::InitInputManager(HWND mainWindow) {
	// collect parameters to give to init InputManager
	// just following the example here... I'm not 100% on this
	std::ostringstream wnd;
	wnd << (size_t) mainWindow;

	ParamList pl;
	pl.insert(std::make_pair(std::string("WINDOW"), wnd.str()));

	// mice
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));

	// this throws an exception on errors
	mgr = InputManager::createInputSystem(pl);

	// enable addons... I assume they will be useful
	mgr->enableAddOnFactory(InputManager::AddOn_All);

	// set up our input devices
	kbd = (Keyboard *) mgr->createInputObject(OISKeyboard, true);
	mouse = (Mouse *) mgr->createInputObject(OISMouse, true);

	// we can have more than one joystick
	numJoys = mgr->getNumberOfDevices(OISJoyStick);
	joys = new JoyStick *[numJoys];

	for(int i = 0; i < numJoys; i++) {
		joys[i] = NULL;

		try {
			joys[i] = (JoyStick *) mgr->createInputObject(OISJoyStick, true);
		} catch(OIS::Exception&) {
			ASSERT(false); // maybe some logging would be good here... #105
		}
	}

	// it is very nice that OIS does this for us!
	if(kbd)
		kbd->setEventCallback(&handler);
	else
		ASSERT(false); // this should be logged... wait until #105 is done

	if(mouse)
		kbd->setEventCallback(&handler);
	else
		ASSERT(false); // this should be logged... wait until #105 is done
	
	for(int i = 0; i < numJoys; i++) {
		if(joys[i])
			joys[i]->setEventCallback(&handler);
		else
			ASSERT(false); // this should be logged... wait until #105 is done
	}

	// now everything is set up and hopefully it will working forever!
}

/***
 * Load configuration values from HoverRace::Util::Config for a given
 * player.
 *
 * @param player Which player to load values for (0 through 3)
 */
void Controller::LoadControllerConfig() {
	Config *cfg = Config::GetInstance();

	/* now we need to load the values */
	handler.setControls(cfg->controls);
}

/***
 * Check whether or not the controls have been updated (for setting new controls)
 *
 * @return whether the controls have been updated
 */
bool Controller::controlsUpdated() {
	return handler.controlsUpdated();
}

/***
 * Set up the internal EventHandler class.
 */
Controller::EventHandler::EventHandler() {
	curState[0].brake = false;
	curState[0].fire = false;
	curState[0].jump = false;
	curState[0].left = false;
	curState[0].lookBack = false;
	curState[0].motorOn = false;
	curState[0].right = false;
	curState[0].weapon = false;

	curState[1].brake = false;
	curState[1].fire = false;
	curState[1].jump = false;
	curState[1].left = false;
	curState[1].lookBack = false;
	curState[1].motorOn = false;
	curState[1].right = false;
	curState[1].weapon = false;

	curState[2].brake = false;
	curState[2].fire = false;
	curState[2].jump = false;
	curState[2].left = false;
	curState[2].lookBack = false;
	curState[2].motorOn = false;
	curState[2].right = false;
	curState[2].weapon = false;

	curState[3].brake = false;
	curState[3].fire = false;
	curState[3].jump = false;
	curState[3].left = false;
	curState[3].lookBack = false;
	curState[3].motorOn = false;
	curState[3].right = false;
	curState[3].weapon = false;

	captureNext = false;
	captureControl = 0;
	capturePlayerId = 0;

	updated = false;
}

Controller::EventHandler::~EventHandler() {

}

bool Controller::EventHandler::keyPressed(const KeyEvent &arg) {
	updateControlState(arg.key, true);
	return true;
}

bool Controller::EventHandler::keyReleased(const KeyEvent &arg) {
	updateControlState(arg.key, false);
	return true;
}

bool Controller::EventHandler::mouseMoved(const MouseEvent &arg) {
	return true;
}

bool Controller::EventHandler::mousePressed(const MouseEvent &arg, MouseButtonID id) {
	return true;
}

bool Controller::EventHandler::mouseReleased(const MouseEvent &arg, MouseButtonID id) {
	return true;
}

bool Controller::EventHandler::buttonPressed(const JoyStickEvent &arg, int button) {
	return true;
}

bool Controller::EventHandler::buttonReleased(const JoyStickEvent &arg, int button) {
	return true;
}

bool Controller::EventHandler::axisMoved(const JoyStickEvent &arg, int axis) {
	return true;
}

bool Controller::EventHandler::povMoved(const JoyStickEvent &arg, int pov) {
	return true;
}

bool Controller::EventHandler::vector3Moved(const JoyStickEvent &arg, int index) {
	return true;
}

/***
 * Return the current control state.
 */
ControlState Controller::EventHandler::getControlState(int player) const {
	return (const ControlState) curState[player];
}

/***
 * Update the current control state, given the current key code and whether
 * or not the key just went down or up.
 *
 * @param keyCode Key code in question, that has just been pressed or released
 * @param pressed True if the key has been pressed, false if released (no other possibility)
 */
void Controller::EventHandler::updateControlState(int keyCode, bool pressed) {
	//MessageBox(NULL, "GOT SHIT", "YO", MB_OK);
 	if(captureNext) {
		// capture this input as a keycode
		Config *cfg = Config::GetInstance();

		InputControl *input = NULL;
		Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

		switch(captureControl) {
			case CTL_MOTOR_ON:
				input = &motorOn[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].motorOn;
				break;
			case CTL_LEFT:
				input = &left[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].left;
				break;
			case CTL_RIGHT:
				input = &right[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].right;
				break;
			case CTL_JUMP:
				input = &jump[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].jump;
				break;
			case CTL_BRAKE:
				input = &brake[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].brake;
				break;
			case CTL_FIRE:
				input = &fire[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].fire;
				break;
			case CTL_WEAPON:
				input = &weapon[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].weapon;
				break;
			case CTL_LOOKBACK:
				input = &lookBack[capturePlayerId];
				cfg_input = &cfg->controls[capturePlayerId].lookBack;
				break;
		}

		if((*input).kbdBinding != keyCode) {
			(*input).kbdBinding = keyCode;
			(*input).inputType = OISKeyboard;
			(*cfg_input).kbdBinding = keyCode;
			(*cfg_input).inputType = OISKeyboard;
			updated = true;
		}

		captureNext = false;
		captureControl = 0;
		capturePlayerId = 0;
		captureHwnd = NULL;
	} else {
		// TODO: make mouse and joystick input work
		if(keyCode == brake[0].kbdBinding) {
			curState[0].brake = pressed;
		} else if(keyCode == fire[0].kbdBinding) {
			curState[0].fire = pressed;
		} else if(keyCode == jump[0].kbdBinding) {
			curState[0].jump = pressed;
		} else if(keyCode == left[0].kbdBinding) {
			curState[0].left = pressed;
		} else if(keyCode == lookBack[0].kbdBinding) {
			curState[0].lookBack = pressed;
		} else if(keyCode == motorOn[0].kbdBinding) {
			curState[0].motorOn = pressed;
		} else if(keyCode == right[0].kbdBinding) {
			curState[0].right = pressed;
		} else if(keyCode == weapon[0].kbdBinding) {
			curState[0].weapon = pressed;
		} else if(keyCode == brake[1].kbdBinding) {
			curState[1].brake = pressed;
		} else if(keyCode == fire[1].kbdBinding) {
			curState[1].fire = pressed;
		} else if(keyCode == jump[1].kbdBinding) {
			curState[1].jump = pressed;
		} else if(keyCode == left[1].kbdBinding) {
			curState[1].left = pressed;
		} else if(keyCode == lookBack[1].kbdBinding) {
			curState[1].lookBack = pressed;
		} else if(keyCode == motorOn[1].kbdBinding) {
			curState[1].motorOn = pressed;
		} else if(keyCode == right[1].kbdBinding) {
			curState[1].right = pressed;
		} else if(keyCode == weapon[1].kbdBinding) {
			curState[1].weapon = pressed;
		} else if(keyCode == brake[2].kbdBinding) {
			curState[2].brake = pressed;
		} else if(keyCode == fire[2].kbdBinding) {
			curState[2].fire = pressed;
		} else if(keyCode == jump[2].kbdBinding) {
			curState[2].jump = pressed;
		} else if(keyCode == left[2].kbdBinding) {
			curState[2].left = pressed;
		} else if(keyCode == lookBack[2].kbdBinding) {
			curState[2].lookBack = pressed;
		} else if(keyCode == motorOn[2].kbdBinding) {
			curState[2].motorOn = pressed;
		} else if(keyCode == right[2].kbdBinding) {
			curState[2].right = pressed;
		} else if(keyCode == weapon[2].kbdBinding) {
			curState[2].weapon = pressed;
		} else if(keyCode == brake[3].kbdBinding) {
			curState[3].brake = pressed;
		} else if(keyCode == fire[3].kbdBinding) {
			curState[3].fire = pressed;
		} else if(keyCode == jump[3].kbdBinding) {
			curState[3].jump = pressed;
		} else if(keyCode == left[3].kbdBinding) {
			curState[3].left = pressed;
		} else if(keyCode == lookBack[3].kbdBinding) {
			curState[3].lookBack = pressed;
		} else if(keyCode == motorOn[3].kbdBinding) {
			curState[3].motorOn = pressed;
		} else if(keyCode == right[3].kbdBinding) {
			curState[3].right = pressed;
		} else if(keyCode == weapon[3].kbdBinding) {
			curState[3].weapon = pressed;
		} else {
			// not a relevant event
		}
	}
}

/***
 * Set the controls recognized by the EventHandler to a certain set of controls.
 * For now it only recognizes keyboard input, so the parameters are key codes.
 * The name of each parameter is the control it denotes.
 */
void Controller::EventHandler::setControls(HoverRace::Util::Config::cfg_controls_t *controls) {
	for(int i = 0; i < 4; i++) {
		this->brake[i] = toInputControl(controls[i].brake);
		this->fire[i] = toInputControl(controls[i].fire);
		this->jump[i] = toInputControl(controls[i].jump);
		this->left[i] = toInputControl(controls[i].left);
		this->lookBack[i] = toInputControl(controls[i].lookBack);
		this->motorOn[i] = toInputControl(controls[i].motorOn);
		this->right[i] = toInputControl(controls[i].right);
		this->weapon[i] = toInputControl(controls[i].weapon);
	}
}

/***
 * Store our controls back to the Config object
 */
void Controller::EventHandler::saveControls() {
	Config *cfg = Config::GetInstance();

	for(int i = 0; i < 4; i++) {
		cfg->controls[i].brake = toCfgControl(this->brake[i]);
		cfg->controls[i].fire = toCfgControl(this->fire[i]);
		cfg->controls[i].jump = toCfgControl(this->jump[i]);
		cfg->controls[i].left = toCfgControl(this->left[i]);
		cfg->controls[i].lookBack = toCfgControl(this->lookBack[i]);
		cfg->controls[i].motorOn = toCfgControl(this->motorOn[i]);
		cfg->controls[i].right = toCfgControl(this->right[i]);
		cfg->controls[i].weapon = toCfgControl(this->weapon[i]);
	}
}

/***
 * Tell the Controller to take the next input and assign it to a control.
 *
 * @param control ID of the control (CTL_MOTOR_ON, ...)
 * @param player ID of the player; 0-3
 * @param hwnd HWND to send a message to when the input is captured
 */
void Controller::EventHandler::captureNextInput(int control, int player, HWND hwnd) {
	captureNext = true;
	captureControl = control;
	capturePlayerId = player;
	captureHwnd = hwnd;
}

/***
 * Stop listening to capture the next input.
 */
void Controller::EventHandler::stopCapture() {
	captureNext = false;
	captureControl = 0;
	capturePlayerId = 0;
	captureHwnd = NULL;
}

/***
 * Tell the EventHandler to disable the given control for the given player.
 *
 * @param control ID of the control (CTL_MOTOR_ON, ...)
 * @param player ID of the player; 0-3
 */
void Controller::EventHandler::disableControl(int control, int player) {
	Config *cfg = Config::GetInstance();

	switch(control) {
		case CTL_MOTOR_ON:
			motorOn[player].inputType = OISUnknown;
			cfg->controls[player].motorOn.inputType = OISUnknown;
			break;
		case CTL_LEFT:
			left[player].inputType = OISUnknown;
			cfg->controls[player].left.inputType = OISUnknown;
			break;
		case CTL_RIGHT:
			right[player].inputType = OISUnknown;
			cfg->controls[player].right.inputType = OISUnknown;
			break;
		case CTL_JUMP:
			jump[player].inputType = OISUnknown;
			cfg->controls[player].jump.inputType = OISUnknown;
			break;
		case CTL_BRAKE:
			brake[player].inputType = OISUnknown;
			cfg->controls[player].brake.inputType = OISUnknown;
			break;
		case CTL_FIRE:
			fire[player].inputType = OISUnknown;
			cfg->controls[player].fire.inputType = OISUnknown;
			break;
		case CTL_WEAPON:
			weapon[player].inputType = OISUnknown;
			cfg->controls[player].weapon.inputType = OISUnknown;
			break;
		case CTL_LOOKBACK:
			lookBack[player].inputType = OISUnknown;
			cfg->controls[player].lookBack.inputType = OISUnknown;
			break;
	}
}

bool Controller::EventHandler::controlsUpdated() {
	return updated;
	updated = false;
}

InputControl Controller::EventHandler::toInputControl(HoverRace::Util::Config::cfg_controls_t::cfg_control_t control) {
	InputControl ret;

	ret.axis = control.axis;
	ret.button = control.button;
	ret.inputType = control.inputType;
	ret.kbdBinding = control.kbdBinding;
	ret.pov = control.pov;
	ret.sensitivity = control.sensitivity;
	ret.slider = control.slider;

	return ret;
}

HoverRace::Util::Config::cfg_controls_t::cfg_control_t Controller::EventHandler::toCfgControl(InputControl control) {
	HoverRace::Util::Config::cfg_controls_t::cfg_control_t ret;

	ret.axis = control.axis;
	ret.button = control.button;
	ret.inputType = control.inputType;
	ret.kbdBinding = control.kbdBinding;
	ret.pov = control.pov;
	ret.sensitivity = control.sensitivity;
	ret.slider = control.slider;

	return ret;
}