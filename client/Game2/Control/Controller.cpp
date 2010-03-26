// Controller.cpp
//
// This file contains the definition for the HoverRace::Client::Controller
// class.  It contains all the code to control input devices; joysticks, 
// keyboards, and the like.

#include "StdAfx.h"

#include "InputHandler.h"

#include "Controller.h"

using namespace HoverRace;
using namespace HoverRace::Client::Control;
using namespace HoverRace::Util;
using namespace OIS;

#define DIVERT_TO_CONTROL_STACK \
	if (!controlLayers.empty()) \
		return controlLayers.back()


/***
 * Initialize the controller for player 1, loading configuration values.
 *
 * @param mainWindow Handle to the main HR window
 */
Controller::Controller(Util::OS::wnd_t mainWindow)
{
	kbd = NULL;
	mouse = NULL;
	joys = NULL;
	joyIds = NULL;

	captureNext = false;
	captureControl = 0;
	capturePlayerId = 0;

	mouseXLast = 0;
	mouseYLast = 0;
	mouseZLast = 0;

	updated = false;

	clearControlState(); // initialize to all false

	InitInputManager(mainWindow);
	LoadControllerConfig();
}

/***
 * Clean up the Controller and everything in it.
 */
Controller::~Controller() 
{
	// simple cleanup... I like OIS
	InputManager::destroyInputSystem(mgr);
	delete[] joys;
	delete[] joyIds;
}

/***
 * Save any control bindings that may have changed.
 */
void Controller::saveControls() 
{
	Config *cfg = Config::GetInstance();

	for(int i = 0; i < 4; i++) {
		cfg->controls[i].brake = this->brake[i];
		cfg->controls[i].fire = this->fire[i];
		cfg->controls[i].jump = this->jump[i];
		cfg->controls[i].left = this->left[i];
		cfg->controls[i].lookBack = this->lookBack[i];
		cfg->controls[i].motorOn = this->motorOn[i];
		cfg->controls[i].right = this->right[i];
		cfg->controls[i].weapon = this->weapon[i];
	}
}

/***
 * Poll the inputs for new input.  If there is input, it will call the correct handler.
 */
void Controller::poll() 
{
	try {
		if(kbd)
			kbd->capture();
		if(mouse) {
			// clear mouse inputs
			for(int i = 0; i < 4; i++) {
				if((brake[i].inputType == OISMouse) && (brake[i].axis != 0))
					curState[i].brake = false;
				if((fire[i].inputType == OISMouse) && (fire[i].axis != 0))
					curState[i].fire = false;
				if((jump[i].inputType == OISMouse) && (jump[i].axis != 0))
					curState[i].jump = false;
				if((left[i].inputType == OISMouse) && (left[i].axis != 0))
					curState[i].left = false;
				if((lookBack[i].inputType == OISMouse) && (lookBack[i].axis != 0))
					curState[i].lookBack = false;
				if((motorOn[i].inputType == OISMouse) && (motorOn[i].axis != 0))
					curState[i].motorOn = false;
				if((right[i].inputType == OISMouse) && (right[i].axis != 0))
					curState[i].right = false;
				if((weapon[i].inputType == OISMouse) && (weapon[i].axis != 0))
					curState[i].weapon = false;
			}
			mouse->capture();
		}
		if(joys) {
			for(int i = 0; i < numJoys; i++)
				joys[i]->capture();
		}
	} catch(Exception&) { // this should not happen, hence our error message
		ASSERT(FALSE);
	}
}

/***
 * Return the current control state.
 */
ControlState Controller::getControlState(int player)
{
	if (!controlLayers.empty())
		return curState[player];

	// update control state
	curState[player].brake		= getSingleControlState(brake[player]);
	curState[player].fire		= getSingleControlState(fire[player]);
	curState[player].jump		= getSingleControlState(jump[player]);
	curState[player].left		= getSingleControlState(left[player]);
	curState[player].lookBack	= getSingleControlState(lookBack[player]);
	curState[player].motorOn	= getSingleControlState(motorOn[player]);
	curState[player].right		= getSingleControlState(right[player]);
	curState[player].weapon		= getSingleControlState(weapon[player]);

	if(mouse) { // update mouse positions
		mouseXLast = mouse->getMouseState().X.abs;
		mouseYLast = mouse->getMouseState().Y.abs;
		mouseZLast = mouse->getMouseState().Z.abs;
	}

	return (const ControlState) curState[player];
}

/***
 * Get the control state of one particular input control.
 *
 * @return bool indicating whether or not the given control is activated
 */
