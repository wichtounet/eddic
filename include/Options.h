//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>

enum Option {
	OPTIMIZE_ALL,
	OPTIMIZE_INTEGERS,
	OPTIMIZE_STRINGS
};

class Options {
	private:
		static std::map<Option, bool> options;

	public:
		static void set(Option option){ options[option] = true; }
		static void unset(Option option){ options[option] = false; }
		static bool isSet(Option option){ return options[option]; }
};

#endif
