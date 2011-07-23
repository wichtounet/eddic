//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <string>
#include <map>

#include "ParseNode.hpp"

namespace eddic {

class StringPool : public ParseNode {
    private:
        std::map<std::string, std::string> pool;
        unsigned int currentString;
    public:
        StringPool(Context* context) : ParseNode(context), currentString(0) {
            label("\"\n\"");    
        };

        std::string label(const std::string& value);
        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