bool Controller::getSingleControlState(InputControl input)
{
	bool ret = false;
	switch(input.inputType) {
		case OISKeyboard:
			ret = (kbd->isKeyDown((OIS::KeyCode) input.kbdBinding));
			break;
		case OISMouse:
			{
				MouseState ms = mouse->getMouseState();
				if(input.axis == 0) {
					// button
					ret = (ms.buttonDown((MouseButtonID) input.button));
				} else {
					// looking for movement
					switch(input.axis) {
						case AXIS_X:
							ret = ((input.direction == 1) ? (ms.X.abs > mouseXLast) : (ms.X.abs < mouseXLast));
							break;
						case AXIS_Y:
							ret = ((input.direction == 1) ? (ms.Y.abs > mouseYLast) : (ms.Y.abs < mouseYLast));
							break;
						case AXIS_Z:
							ret = ((input.direction == 1) ? (ms.Z.abs > mouseZLast) : (ms.Z.abs < mouseZLast));
							break;
					}
				}
			}
			break;
		case OISJoyStick:
			if(input.joystickId < numJoys) { // otherwise we will ignore the input (controller is not plugged in?)
				JoyStickState js = joys[input.joystickId]->getJoyStickState();
				if(input.axis != 0) {
					ret = ((input.direction == 1) ? (js.mAxes.at(input.axis - 1).abs > 5000) : (js.mAxes.at(input.axis - 1).abs < -5000));
				} else if(input.pov != 0) {
					ret = (input.direction == js.mPOV[input.pov].direction);
				} else if(input.slider != 0) {
					// not yet implemented
				} else { // button
					ret = js.mButtons.at(input.button);
				}
			}
			break;
	}

	// control must be disabled or something; return false
	return ret;
}

/***
 * Tell the Controller to take the next input and assign it to a control.
 *
 * @param control ID of the control (CTL_MOTOR_ON, ...)
 * @param player ID of the player; 0-3
 * @param hwnd Handle to the window we should contact when we are done
 */
void Controller::captureNextInput(int control, int player, Util::OS::wnd_t hwnd)
{
	captureNext = true;
	captureControl = control;
	capturePlayerId = player;
	captureHwnd = hwnd;
}

/***
 * Stop waiting to capture the next input.
 */
void Controller::stopCapture()
{
	captureNext = false;
	captureControl = 0;
	capturePlayerId = 0;
	captureHwnd = NULL;
}

/***
 * Tell the Controller to disable the given control for the given player.
 *
 * @param control ID of the control (CTL_MOTOR_ON, ...)
 * @param player ID of the player; 0-3
 */
