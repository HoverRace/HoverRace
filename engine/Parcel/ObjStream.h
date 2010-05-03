
// ObjStream.h
// Base class for parcel serializers.
//
// Copyright (c) 2010 Michael Imamura.
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

#include <string>

#include "../Util/MR_Types.h"

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
namespace Parcel {

class MR_DllDeclare ObjStreamExn : public std::exception
{
	typedef std::exception SUPER;

	public:
		ObjStreamExn() : SUPER() { }
		ObjStreamExn(const std::string &name, const std::string &details) :
			SUPER(), msg(name)
		{
			msg += ": ";
			msg += details;
		}
		virtual ~ObjStreamExn() throw() { }

		virtual const char* what() const throw() { return msg.c_str(); }

	private:
		std::string msg;
};

/**
 * Base class for parcel serializers.
 * @author Michael Imamura
 */
class MR_DllDeclare ObjStream
{
	private:
		ObjStream() { }
	public:
		ObjStream(const std::string &name, int version, bool writing) :
			name(name), version(version), writing(writing)
			{ }
		virtual ~ObjStream() { }

	public:
		const std::string &GetName() const { return name; }
		int GetVersion() const { return version; }
		bool IsWriting() const { return writing; }

	public:
		virtual void Write(const void *buf, size_t ct) = 0;

		virtual void WriteUInt8(MR_UInt8 i) = 0;
		friend ObjStream &operator<<(ObjStream &os, MR_UInt8 i) { os.WriteUInt8(i); return os; }

		virtual void WriteInt16(MR_Int16 i) = 0;
		friend ObjStream &operator<<(ObjStream &os, MR_Int16 i) { os.WriteInt16(i); return os; }

		virtual void WriteUInt16(MR_UInt16 i) = 0;
		friend ObjStream &operator<<(ObjStream &os, MR_UInt16 i) { os.WriteUInt16(i); return os; }

		virtual void WriteInt32(MR_Int32 i) = 0;
		friend ObjStream &operator<<(ObjStream &os, MR_Int32 i) { os.WriteInt32(i); return os; }

		virtual void WriteUInt32(MR_UInt32 i) = 0;
		friend ObjStream &operator<<(ObjStream &os, MR_UInt32 i) { os.WriteUInt32(i); return os; }

#		ifdef _WIN32
			virtual void WriteCString(const CString &s) = 0;
			friend ObjStream &operator<<(ObjStream &os, const CString &s) { os.WriteCString(s); return os; }
#		endif

		virtual void Read(void *buf, size_t ct) = 0;

		virtual void ReadUInt8(MR_UInt8 &i) = 0;
		friend ObjStream &operator>>(ObjStream &os, MR_UInt8 &i) { os.ReadUInt8(i); return os; }

		virtual void ReadInt16(MR_Int16 &i) = 0;
		friend ObjStream &operator>>(ObjStream &os, MR_Int16 &i) { os.ReadInt16(i); return os; }

		virtual void ReadUInt16(MR_UInt16 &i) = 0;
		friend ObjStream &operator>>(ObjStream &os, MR_UInt16 &i) { os.ReadUInt16(i); return os; }

		virtual void ReadInt32(MR_Int32 &i) = 0;
		friend ObjStream &operator>>(ObjStream &os, MR_Int32 &i) { os.ReadInt32(i); return os; }

		virtual void ReadUInt32(MR_UInt32 &i) = 0;
		friend ObjStream &operator>>(ObjStream &os, MR_UInt32 &i) { os.ReadUInt32(i); return os; }

#		ifdef _WIN32
			virtual void ReadCString(CString &s) = 0;
			friend ObjStream &operator>>(ObjStream &os, CString &s) { os.ReadCString(s); return os; }
#		endif

	private:
		std::string name;
		int version;
		bool writing;
};
typedef boost::shared_ptr<ObjStream> ObjStreamPtr;

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
