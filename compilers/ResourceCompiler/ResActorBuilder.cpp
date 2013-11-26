// ResActorBuilder.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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
//

#include "StdAfx.h"

#include "../../engine/Exception.h"

#include "ResActorBuilder.h"
#include "TextParsing.h"

using HoverRace::ObjFacTools::ResourceLib;

#define MR_MAX_SEQUENCE   1024
#define MR_MAX_FRAME      2048

namespace HoverRace {
namespace ResourceCompiler {

namespace {
	// Temporary until this is rewritten.
	char *sfgets(char *s, int sz, FILE *file)
	{
		char *retv = fgets(s, sz, file);
		if (!retv) {
			if (feof(file)) {
				throw Exception("Unexpected end of file");
			}
			else if (ferror(file)) {
				throw Exception("Read error");
			}
			else {
				throw Exception("Unknown read error");
			}
		}
		return retv;
	}
}

ResActorBuilder::ResActorBuilder(int pResourceId) :
	SUPER(pResourceId)
{

}

BOOL ResActorBuilder::BuildFromFile(const char *pFile, ResourceLib *pBitmapLib)
{
	int lCounter;
	BOOL lReturnValue = TRUE;

	FILE *lFile = fopen(pFile, "r");

	if(lFile == NULL) {
		fprintf(stderr, "%s: %s (%s).\n", _("ERROR"), _("unable to open actor file"), pFile);
		lReturnValue = FALSE;
	}
	else {
		char lBuffer[250];
		char *lBufferPtr;

		int lFrameCount[MR_MAX_SEQUENCE];
		int lComponentCount[MR_MAX_FRAME];

		int lCurrentFrame = -1;

		ActorComponent *lCurrentComponent = NULL;
		typedef std::vector<ActorComponent*> lComponentList_t;
		lComponentList_t lComponentList;

		for(lCounter = 0; lCounter < MR_MAX_SEQUENCE; lCounter++) {
			lFrameCount[lCounter] = 0;
		}

		for(lCounter = 0; lCounter < MR_MAX_FRAME; lCounter++) {
			lComponentCount[lCounter] = 0;
		}

		// Load the sequences
		lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);

		while(lReturnValue && lBufferPtr) {
			if(MR_BeginByKeyword(lBuffer, "SEQUENCE")) {
				mNbSequence++;

				// Load the Frames
				lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);

				while(lReturnValue && lBufferPtr) {
					if(MR_BeginByKeyword(lBuffer, "FRAME")) {
						lFrameCount[mNbSequence - 1] += 1;
						lCurrentFrame++;

						// Load the parts
						lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);

						while(lReturnValue && lBufferPtr) {

							lCurrentComponent = NULL;

							if(MR_BeginByKeyword(lBuffer, "PATCH")) {
								lCurrentComponent = ReadPatch(lFile, pBitmapLib);
							}
							else if(MR_BeginByKeyword(lBuffer, "POLYGON")) {
								ASSERT(FALSE);
							}
							else if(MR_BeginByKeyword(lBuffer, "FRAME")) {
								break;
							}
							else if(MR_BeginByKeyword(lBuffer, "SEQUENCE")) {
								break;
							}
							else {
								lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);
							}

							if(lCurrentComponent != NULL) {
								lComponentCount[lCurrentFrame]++;
								lComponentList.push_back(lCurrentComponent);
								lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);
							}
						}
					}

					if(MR_BeginByKeyword(lBuffer, "FRAME")) {
					}
					else if(MR_BeginByKeyword(lBuffer, "SEQUENCE")) {
						break;
					}
					else {
						lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);
					}
				}
			}
			else {
				lBufferPtr = sfgets(lBuffer, sizeof(lBuffer), lFile);
			}
		}
		fclose(lFile);

		if(lReturnValue) {
			// Construct the real actor
			lComponentList_t::iterator componentListIter = lComponentList.begin();

			mSequenceList = new Sequence[mNbSequence];

			lCurrentFrame = 0;

			for(int lSequence = 0; lSequence < mNbSequence; lSequence++) {
				mSequenceList[lSequence].mNbFrame = lFrameCount[lSequence];
				mSequenceList[lSequence].mFrameList = new Frame[lFrameCount[lSequence]];

				for(int lFrame = 0; lFrame < lFrameCount[lSequence]; lFrame++) {
					mSequenceList[lSequence].mFrameList[lFrame].mNbComponent = lComponentCount[lCurrentFrame];
					mSequenceList[lSequence].mFrameList[lFrame].mComponentList = new ActorComponent *[lComponentCount[lCurrentFrame]];

					for(int lComponent = 0; lComponent < lComponentCount[lCurrentFrame]; lComponent++) {
						mSequenceList[lSequence].mFrameList[lFrame].mComponentList[lComponent] = *componentListIter;
						++componentListIter;
					}
					lCurrentFrame++;
				}
			}
		}
		else {
			// Delete the lists
			for (lComponentList_t::iterator iter = lComponentList.begin();
				iter != lComponentList.end(); ++iter)
			{
				delete *iter;
			}
		}
	}

	return lReturnValue;
}

ResActorBuilder::Patch * ResActorBuilder::ReadPatch(FILE * pFile, ResourceLib * pBitmapLib)
{
	Patch *lReturnValue = new Patch;
	char lBuffer[250];

	sfgets(lBuffer, sizeof(lBuffer), pFile);
	sscanf(lBuffer, " %d %d", &(lReturnValue->mURes), &(lReturnValue->mVRes));

	lReturnValue->mVertexList = new MR_3DCoordinate[lReturnValue->mURes * lReturnValue->mVRes];

	for(int lCounter = 0; lCounter < (lReturnValue->mURes * lReturnValue->mVRes); lCounter++) {
		int lX;
		int lY;
		int lZ;

		sfgets(lBuffer, sizeof(lBuffer), pFile);
		sscanf(lBuffer, " %d %d %d", &lX, &lY, &lZ);

		lReturnValue->mVertexList[lCounter].mX = lX;
		lReturnValue->mVertexList[lCounter].mY = lY;
		lReturnValue->mVertexList[lCounter].mZ = lZ;
	}

	// Load the bitmap id
	int lBitmapId;

	sfgets(lBuffer, sizeof(lBuffer), pFile);

	if(sscanf(MR_PreProcLine(lBuffer).c_str(), " %d ", &lBitmapId) != 1) {
		delete lReturnValue;
		lReturnValue = FALSE;

		fprintf(stderr, "%s: %s.\n", _("ERROR"), _("No bitmap associated with the patch"));
	}
	else {
		lReturnValue->mBitmap = pBitmapLib->GetBitmap(lBitmapId);

		if(lReturnValue->mBitmap == NULL) {
			delete lReturnValue;
			lReturnValue = FALSE;

			fprintf(stderr, "%s: %s.\n", _("ERROR"), _("Bad bitmap ID"));
		}
	}

	return lReturnValue;
}

}  // namespace ResourceCompiler
}  // namespace HoverRace
