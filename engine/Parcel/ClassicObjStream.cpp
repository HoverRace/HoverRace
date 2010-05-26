
// ClassicObjStream.cpp
// Standard HoverRace 1.x parcel data stream.
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

#include "StdAfx.h"

#include "ClassicObjStream.h"

namespace HoverRace {
namespace Parcel {

ClassicObjStream::ClassicObjStream(std::iostream &stream, const std::string &name, bool writing) :
	SUPER(name, 1, writing),
	stream(&stream)
{
	// Version for classic record file is currently always 1.
}

void ClassicObjStream::WriteUInt8(MR_UInt8 i)
{
	stream->write(reinterpret_cast<char*>(&i), 1);
}

void ClassicObjStream::WriteInt16(MR_Int16 i)
{
	//TODO: Big-endian conversion.
	stream->write(reinterpret_cast<char*>(&i), 2);
}

void ClassicObjStream::WriteUInt16(MR_UInt16 i)
{
	//TODO: Big-endian conversion.
	stream->write(reinterpret_cast<char*>(&i), 2);
}

void ClassicObjStream::WriteInt32(MR_Int32 i)
{
	//TODO: Big-endian conversion.
	stream->write(reinterpret_cast<char*>(&i), 4);
}

void ClassicObjStream::WriteUInt32(MR_UInt32 i)
{
	//TODO: Big-endian conversion.
	stream->write(reinterpret_cast<char*>(&i), 4);
}

void ClassicObjStream::WriteString(const std::string &s)
{
	//TODO
}

void ClassicObjStream::ReadUInt8(MR_UInt8 &i)
{
	stream->read(reinterpret_cast<char*>(&i), 1);
}

void ClassicObjStream::ReadInt16(MR_Int16 &i)
{
	stream->read(reinterpret_cast<char*>(&i), 2);
	//TODO: Big-endian conversion.
}

void ClassicObjStream::ReadUInt16(MR_UInt16 &i)
{
	stream->read(reinterpret_cast<char*>(&i), 2);
	//TODO: Big-endian conversion.
}

void ClassicObjStream::ReadInt32(MR_Int32 &i)
{
	stream->read(reinterpret_cast<char*>(&i), 4);
	//TODO: Big-endian conversion.
}

void ClassicObjStream::ReadUInt32(MR_UInt32 &i)
{
	stream->read(reinterpret_cast<char*>(&i), 4);
	//TODO: Big-endian conversion.
}

// String reading based on C# code by Robert Pittenger:
// http://www.codeproject.com/KB/cs/MFC_Serialization2.aspx

void ClassicObjStream::ReadString(std::string &s)
{
	MR_UInt32 len = ReadStringLength();

	char *buf = new char[len];
	stream->read(buf, len);
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
		throw std::exception();
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
