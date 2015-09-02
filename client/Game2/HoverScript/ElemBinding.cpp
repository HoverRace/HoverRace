
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

#include "ElemBinding.h"

namespace HoverRace {
namespace Client {
namespace HoverScript {

void ElemBinding::Register(Script::Core &scripting)
{
	using namespace luabind;
	using namespace HoverRace::Model;
	using namespace HoverRace::ObjFac1;
	lua_State *L = scripting.GetState();

	module(L) [
		class_<FreeElement, std::shared_ptr<FreeElement>>("Actor")
			.def(constructor<>()),
		class_<BallElement, FreeElement, std::shared_ptr<FreeElement>>("Ball"),
		class_<BumperGate, FreeElement, std::shared_ptr<FreeElement>>("BumperGate"),
		class_<FuelSource, FreeElement, std::shared_ptr<FreeElement>>("FuelSource"),
		class_<Mine, FreeElement, std::shared_ptr<FreeElement>>("Mine"),
		class_<Missile, FreeElement, std::shared_ptr<FreeElement>>("Missile"),
		class_<PowerUp, FreeElement, std::shared_ptr<FreeElement>>("PowerUp")
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