void Controller::disableInput(int control, int player)
{
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

/***
 * Return a string representation of the input control.
 */
std::string Controller::toString(HoverRace::Util::Config::cfg_controls_t::cfg_control_t control)
{
	if(control.inputType == OISKeyboard) {
		return kbd->getAsString((OIS::KeyCode) control.kbdBinding);
	} else if(control.inputType == OISMouse) {
		if(control.axis == 0) {
			// OIS has no nice "getAsString" for mice so we have to do it by hand
			switch(control.button) {
				case MB_Left:
					return _("Left Mouse Btn");
				case MB_Right:
					return _("Right Mouse Btn");
				case MB_Middle:
					return _("Middle Mouse Btn");
				case MB_Button3:
					return _("Mouse Button 3");
				case MB_Button4:
					return _("Mouse Button 4");
				case MB_Button5:
					return _("Mouse Button 5");
				case MB_Button6:
					return _("Mouse Button 6");
				case MB_Button7:
					return _("Mouse Button 7");
				// apparently there is no support for more than 7 mouse buttons
				default:
					return _("Unknown Mouse Button");
			}
		} else {
			switch(control.axis) {
				case AXIS_X:
					if(control.direction == 1)
						return _("Mouse X+ Axis");
					else
						return _("Mouse X- Axis");
				case AXIS_Y:
					if(control.direction == 1)
						return _("Mouse Y+ Axis");
					else
						return _("Mouse Y- Axis");
				case AXIS_Z:
					if(control.direction == 1)
						return _("Mouse Z+ Axis");
					else
						return _("Mouse Z- Axis");
				default:
					if(control.direction == 1)
						return _("Unknown Mouse Axis +");
					else
						return _("Unknown Mouse Axis -");
			}
		}
	} else if(control.inputType == OISJoyStick) {
		char joynum[100];
		sprintf(joynum, "%s %d", _("Joystick"), control.joystickId);

		if(control.axis != 0) {
			// joystick axis
			char joyax[150];
			sprintf(joyax, "%s %s %d%c",
				joynum,
				_("Axis"),
				control.axis,
				(control.direction == 1) ? '+' : '-');
			return joyax;
		} else if(control.pov != 0) {
			// joystick pov
			char *povnum;
			switch(control.pov) { // try to use longer gettext phrases
				case 1:
					povnum = _("POV 1");
					break;
				case 2:
					povnum = _("POV 2");
					break;
				case 3:
					povnum = _("POV 3");
					break;
				case 4:
					povnum = _("POV 4");
					break;
			}
			// now the direction
			char *dir;
			switch(control.direction) {
				case Pov::Centered:
					dir = _("Center");
					break;
				case Pov::North:
					dir = _("North");
					break;
				case Pov::East:
					dir = _("East");
					break;
				case Pov::West:
					dir = _("West");
					break;
				case Pov::South:
					dir = _("South");
					break;
				case Pov::NorthEast:
					dir = _("Northeast");
					break;
				case Pov::NorthWest:
					dir = _("Northwest");
					break;
				case Pov::SouthEast:
					dir = _("Southeast");
					break;
				case Pov::SouthWest:
					dir = _("Southwest");
					break;
				default:
					dir = _("Unknown direction");
					break;
			}
			char ret[150];
			sprintf(ret, "%s %s %s\0", joynum, povnum, dir);
			return ret;
		} else if(control.slider != 0) {
			// joystick slider
			char slider[100];
			sprintf(slider, "%s %s %d", joynum, _("Slider"), control.slider);
			return slider;
		} else {
			// joystick button
			char ret[100];
			sprintf(ret, "%s %s %d\0", joynum, _("Button"), control.button);
			return ret;
		}
	} else if(control.inputType == OISUnknown) { // cfg_control_t is set entirely to 0: disabled
		return _("Disabled");
	} else {
		return "Something crazy"; // ??? should not happen
	}
}

/**
 * Push an input handler onto the control layer stack.
 * While this handler is at the top of the stack, it will receive all input events.
 * @param handler The handler (may not be @c NULL).
 * @see LeaveControlLayer()
 */
void Controller::EnterControlLayer(InputHandlerPtr handler)
{
	ASSERT(handler != NULL);
	controlLayers.push_back(handler);
}

/**
 * Pop an input handler from the control layer stack.
 */
void Controller::LeaveControlLayer()
{
	ASSERT(!controlLayers.empty());
	controlLayers.pop_back();
}

/**
 * Reset the control layer stack.
 * All input events will revert to controlling the player.
 */
void Controller::ResetControlLayers()
{
	controlLayers.clear();
}

/***
 * Initialize the InputManager object and set up all input devices.
 *
 * @param mainWindow Handle to the main HR window
 */
void Controller::InitInputManager(Util::OS::wnd_t mainWindow)
{
	// collect parameters to give to init InputManager
	// just following the example here... I'm not 100% on this
	std::ostringstream wnd;
	wnd.imbue(OS::stdLocale);
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
	joyIds = new int[numJoys];

	for(int i = 0; i < numJoys; i++) {
		joys[i] = NULL;

		try {
			joys[i] = (JoyStick *) mgr->createInputObject(OISJoyStick, true);
			joyIds[i] = ((Object *) joys[i])->getID();
		} catch(OIS::Exception&) {
			ASSERT(false); // maybe some logging would be good here... #105
		}
	}

	// it is very nice that OIS does this for us!
	if(kbd)
		kbd->setEventCallback(this);
	else
		ASSERT(false); // this should be logged... wait until #105 is done

	if(mouse) {
		mouse->setEventCallback(this);
		mouseXLast = mouse->getMouseState().X.abs;
		mouseYLast = mouse->getMouseState().Y.abs;
		mouseZLast = mouse->getMouseState().Z.abs;
	} else
		ASSERT(false); // this should be logged... wait until #105 is done
	
	for(int i = 0; i < numJoys; i++) {
		if(joys[i])
			joys[i]->setEventCallback(this);
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
void Controller::LoadControllerConfig()
{
	Config *cfg = Config::GetInstance();

	/* now we need to load the values */
	for(int i = 0; i < 4; i++) {
		this->brake[i] = cfg->controls[i].brake;
		this->fire[i] = cfg->controls[i].fire;
		this->jump[i] = cfg->controls[i].jump;
		this->left[i] = cfg->controls[i].left;
		this->lookBack[i] = cfg->controls[i].lookBack;
		this->motorOn[i] = cfg->controls[i].motorOn;
		this->right[i] = cfg->controls[i].right;
		this->weapon[i] = cfg->controls[i].weapon;
	}
}

/***
 * Check whether or not the controls have been updated (for setting new controls)
 *
 * @return whether the controls have been updated
 */
bool Controller::controlsUpdated()
{
	return updated;
	updated = false;	
}

bool Controller::keyPressed(const KeyEvent &arg)
{
	// modify modifier keys to reflect the same keycode; LShift == RShift, etc.
	OIS::KeyCode kc = arg.key;

	if(kc == KC_RSHIFT)
		kc = KC_LSHIFT;
	else if(kc == KC_RCONTROL)
		kc = KC_LCONTROL;
	else if(arg.key == KC_RMENU)
		kc = KC_LMENU;

	DIVERT_TO_CONTROL_STACK->KeyPressed(kc, arg.text);

	if(captureNext) {
		// capture this input as a keycode
		InputControl *input = NULL;
		Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

		getCaptureControl(captureControl, &input, &cfg_input);
		
		// if it is a new key, set the binding
		if(((*input).inputType != OISKeyboard) || ((*input).kbdBinding != kc)) {
			(*input).kbdBinding = kc;
			(*input).inputType = OISKeyboard;
			(*cfg_input).kbdBinding = kc;
			(*cfg_input).inputType = OISKeyboard;
			updated = true; // so we know if we need to say
		}

		// disable capture hook
		captureNext = false;
		captureControl = 0;
		capturePlayerId = 0;
		captureHwnd = NULL;
	} else {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISKeyboard) && (kc == brake[i].kbdBinding))
				curState[i].brake = true;
			if((fire[i].inputType == OISKeyboard) && (kc == fire[i].kbdBinding))
				curState[i].fire = true;
			if((jump[i].inputType == OISKeyboard) && (kc == jump[i].kbdBinding))
				curState[i].jump = true;
			if((left[i].inputType == OISKeyboard) && (kc == left[i].kbdBinding))
				curState[i].left = true;
			if((lookBack[i].inputType == OISKeyboard) && (kc == lookBack[i].kbdBinding))
				curState[i].lookBack = true;
			if((motorOn[i].inputType == OISKeyboard) && (kc == motorOn[i].kbdBinding))
				curState[i].motorOn = true;
			if((right[i].inputType == OISKeyboard) && (kc == right[i].kbdBinding))
				curState[i].right = true;
			if((weapon[i].inputType == OISKeyboard) && (kc == weapon[i].kbdBinding))
				curState[i].weapon = true;
		}
	}

	return true;
}

