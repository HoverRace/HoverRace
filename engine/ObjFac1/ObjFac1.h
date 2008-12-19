
#pragma once

#include <string>

class MR_ObjectFromFactory;
class MR_ResourceLib;

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
				MR_ResourceLib* resourceLib;
		};

	} // namespace ObjFac1
} // namespace HoverRace
