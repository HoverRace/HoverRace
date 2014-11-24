
#pragma once

namespace HoverRace {
	namespace ObjFacTools {
		class ResourceLib;
	}
	namespace Util {
		class ObjectFromFactory;
	}
}

namespace HoverRace {
namespace ObjFac1 {

class ObjFac1
{
public:
	ObjFac1();
	~ObjFac1();

	ObjFacTools::ResourceLib *GetResourceLib() const { return resourceLib; }

	Util::ObjectFromFactory *GetObject(int pClassId);

private:
	ObjFacTools::ResourceLib *resourceLib;
};

} // namespace ObjFac1
} // namespace HoverRace
