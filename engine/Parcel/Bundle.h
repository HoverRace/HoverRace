
// Bundle.h
// A source of parcels.
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

#include "../Util/OS.h"

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

class Bundle;
typedef boost::shared_ptr<Bundle> BundlePtr;
class RecordFile;
typedef boost::shared_ptr<RecordFile> RecordFilePtr;

/**
 * A source of parcels.
 * @author Michael Imamura
 */
class MR_DllDeclare Bundle
{
	public:
		Bundle(const Util::OS::path_t &dir, BundlePtr subBundle=BundlePtr());
		virtual ~Bundle() { }

		virtual RecordFilePtr OpenParcel(const std::string &name, bool writing=false) const;

	private:
		class MR_DllDeclare Iterator :
			public std::iterator<std::input_iterator_tag, Util::OS::dirEnt_t>
		{
			public:
				Iterator();
				Iterator(const Bundle *bundle);

			public:
				bool operator==(const Iterator &other) const
				{
					return (bundle == other.bundle) && (iter == other.iter);
				}
				bool operator!=(const Iterator &other) const
				{
					return (bundle != other.bundle) || (iter != other.iter);
				}

				Util::OS::dirEnt_t &operator*() const { return *iter; }
				Util::OS::dirEnt_t *operator->() const { return &*iter; }

				Iterator &operator++();
				Iterator operator++(int);

			private:
				void FindNextValidBundle();

			private:
				const Bundle *bundle;
				Util::OS::dirIter_t iter;
				static const Util::OS::dirIter_t END;
		};
	public:
		typedef Iterator iterator;
		typedef Iterator const_iterator;
		typedef Util::OS::dirEnt_t value_type;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

	private:
		Util::OS::path_t dir;
		BundlePtr subBundle;
		static const iterator END;
};

}  // namespace Parcel
}  // namespace HoverRace

#undef MR_DllDeclare
