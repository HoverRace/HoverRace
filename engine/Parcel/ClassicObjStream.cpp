
// ClassicObjStream.cpp
// Standard HoverRace 1.x parcel data stream.
//
// Copyright (c) 2010, 2012 Michael Imamura.
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

#include "../StdAfx.h"

#include "../Exception.h"

#include "ClassicObjStream.h"

namespace HoverRace {
namespace Parcel {

ClassicObjStream::ClassicObjStream(FILE *stream, const Util::OS::path_t &name, bool writing) :
	SUPER(name, 1, writing),
	stream(stream)
{
	// Version for classic record file is currently always 1.
}

void ClassicObjStream::WriteUInt8(MR_UInt8 i)
{
	WriteBuf(&i, 1);
}

void ClassicObjStream::WriteInt16(MR_Int16 i)
{
	//TODO: Big-endian conversion.
	WriteBuf(&i, 2);
}

void ClassicObjStream::WriteUInt16(MR_UInt16 i)
{
	//TODO: Big-endian conversion.
	WriteBuf(&i, 2);
}

void ClassicObjStream::WriteInt32(MR_Int32 i)
{
	//TODO: Big-endian conversion.
	WriteBuf(&i, 4);
}

void ClassicObjStream::WriteUInt32(MR_UInt32 i)
{
	//TODO: Big-endian conversion.
	WriteBuf(&i, 4);
}

void ClassicObjStream::WriteString(const std::string &s)
{
	MR_UInt32 len = s.length();
	WriteStringLength(len);
	WriteBuf(s.c_str(), len);
}

void ClassicObjStream::WriteStringLength(MR_UInt32 len)
{
	if (len < 0xff) {
		WriteUInt8(static_cast<MR_UInt8>(len));
		return;
	}

	WriteUInt8(0xff);
	// 0xfffe is the marker for Unicode strings.
	if (len < 0xfffe) {
		WriteUInt16(static_cast<MR_UInt16>(len));
		return;
	}

	WriteUInt16(0xffff);
	WriteUInt32(len);
}

void ClassicObjStream::ReadUInt8(MR_UInt8 &i)
{
	ReadBuf(&i, 1);
}

void ClassicObjStream::ReadInt16(MR_Int16 &i)
{
	ReadBuf(&i, 2);
	//TODO: Big-endian conversion.
}

void ClassicObjStream::ReadUInt16(MR_UInt16 &i)
{
	ReadBuf(&i, 2);
	//TODO: Big-endian conversion.
}

void ClassicObjStream::ReadInt32(MR_Int32 &i)
{
	ReadBuf(&i, 4);
	//TODO: Big-endian conversion.
}

void ClassicObjStream::ReadUInt32(MR_UInt32 &i)
{
	ReadBuf(&i, 4);
	//TODO: Big-endian conversion.
}

// String reading based on C# code by Robert Pittenger:
// http://www.codeproject.com/KB/cs/MFC_Serialization2.aspx

void ClassicObjStream::ReadString(std::string &s)
{
	MR_UInt32 len = ReadStringLength();

	char *buf = new char[len];
	ReadBuf(buf, len);
	s.assign(buf, len);
	delete[] buf;
}

MR_UInt32 ClassicObjStream::ReadStringLength()
{
	MR_UInt8 b;
	ReadUInt8(b);
	if (b < 0xff) return b;

	MR_UInt16 w;
	ReadUInt16(w);
	if (w == 0xfffe) {
		// Unicode (length follows).
		ASSERT(FALSE);
		throw UnimplementedExn("ClassicObjStream::ReadStringLength for unicode strings");
	}
	else if (w == 0xffff) {
		MR_UInt32 dw;
		ReadUInt32(dw);
		return dw;
	}
	else {
		return w;
	}
}

}  // namespace Parcel
}  // namespace HoverRace