bool Controller::keyReleased(const KeyEvent &arg) {
	// modify modifier keys to reflect the same keycode; LShift == RShift, etc.
	OIS::KeyCode kc = arg.key;

	if(kc == KC_RSHIFT) {

		/**
		 * Combat absolutely bizarre DirectInput bug where pressing an arrow key when right shift is down
		 * causes a key-up notification for right shift to be sent; while DirectInput lies about this,
		 * GetKeyState() does not;
		 * References:
		 * 
		 * http://ogre3d.org/forums/viewtopic.php?f=2&t=17229
		 * http://forum.teamspeak.com/archive/index.php/t-90.html
		 * http://www.wreckedgames.com/forum/index.php?topic=893.0
		 */
#ifdef WIN32
		if((GetKeyState(VK_RSHIFT) & 0x80) > 0)
			return true; // ignore faulty event
#endif
		kc = KC_LSHIFT;
	}
	else if(kc == KC_RCONTROL)
		kc = KC_LCONTROL;
	else if(arg.key == KC_RMENU)
		kc = KC_LMENU;

	DIVERT_TO_CONTROL_STACK->KeyReleased(kc, arg.text);

	if(!captureNext) {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISKeyboard) && (kc == brake[i].kbdBinding))
				curState[i].brake = false;
			if((fire[i].inputType == OISKeyboard) && (kc == fire[i].kbdBinding))
				curState[i].fire = false;
			if((jump[i].inputType == OISKeyboard) && (kc == jump[i].kbdBinding))
				curState[i].jump = false;
			if((left[i].inputType == OISKeyboard) && (kc == left[i].kbdBinding))
				curState[i].left = false;
			if((lookBack[i].inputType == OISKeyboard) && (kc == lookBack[i].kbdBinding))
				curState[i].lookBack = false;
			if((motorOn[i].inputType == OISKeyboard) && (kc == motorOn[i].kbdBinding))
				curState[i].motorOn = false;
			if((right[i].inputType == OISKeyboard) && (kc == right[i].kbdBinding))
				curState[i].right = false;
			if((weapon[i].inputType == OISKeyboard) && (kc == weapon[i].kbdBinding))
				curState[i].weapon = false;
		}
	}

	return true;	
}

