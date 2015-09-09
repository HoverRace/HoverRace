
// ElemBinding.cpp
//
// Copyright (c) 2015 Michael Imamura.
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

#include "../../../engine/Model/MazeElement.h"
#include "../../../engine/ObjFac1/BallElement.h"
#include "../../../engine/ObjFac1/BumperGate.h"
#include "../../../engine/ObjFac1/FuelSource.h"
#include "../../../engine/ObjFac1/Mine.h"
#include "../../../engine/ObjFac1/Missile.h"
#include "../../../engine/ObjFac1/PowerUp.h"
#include "../../../engine/Script/Core.h"
#include "../../../engine/Script/Wrapper.h"

#include "ElemBinding.h"

using namespace HoverRace::Model;
using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace Actor {

class Wrapper : public FreeElement, public Script::Wrapper
{
	using SUPER = FreeElement;

public:
	Wrapper() : SUPER({0, 0}) { }
	virtual ~Wrapper() { }
};

void LGetPos(const FreeElement &actor, lua_State *L)
{
	auto &pos = actor.mPosition;
	lua_pushnumber(L, pos.mX);
	lua_pushnumber(L, pos.mY);
	lua_pushnumber(L, pos.mZ);
}

void LSetPos(FreeElement &actor, MR_Int32 x, MR_Int32 y, MR_Int32 z)
{
	auto &pos = actor.mPosition;
	pos.mX = x;
	pos.mY = y;
	pos.mZ = z;
}

}  // namespace Actor

void ElemBinding::Register(Script::Core &scripting)
{
	using namespace luabind;
	using namespace HoverRace::ObjFac1;
	lua_State *L = scripting.GetState();

	module(L) [
		class_<FreeElement, Actor::Wrapper, std::shared_ptr<FreeElement>>("Actor")
			.def(constructor<>())
			.def("get_pos", &Actor::LGetPos)
			.def("set_pos", &Actor::LSetPos),
		class_<BallElement, FreeElement, std::shared_ptr<FreeElement>>("Ball"),
		class_<BumperGate, FreeElement, std::shared_ptr<FreeElement>>("BumperGate"),
		class_<FuelSource, FreeElement, std::shared_ptr<FreeElement>>("FuelSource"),
		class_<Mine, FreeElement, std::shared_ptr<FreeElement>>("Mine")
			.def(constructor<>()),
		class_<Missile, FreeElement, std::shared_ptr<FreeElement>>("Missile")
			.def(constructor<>()),
		class_<PowerUp, FreeElement, std::shared_ptr<FreeElement>>("PowerUp")
			.def(constructor<>())
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
