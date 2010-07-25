// ResActor.h
//

#ifndef MR_RES_ACTOR_H
#define MR_RES_ACTOR_H

#include "ResBitmap.h"
#include "../VideoServices/Viewport3D.h"

#ifdef MR_ENGINE
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

namespace HoverRace {
	namespace ObjFac1 {
		class ResActorFriend;
	}
	namespace ObjFacTools {
		class ResourceLib;
	}
	namespace Parcel {
		class ObjStream;
	}
}

namespace HoverRace {
namespace ObjFacTools {

class ResActor
{
	public:
		friend class HoverRace::ObjFac1::ResActorFriend;

		enum eComponentType { ePatch };

	protected:

		class MR_DllDeclare ActorComponent
		{
			public:

				virtual ~ ActorComponent();
				virtual eComponentType GetType() const = 0;
				virtual void Serialize(Parcel::ObjStream &pArchive, ResourceLib *pLib) = 0;
				virtual void Draw(VideoServices::Viewport3D *pDest, const VideoServices::PositionMatrix & pMatrix) const = 0;

		};

		class MR_DllDeclare Patch:public ActorComponent, public MR_Patch
		{
			public:
				int mURes;
				int mVRes;
				const MR_ResBitmap *mBitmap;
				MR_3DCoordinate *mVertexList;

				Patch();
				~Patch();

				eComponentType GetType() const;
				void Serialize(Parcel::ObjStream &pArchive, ResourceLib *pLib);
				void Draw(VideoServices::Viewport3D *pDest, const VideoServices::PositionMatrix & pMatrix) const;

				int GetURes() const;
				int GetVRes() const;
				const MR_3DCoordinate *GetNodeList() const;

		};

		class MR_DllDeclare Frame
		{
			public:
				int mNbComponent;
				ActorComponent **mComponentList;

				Frame();
				~Frame();
				void Clean();
				void Serialize(Parcel::ObjStream &pArchive, ResourceLib *pLib);
				void Draw(VideoServices::Viewport3D *pDest, const VideoServices::PositionMatrix & pMatrix) const;

		};

		class MR_DllDeclare Sequence
		{
			public:
				int mNbFrame;
				Frame *mFrameList;

				Sequence();
				~Sequence();
				void Serialize(Parcel::ObjStream &pArchive, ResourceLib *pLib);
				void Draw(VideoServices::Viewport3D * pDest, const VideoServices::PositionMatrix & pMatrix, int pFrame) const;

		};

		int mResourceId;
		int mNbSequence;
		Sequence *mSequenceList;

	public:
												  // Only availlable for resourceLib and construction
		MR_DllDeclare ResActor(int mResourceId);
		MR_DllDeclare ~ ResActor();

		MR_DllDeclare int GetResourceId() const;

		MR_DllDeclare int GetSequenceCount() const;
		MR_DllDeclare int GetFrameCount(int pSequence) const;

		MR_DllDeclare void Serialize(Parcel::ObjStream &pArchive, ResourceLib *pLib = NULL);
		MR_DllDeclare void Draw(VideoServices::Viewport3D * pDest, const VideoServices::PositionMatrix & pMatrix, int pSequence, int pFrame) const;

};

}  // namespace ObjFacTools
}  // namespace HoverRace

#undef MR_DllDeclare
#endif