bool Controller::mouseMoved(const MouseEvent &arg)
{
	DIVERT_TO_CONTROL_STACK->MouseMoved(arg.state);

	// examine the axes to find the maximum
	int x, y, z, ax, ay, az;

	if(arg.state.X.absOnly)
		x = arg.state.X.abs;
	else
		x = arg.state.X.rel;

	if(arg.state.Y.absOnly)
		y = arg.state.Y.abs;
	else
		y = arg.state.Y.rel;

	if(arg.state.Z.absOnly)
		z = arg.state.Z.abs;
	else
		z = arg.state.Z.rel;

	// find magnitudes
	ax = abs(x);
	ay = abs(y);
	az = abs(z);

	if(captureNext) {
		InputControl *input = NULL;
		Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

		getCaptureControl(captureControl, &input, &cfg_input);

		(*input).inputType = OISMouse;
		(*input).button = 0;
		(*input).sensitivity = 1; // not touching this for now

		if((ax >= ay) && (ax >= az)) {
			// x axis is being set
			(*input).axis = AXIS_X;
			if(x > 0) {
				(*input).direction = 1;
				(*cfg_input).direction = 1;
			} else {
				(*input).direction = -1;
				(*cfg_input).direction = -1;
			}
		} else if((ay >= ax) && (ay >= az)) {
			// y axis
			(*input).axis = AXIS_Y;
			if(y > 0) {
				(*input).direction = 1;
				(*cfg_input).direction = 1;
			} else {
				(*input).direction = -1;
				(*cfg_input).direction = -1;
			}
		} else if((az >= ax) && (az >= ay)) {
			// z axis
			(*input).axis = AXIS_Z;
			if(z > 0) {
				(*input).direction = 1;
				(*cfg_input).direction = 1;
			} else {
				(*input).direction = -1;
				(*cfg_input).direction = -1;
			}
		}

		(*cfg_input).inputType = OISMouse;
		(*cfg_input).axis = (*input).axis;
		(*cfg_input).sensitivity = (*input).sensitivity;
		(*cfg_input).button = 0;

		updated = true;
	} else {
		for(int i = 0; i < 4; i++) {
			int axes[3] = { x, y, z };
			if((brake[i].inputType == OISMouse) && (brake[i].axis != 0))
				updateAxisControl(curState[i].brake, brake[i], axes, 3);
			if((fire[i].inputType == OISMouse) && (fire[i].axis != 0))
				updateAxisControl(curState[i].fire, fire[i], axes, 3);
			if((jump[i].inputType == OISMouse) && (jump[i].axis != 0))
				updateAxisControl(curState[i].jump, jump[i], axes, 3);
			if((left[i].inputType == OISMouse) && (left[i].axis != 0))
				updateAxisControl(curState[i].left, left[i], axes, 3);
			if((lookBack[i].inputType == OISMouse) && (lookBack[i].axis != 0))
				updateAxisControl(curState[i].lookBack, lookBack[i], axes, 3);
			if((motorOn[i].inputType == OISMouse) && (motorOn[i].axis != 0))
				updateAxisControl(curState[i].motorOn, motorOn[i], axes, 3);
			if((right[i].inputType == OISMouse) && (right[i].axis != 0))
				updateAxisControl(curState[i].right, right[i], axes, 3);
			if((weapon[i].inputType == OISMouse) && (weapon[i].axis != 0))
				updateAxisControl(curState[i].weapon, weapon[i], axes, 3);
		}
	}

	return true;
}

bool Controller::mousePressed(const MouseEvent &arg, MouseButtonID id)
{
	DIVERT_TO_CONTROL_STACK->MousePressed(arg.state, id);

	if(captureNext) {
		InputControl *input = NULL;
		Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

		getCaptureControl(captureControl, &input, &cfg_input);
		
		// if it is a new key, set the binding
		if(((*input).inputType != OISMouse) || ((*input).button != id)) {
			(*input).inputType = OISMouse;
			(*input).axis = 0;
			(*input).button = id;
			(*input).sensitivity = 0;
			(*cfg_input).inputType = OISMouse;
			(*cfg_input).axis = 0;
			(*cfg_input).button = id;
			(*cfg_input).sensitivity = 0;
			updated = true; // so we know if we need to say
		}

		// disable capture hook
		captureNext = false;
		captureControl = 0;
		capturePlayerId = 0;
		captureHwnd = NULL;
	} else {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISMouse) && (brake[i].button == id))
				curState[i].brake = true;
			if((fire[i].inputType == OISMouse) && (fire[i].button == id))
				curState[i].fire = true;
			if((jump[i].inputType == OISMouse) && (jump[i].button == id))
				curState[i].jump = true;
			if((left[i].inputType == OISMouse) && (left[i].button == id))
				curState[i].left = true;
			if((lookBack[i].inputType == OISMouse) && (lookBack[i].button == id))
				curState[i].lookBack = true;
			if((motorOn[i].inputType == OISMouse) && (motorOn[i].button == id))
				curState[i].motorOn = true;
			if((right[i].inputType == OISMouse) && (right[i].button == id))
				curState[i].right = true;
			if((weapon[i].inputType == OISMouse) && (weapon[i].button == id))
				curState[i].weapon = true;
		}
	}

	return true;
}

