//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>
#include <string>

namespace eddic {

enum BooleanOption {
	OPTIMIZE_INTEGERS,
	OPTIMIZE_STRINGS
};

enum ValueOption {
	OUTPUT
};

class Options {
	private:
		static std::map<BooleanOption, bool> booleanOptions;
		static std::map<ValueOption, std::string> valueOptions;

	public:
		static void setDefaults(){
			set(OUTPUT, "a.out");
		}

		static void set(BooleanOption option){ booleanOptions[option] = true; }
		static void unset(BooleanOption option){ booleanOptions[option] = false; }
		static bool isSet(BooleanOption option){ return booleanOptions[option]; }
	
		static void set(ValueOption option, std::string value) { valueOptions[option] = value; }
		static std::string get(ValueOption option){ return valueOptions[option]; }
};

} //end of eddic

#endif
