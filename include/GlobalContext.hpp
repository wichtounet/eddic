//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"

#include "Context.hpp"

namespace eddic {

class GlobalContext : public Context {
    public:
        GlobalContext() : Context(NULL) {}
        
        void writeIL(IntermediateProgram& writer);
        
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type, ast::Value& value);
};

} //end of eddic

#endif
