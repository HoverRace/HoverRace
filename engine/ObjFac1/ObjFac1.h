
#pragma once

#include <string>

namespace HoverRace {
	namespace ObjFacTools {
		class ResourceLib;
	}
	namespace Util {
		class ObjectFromFactory;
	}
}

namespace HoverRace
{
	namespace ObjFac1
	{

		class ObjFac1
		{
			public:
				ObjFac1();
				~ObjFac1();

				HoverRace::Util::ObjectFromFactory *GetObject(int pClassId);

			private:
				HoverRace::ObjFacTools::ResourceLib* resourceLib;
		};

	} // namespace ObjFac1
} // namespace HoverRace