bool Controller::mouseReleased(const MouseEvent &arg, MouseButtonID id)
{
	DIVERT_TO_CONTROL_STACK->MouseReleased(arg.state, id);

	if(!captureNext) {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISMouse) && (brake[i].button == id))
				curState[i].brake = false;
			if((fire[i].inputType == OISMouse) && (fire[i].button == id))
				curState[i].fire = false;
			if((jump[i].inputType == OISMouse) && (jump[i].button == id))
				curState[i].jump = false;
			if((left[i].inputType == OISMouse) && (left[i].button == id))
				curState[i].left = false;
			if((lookBack[i].inputType == OISMouse) && (lookBack[i].button == id))
				curState[i].lookBack = false;
			if((motorOn[i].inputType == OISMouse) && (motorOn[i].button == id))
				curState[i].motorOn = false;
			if((right[i].inputType == OISMouse) && (right[i].button == id))
				curState[i].right = false;
			if((weapon[i].inputType == OISMouse) && (weapon[i].button == id))
				curState[i].weapon = false;
		}
	}

	return true;
}

bool Controller::buttonPressed(const JoyStickEvent &arg, int button)
{
	DIVERT_TO_CONTROL_STACK->ButtonPressed(arg.state, button);

	if(captureNext) {
		// capture this input as a key binding
		InputControl *input = NULL;
		Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

		getCaptureControl(captureControl, &input, &cfg_input);
		
		// if it is a new key, set the binding
		if(((*input).inputType != OISJoyStick) || ((*input).button != button)) {
			(*input).inputType = OISJoyStick;
			(*input).axis = 0;
			(*input).button = button;
			(*input).sensitivity = 0;
			(*input).pov = 0;
			(*input).joystickId = arg.device->getID();
			(*cfg_input).inputType = OISJoyStick;
			(*cfg_input).axis = 0;
			(*cfg_input).button = button;
			(*cfg_input).sensitivity = 0;
			(*cfg_input).pov = 0;
			(*cfg_input).joystickId = arg.device->getID();
			updated = true; // so we know if we need to say
		}

		// disable capture hook
		captureNext = false;
		captureControl = 0;
		capturePlayerId = 0;
		captureHwnd = NULL;
	} else {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISJoyStick) && (brake[i].joystickId == arg.device->getID()) && (brake[i].button == button))
				curState[i].brake = true;
			if((fire[i].inputType == OISJoyStick) && (fire[i].joystickId == arg.device->getID()) && (fire[i].button == button))
				curState[i].fire = true;
			if((jump[i].inputType == OISJoyStick) && (jump[i].joystickId == arg.device->getID()) && (jump[i].button == button))
				curState[i].jump = true;
			if((left[i].inputType == OISJoyStick) && (left[i].joystickId == arg.device->getID()) && (left[i].button == button))
				curState[i].left = true;
			if((lookBack[i].inputType == OISJoyStick) && (lookBack[i].joystickId == arg.device->getID()) && (lookBack[i].button == button))
				curState[i].lookBack = true;
			if((motorOn[i].inputType == OISJoyStick) && (motorOn[i].joystickId == arg.device->getID()) && (motorOn[i].button == button))
				curState[i].motorOn = true;
			if((right[i].inputType == OISJoyStick) && (right[i].joystickId == arg.device->getID()) && (right[i].button == button))
				curState[i].right = true;
			if((weapon[i].inputType == OISJoyStick) && (weapon[i].joystickId == arg.device->getID()) && (weapon[i].button == button))
				curState[i].weapon = true;
		}
	}
	return true;
}

bool Controller::buttonReleased(const JoyStickEvent &arg, int button)
{
	DIVERT_TO_CONTROL_STACK->ButtonReleased(arg.state, button);

	if(!captureNext) {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISJoyStick) && (brake[i].joystickId == arg.device->getID()) && (brake[i].button == button))
				curState[i].brake = false;
			if((fire[i].inputType == OISJoyStick) && (fire[i].joystickId == arg.device->getID()) && (fire[i].button == button))
				curState[i].fire = false;
			if((jump[i].inputType == OISJoyStick) && (jump[i].joystickId == arg.device->getID()) && (jump[i].button == button))
				curState[i].jump = false;
			if((left[i].inputType == OISJoyStick) && (left[i].joystickId == arg.device->getID()) && (left[i].button == button))
				curState[i].left = false;
			if((lookBack[i].inputType == OISJoyStick) && (lookBack[i].joystickId == arg.device->getID()) && (lookBack[i].button == button))
				curState[i].lookBack = false;
			if((motorOn[i].inputType == OISJoyStick) && (motorOn[i].joystickId == arg.device->getID()) && (motorOn[i].button == button))
				curState[i].motorOn = false;
			if((right[i].inputType == OISJoyStick) && (right[i].joystickId == arg.device->getID()) && (right[i].button == button))
				curState[i].right = false;
			if((weapon[i].inputType == OISJoyStick) && (weapon[i].joystickId == arg.device->getID()) && (weapon[i].button == button))
				curState[i].weapon = false;
		}
	}
	return true;
}

