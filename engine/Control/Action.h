
// Action.h
//
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

#pragma once

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>

#include "../Vec.h"
#include "../Exception.h"

#include "ControlAction.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Control {

namespace TextControl {

/// Keycodes used for text input control.
enum key_t {
	BACKSPACE = SDLK_BACKSPACE,
	ENTER = SDLK_RETURN
};

}  // namepsace TextControl

namespace Mouse {

/// Mouse buttons.
enum button_t {
	LEFT = SDL_BUTTON_LEFT,
	MIDDLE = SDL_BUTTON_MIDDLE,
	RIGHT = SDL_BUTTON_RIGHT,
	B4 = SDL_BUTTON_X1,
	B5 = SDL_BUTTON_X2
};

/// Mouse click events.
struct Click {
	Click(double x, double y, button_t btn) : pos(x, y), btn(btn) { }
	Vec2 pos;
	button_t btn;
};

}  // namespace Mouse

/// Signals which are self-contained (no payload).
using voidSignal_t = boost::signals2::signal<void()>;

/// Signals which have a single (action-dependent) payload.
using valueSignal_t = boost::signals2::signal<void(int)>;

/// Signals which have a single string payload.
using stringSignal_t = boost::signals2::signal<void(const std::string&)>;

/// Signals for text input control.
using textControlSignal_t = boost::signals2::signal<void(TextControl::key_t)>;

/// Signals with have a Vec2 payload.
using vec2Signal_t = boost::signals2::signal<void(const Vec2&)>;

/// Signals for mouse clicks.
using mouseClickSignal_t = boost::signals2::signal<void(const Mouse::Click&)>;

template<class T, class Val>
inline void PerformAction(const T&, Val)
{
	std::ostringstream oss;
	oss << "PerformAction<" << typeid(T).name() << ", " <<
		typeid(Val).name() << '>';
	throw UnimplementedExn(oss.str());
}

template<>
inline void PerformAction<voidSignal_t, int>(const voidSignal_t &signal,
                                             int value)
{
	// voidSignals are only triggered if the value is > 0.
	// For keys and buttons, that means key-down / button-down.
	if (value > 0) {
		signal();
	}
}

template<>
inline void PerformAction<valueSignal_t, int>(const valueSignal_t &signal,
                                              int value)
{
	signal(value);
}

template<>
inline void PerformAction<stringSignal_t, const std::string &>(
	const stringSignal_t &signal, const std::string &s)
{
	signal(s);
}

template<>
inline void PerformAction<textControlSignal_t, TextControl::key_t>(
	const textControlSignal_t &signal, TextControl::key_t key)
{
	signal(key);
}

template<>
inline void PerformAction<vec2Signal_t, const Vec2&>(
	const vec2Signal_t &signal, const Vec2 &vec)
{
	signal(vec);
}

template<>
inline void PerformAction<mouseClickSignal_t, const Mouse::Click&>(
	const mouseClickSignal_t &signal, const Mouse::Click &vec)
{
	signal(vec);
}

/**
 * Actions are effectively signals that can be triggered by both the
 * InputEventController and other things such as UI buttons.
 *
 * Actions also keep track of what key/button is assigned to it for UI
 * components to display to the user.
 *
 * @tparam T The signal type.
 * @tparam Val The type of the value passed to the signal.
 * @author Michael Imamura
 */
template<class T, class Val=int>
class Action : public ControlAction<Val>
{
	using SUPER = ControlAction<Val>;

public:
	Action(const std::string &name, int listOrder) :
		SUPER(name, listOrder), primaryTrigger(0) { }
	virtual ~Action() { }

	virtual void operator()(Val value) {
		PerformAction<T, Val>(signal, value);
	}

	/**
	 * Connect a slot to the signal.
	 * This is a convenience function.  To access the signal directly,
	 * see GetSignal().
	 */
	template<class U>
	boost::signals2::connection Connect(U &&del) {
		return signal.connect(std::forward<U>(del));
	}

	/**
	 * Retrieve the hash of the key or button assigned to this action by
	 * the InputEventController.
	 * @return The hash (see InputEventController::HashToString) or 0 if no key
	 *         or button has been assigned.
	 */
	int GetPrimaryTrigger() const { return primaryTrigger; }

	/**
	 * Set the hash of the key or button assigned to this action.
	 * This should only be called by the InputEventController that owns this
	 * Action (if any).
	 * @param hash The input hash or 0 if none is assigned.
	 */
	void SetPrimaryTrigger(int hash) { primaryTrigger = hash; }

	T &GetSignal() { return signal; }

private:
	int primaryTrigger;
	T signal;
};

} // namespace Control
} // namespace HoverRace

#undef MR_DllDeclare
