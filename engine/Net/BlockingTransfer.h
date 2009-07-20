
// BlockingTransfer.h
// Header for the simple blocking transfer.
//
// Copyright (c) 2009 Michael Imamura.
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

#pragma once

#include "BaseTransfer.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Net {

/**
 * This is used for the simple forms of transfer, only used in Agent::Get.
 * @author Michael Imamura
 */
class MR_DllDeclare BlockingTransfer : public BaseTransfer
{
	typedef BaseTransfer SUPER;
	public:
		BlockingTransfer(const Agent &agent, std::string &buf);
		BlockingTransfer(const Agent &agent, std::ostream &buf);
		virtual ~BlockingTransfer() { }
	private:
		void Init();

	public:
		virtual bool IsComplete() const { return true; }

	public:
		void Go();

	private:
		static size_t StringWriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);
		static size_t StreamWriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);
};

}  // namespace Net
}  // namespace HoverRace

#undef MR_DllDeclare