bool Controller::axisMoved(const JoyStickEvent &arg, int axis)
{
	DIVERT_TO_CONTROL_STACK->AxisMoved(arg.state, axis);

	// examine the axes to find the maximum
	int numAxes = arg.state.mAxes.size();
	int *axes = new int[numAxes];
	int *absAxes = new int[numAxes];

	for(int i = 0; i < numAxes; i++) {
		if(arg.state.mAxes.at(i).absOnly)
			axes[i] = arg.state.mAxes.at(i).abs;
		else
			axes[i] = arg.state.mAxes.at(i).rel;

		absAxes[i] = abs(axes[i]);

		// offset so that there is actually a dead zone somewhere
		// this should be configurable someday
		if(absAxes[i] < 5000) {
			axes[i] = 0;
			absAxes[i] = 0;
		}
	}

	if(captureNext) {
		int axisIndex = 0;
		for(int i = 1; i < numAxes; i++) {
			if(absAxes[i] >= absAxes[axisIndex])
				axisIndex = i;
		}

		if(axes[axisIndex] != 0) { // make sure we actually got usable input
			InputControl *input = NULL;
			Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

			getCaptureControl(captureControl, &input, &cfg_input);

			(*input).inputType = OISJoyStick;
			(*input).button = 0;
			(*input).sensitivity = 1; // not touching this for now
			(*input).pov = 0;
			(*input).slider = 0;
			(*input).axis = axisIndex + 1;
			(*input).joystickId = arg.device->getID();

			if(axes[axisIndex] > 0) {
				(*input).direction = 1;
				(*cfg_input).direction = 1;
			} else {
				(*input).direction = -1;
				(*cfg_input).direction = -1;
			}

			(*cfg_input).inputType = OISJoyStick;
			(*cfg_input).axis = (*input).axis;
			(*cfg_input).sensitivity = (*input).sensitivity;
			(*cfg_input).button = 0;
			(*cfg_input).slider = 0;
			(*cfg_input).pov = 0;
			(*cfg_input).joystickId = (*input).joystickId;

			updated = true;
			
			// disable capture hook
			captureNext = false;
			captureControl = 0;
			capturePlayerId = 0;
			captureHwnd = NULL;
		}
	} else {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISJoyStick) && (brake[i].joystickId == arg.device->getID()) && (brake[i].axis != 0))
				updateAxisControl(curState[i].brake, brake[i], axes, numAxes);
			if((fire[i].inputType == OISJoyStick) && (fire[i].joystickId == arg.device->getID()) && (fire[i].axis != 0))
				updateAxisControl(curState[i].fire, fire[i], axes, numAxes);
			if((jump[i].inputType == OISJoyStick) && (jump[i].joystickId == arg.device->getID()) && (jump[i].axis != 0))
				updateAxisControl(curState[i].jump, jump[i], axes, numAxes);
			if((left[i].inputType == OISJoyStick) && (left[i].joystickId == arg.device->getID()) && (left[i].axis != 0))
				updateAxisControl(curState[i].left, left[i], axes, numAxes);
			if((lookBack[i].inputType == OISJoyStick) && (lookBack[i].joystickId == arg.device->getID()) && (lookBack[i].axis != 0))
				updateAxisControl(curState[i].lookBack, lookBack[i], axes, numAxes);
			if((motorOn[i].inputType == OISJoyStick) && (motorOn[i].joystickId == arg.device->getID()) && (motorOn[i].axis != 0))
				updateAxisControl(curState[i].motorOn, motorOn[i], axes, numAxes);
			if((right[i].inputType == OISJoyStick) && (right[i].joystickId == arg.device->getID()) && (right[i].axis != 0))
				updateAxisControl(curState[i].right, right[i], axes, numAxes);
			if((weapon[i].inputType == OISJoyStick) && (weapon[i].joystickId == arg.device->getID()) && (weapon[i].axis != 0))
				updateAxisControl(curState[i].weapon, weapon[i], axes, numAxes);
		}
	}

	delete[] axes;
	delete[] absAxes;

	return true;
}

