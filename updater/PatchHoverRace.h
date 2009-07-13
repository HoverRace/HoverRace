// HoverRace Auto-Updater
// PatchHoverRace.h
//
// The actual code that is called to patch HoverRace with bsdiff-generated
// patches.
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

#ifndef PATCHHOVERRACE_H
#define PATCHHOVERRACE_H

#include "StdAfx.h"
#include <stdio.h>
#include <string>
#include "bsdiff/bspatch.h"

// for zip/unzip of patches
#include <LiteUnzip.h>

using namespace std;

int PatchHoverRace(string targetDir, string patchFile);

#endif