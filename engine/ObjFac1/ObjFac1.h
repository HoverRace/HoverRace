
#pragma once

#include <string>

namespace HoverRace {
	namespace ObjFacTools {
		class ResourceLib;
	}
}

class MR_ObjectFromFactory;

namespace HoverRace
{
	namespace ObjFac1
	{

		class ObjFac1
		{
			public:
				ObjFac1();
				~ObjFac1();

				MR_ObjectFromFactory *GetObject(int pClassId);

			private:
				HoverRace::ObjFacTools::ResourceLib* resourceLib;
		};

	} // namespace ObjFac1
} // namespace HoverRace
