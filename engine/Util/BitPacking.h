// BitPacking.h
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

#pragma once

#include "MR_Types.h"

namespace HoverRace {
namespace Util {

template<int BYTES>
struct BitPack {
	static const int SIZE = BYTES;

	// Need three bytes of padding to be safe in case we write
	// to the array starting at the last byte.
	static const int REALSIZE = BYTES + 3;
	MR_UInt8 mData[REALSIZE];

	void InitFrom(const MR_UInt8 *data);

	void Clear();
	void Set(int pOffset, int pLen, int pPrecision, MR_Int32 pValue);

	MR_Int32 Get(int pOffset, int pLen, int pPrecision) const;
	MR_UInt32 Getu(int pOffset, int pLen, int pPrecision) const;
};

static_assert(std::is_pod<BitPack<32>>::value, "BitPack must be a POD type");

template<int BYTES>
void BitPack<BYTES>::InitFrom(const MR_UInt8 *data) {
	memcpy(mData, data, SIZE);
}

template<int BYTES>
MR_Int32 BitPack<BYTES>::Get(int pOffset, int pLen, int pPrecision) const
{
	union {
		MR_Int32 s;
		MR_UInt32 u;
	} lReturnValue;

	lReturnValue.u = (*(MR_UInt32 *) (mData + pOffset / 8)) >> (pOffset % 8);

	if((pLen + (pOffset % 8)) > 32) {
		lReturnValue.u |= (*(MR_UInt32 *) (mData + (pOffset / 8) + 4)) << (32 - (pOffset % 8));
	}
	// Clear overhead bits and preserve sign
	lReturnValue.s = lReturnValue.s << (32 - pLen) >> (32 - pLen);

	return lReturnValue.s << pPrecision;
}

template<int BYTES>
MR_UInt32 BitPack<BYTES>::Getu(int pOffset, int pLen, int pPrecision) const
{
	MR_UInt32 lReturnValue;

	lReturnValue = (*(MR_UInt32 *) (mData + pOffset / 8)) >> (pOffset % 8);

	if(pLen + (pOffset % 8) > 32) {
		lReturnValue |= (*(MR_UInt32 *) (mData + (pOffset / 8) + 4)) << (32 - (pOffset % 8));
	}
	// Clear overhead bits
	lReturnValue = lReturnValue << (32 - pLen) >> (32 - pLen);

	return lReturnValue << pPrecision;
}

template<int BYTES>
void BitPack<BYTES>::Clear()
{
	memset(mData, 0, REALSIZE);
}

template<int BYTES>
inline void BitPack<BYTES>::Set(int pOffset, int pLen, int pPrecision, MR_Int32 pValue)
{
	MR_UInt32 lValue = pValue >> pPrecision;

	lValue = lValue << (32 - pLen) >> (32 - pLen);

	(*(MR_UInt32 *) (mData + pOffset / 8)) |= lValue << (pOffset % 8);

	if((pLen + (pOffset % 8)) > 32) {
		(*(MR_UInt32 *) (mData + (pOffset / 8) + 4)) |= lValue >> (32 - (pOffset % 8));
	}
}

}  // namespace Util
}  // namespace HoverRace
