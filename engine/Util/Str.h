
// Str.h
//
// Copyright (c) 2009, 2014 Michael Imamura.
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

#include "OS.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Util {

namespace Str {

	MR_DllDeclare wchar_t *Utf8ToWide(const char *s);
	MR_DllDeclare char *WideToUtf8(const wchar_t *ws);

	/** Utility class for easy conversion of UTF-8 to wide strings. */
	class MR_DllDeclare UW
	{
		wchar_t *cs;
		public:
			UW(const char *s=NULL) throw() : cs(Utf8ToWide(s)) { }
			explicit UW(const std::string &s) throw() : cs(Utf8ToWide(s.c_str())) { }
			~UW() throw() { OS::Free(cs); }
			operator const wchar_t*() const throw() { return cs; }
			operator const std::wstring() const { return cs; }
	};

	/** Utility class for easy conversion of wide strings to UTF-8. */
	class MR_DllDeclare WU
	{
		char *cs;
		public:
			WU(const wchar_t *ws=NULL) throw() : cs(WideToUtf8(ws)) { }
			explicit WU(const std::wstring &s) throw() : cs(WideToUtf8(s.c_str())) { }
			~WU() throw() { OS::Free(cs); }
			operator const char*() const throw() { return cs; }
			operator const std::string() const { return cs; }
	};

	// Convert UTF-8 to path_t.
#	ifdef WITH_WIDE_PATHS
		inline OS::path_t UP(const char *s=NULL) throw() { return OS::path_t((const wchar_t*)Str::UW(s)); }
		inline OS::path_t UP(const std::string &s) throw() { return OS::path_t((const wchar_t*)Str::UW(s)); }
#	else
		inline OS::path_t UP(const char *s=NULL) throw() { return OS::path_t(s); }
		inline OS::path_t UP(const std::string &s) throw() { return OS::path_t(s); }
#	endif

	// Convert path_t to UTF-8.
	class MR_DllDeclare PU
	{
#		ifdef WITH_WIDE_PATHS
			WU cs;
			public:
#				if BOOST_FILESYSTEM_VERSION == 2
					explicit PU(const OS::path_t &path) throw() : cs(path.file_string()) { }
#				else
					explicit PU(const OS::path_t &path) throw() : cs(path.wstring()) { }
#				endif
				operator const char*() const throw() { return (const char*)cs; }
				operator const std::string() const { return std::string((const char*)cs); }
#		else
			std::string cs;
			public:
#				if BOOST_FILESYSTEM_VERSION == 2
					explicit PU(const OS::path_t &path) throw() : cs(path.file_string()) { }
#				else
					explicit PU(const OS::path_t &path) throw() : cs(path.string()) { }
#				endif
				operator const char*() const throw() { return cs.c_str(); }
				operator const std::string() const { return cs; }
#		endif
	};

	// Convert wide strings to path_t.
#	ifdef WITH_WIDE_PATHS
		inline OS::path_t WP(const wchar_t *s=NULL) throw() { return OS::path_t(s); }
		inline OS::path_t WP(const std::wstring &s) throw() { return OS::path_t(s); }
#	else
		inline OS::path_t WP(const wchar_t *s=NULL) throw() { return OS::path_t((const char*)Str::WU(s)); }
		inline OS::path_t WP(const std::wstring &s) throw() { return OS::path_t((const char*)Str::WU(s)); }
#	endif

	// Convert path_t to wide string.
	class MR_DllDeclare PW
	{
#		ifdef WITH_WIDE_PATHS
			std::wstring cs;
			public:
#				if BOOST_FILESYSTEM_VERSION == 2
					explicit PW(const OS::path_t &path) throw() : cs(path.file_string()) { }
#				else
					explicit PW(const OS::path_t &path) throw() : cs(path.wstring()) { }
#				endif
				operator const wchar_t*() const throw() { return cs.c_str(); }
				operator const std::wstring() const { return cs; }
#		else
			UW cs;
			public:
#				if BOOST_FILESYSTEM_VERSION == 2
					explicit PW(const OS::path_t &path) throw() : cs(path.file_string()) { }
#				else
					explicit PW(const OS::path_t &path) throw() : cs(path.string()) { }
#				endif
				operator const wchar_t*() const throw() { return (const wchar_t*)cs; }
				operator const std::wstring() const { return cs; }
#		endif
	};

	/**
	 * Mark a string to be streamed out escaped for Lua.
	 *
	 * The surrounding quote marks are *not* included in the output.
	 *
	 * Example:
	 * @code
	 *   std::string str = ...
	 *   std::cout << '"' << Str::Lua(str) << '"' << std::endl;
	 * @endcode
	 */
	class MR_DllDeclare Lua
	{
		public:
			explicit Lua(const std::string &s) : s(s) { }

		public:
			std::ostream &StreamOut(std::ostream &os) const;

		private:
			std::string s;
	};

	inline std::ostream &operator<<(std::ostream &os, const Lua &lua)
	{
		return lua.StreamOut(os);
	}

}  // namespace Str

}  // namespace Util
}  // namespace HoverRace

#undef MR_DllDeclare
