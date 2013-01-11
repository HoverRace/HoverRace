// ResShortSoundExt.cpp
// extension class for MR_ResShortSound

#include "ResShortSoundExt.h"

//ResShortSoundExt::ResShortSoundExt() {
	// nothing to do
//}

unsigned long ResShortSoundExt::GetDataLength() {
	return (unsigned long) mDataLen;
}

char *ResShortSoundExt::GetData() {
	return mData;
}

int ResShortSoundExt::GetNbCopy() {
	return mNbCopy;
}