// ResContinuousSoundExt.cpp
// extension class for MR_ResShortSound

#include "ResContinuousSoundExt.h"

//ResContinuousSoundExt::ResContinuousSoundExt() {
	// nothing to do
//}

unsigned long ResContinuousSoundExt::GetDataLength() {
	return (unsigned long) mDataLen;
}

char *ResContinuousSoundExt::GetData() {
	return mData;
}

int ResContinuousSoundExt::GetNbCopy() {
	return mNbCopy;
}