bool Controller::povMoved(const JoyStickEvent &arg, int pov)
{
	DIVERT_TO_CONTROL_STACK->PovMoved(arg.state, pov);

	pov++; // increment pov by one
	if(captureNext) {
		// capture this input as a key binding
		InputControl *input = NULL;
		Util::Config::cfg_controls_t::cfg_control_t *cfg_input = NULL;

		getCaptureControl(captureControl, &input, &cfg_input);
		
		// if it is a new key, set the binding
		if(((*input).inputType != OISJoyStick) || ((*input).pov != pov) || ((*input).direction != arg.state.mPOV[pov - 1].direction)) {
			(*input).inputType = OISJoyStick;
			(*input).axis = 0;
			(*input).button = -1;
			(*input).sensitivity = 0;
			(*input).pov = pov;
			(*input).direction = arg.state.mPOV[pov - 1].direction;
			(*input).joystickId = arg.device->getID();
			(*cfg_input).inputType = OISJoyStick;
			(*cfg_input).axis = 0;
			(*cfg_input).button = -1;
			(*cfg_input).sensitivity = 0;
			(*cfg_input).pov = pov;
			(*cfg_input).direction = arg.state.mPOV[pov - 1].direction;
			(*cfg_input).joystickId = arg.device->getID();
			updated = true; // so we know if we need to say
		}

		// disable capture hook
		captureNext = false;
		captureControl = 0;
		capturePlayerId = 0;
		captureHwnd = NULL;
	} else {
		for(int i = 0; i < 4; i++) {
			if((brake[i].inputType == OISJoyStick) && (brake[i].joystickId == arg.device->getID()) && (brake[i].pov == pov)) {
				if(brake[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].brake = true;
				else
					curState[i].brake = false;
			}
			if((fire[i].inputType == OISJoyStick) && (fire[i].joystickId == arg.device->getID()) && (fire[i].pov == pov)) {
				if(fire[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].fire = true;
				else
					curState[i].fire = false;
			}
			if((jump[i].inputType == OISJoyStick) && (jump[i].joystickId == arg.device->getID()) && (jump[i].pov == pov)) {
				if(jump[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].jump = true;
				else
					curState[i].jump = false;
			}
			if((left[i].inputType == OISJoyStick) && (left[i].joystickId == arg.device->getID()) && (left[i].pov == pov)) {
				if(left[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].left = true;
				else
					curState[i].left = false;
			}
			if((lookBack[i].inputType == OISJoyStick) && (lookBack[i].joystickId == arg.device->getID()) && (lookBack[i].pov == pov)) {
				if(lookBack[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].lookBack = true;
				else
					curState[i].lookBack = false;
			}
			if((motorOn[i].inputType == OISJoyStick) && (motorOn[i].joystickId == arg.device->getID()) && (motorOn[i].pov == pov)) {
				if(motorOn[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].motorOn = true;
				else
					curState[i].motorOn = false;
			}
			if((right[i].inputType == OISJoyStick) && (right[i].joystickId == arg.device->getID()) && (right[i].pov == pov)) {
				if(right[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].right = true;
				else
					curState[i].right = false;
			}
			if((weapon[i].inputType == OISJoyStick) && (weapon[i].joystickId == arg.device->getID()) && (weapon[i].pov == pov)) {
				if(weapon[i].direction == arg.state.mPOV[pov - 1].direction)
					curState[i].weapon = true;
				else
					curState[i].weapon = false;
			}
		}
	}
	return true;
}

/***
 * Clear the control state of each player.  This is done each time Controller::poll() is
 * called.
 */
void Controller::clearControlState()
{
	memset(curState, 0, sizeof(ControlState));
}

/**
 * Set the pointers given to the correct InputControl and cfg_control_t.
 *
 * @param captureControl ID of control
 * @param input Pointer to InputControl that will be set to the control specified in captureControl
 * @param cfg_input Pointer to cfg_control_t that will be set to the control specified in captureControl
 */
void Controller::getCaptureControl(int captureControl, InputControl **input, HoverRace::Util::Config::cfg_controls_t::cfg_control_t **cfg_input)
{
	Config *cfg = Config::GetInstance();

	switch(captureControl) {
		case CTL_MOTOR_ON:
			*input = &motorOn[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].motorOn;
			return;
		case CTL_LEFT:
			*input = &left[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].left;
			return;
		case CTL_RIGHT:
			*input = &right[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].right;
			return;
		case CTL_JUMP:
			*input = &jump[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].jump;
			return;
		case CTL_BRAKE:
			*input = &brake[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].brake;
			return;
		case CTL_FIRE:
			*input = &fire[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].fire;
			return;
		case CTL_WEAPON:
			*input = &weapon[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].weapon;
			return;
		case CTL_LOOKBACK:
			*input = &lookBack[capturePlayerId];
			*cfg_input = &cfg->controls[capturePlayerId].lookBack;
			return;
	}
}

/**
 * Update the control state given the variable to update, the input control to inspect, and the axes measurements.
 *
 * @param ctlState The variable to modify given whether the input is on or off
 * @param ctl The InputControl to inspect
 * @param axes An array of axes measurements
 * @param numAxes The number of axes
 */
void Controller::updateAxisControl(bool &ctlState, InputControl &ctl, int *axes, int numAxes)
{
	for(int i = 0; i < numAxes; i++) {
		if((ctl.axis - 1) == i)
			ctlState = ((ctl.direction == 1) ? (axes[i] > 0) : (axes[i] < 0));
	}
}