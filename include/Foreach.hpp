//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FOREACH_H
#define FOREACH_H

#include <memory>
#include <string>

#include "ParseNode.hpp"
#include "Types.hpp"

namespace eddic {

class Condition;
class Variable;
class Value;

class Foreach : public ParseNode {
    private: 
        Type m_type;
        std::string m_variable;
        std::shared_ptr<Variable> m_var;
        std::shared_ptr<Value> m_from;
        std::shared_ptr<Value> m_to;

    public:
        Foreach(std::shared_ptr<Context> context, const Tok token, Type type, std::string variable, std::shared_ptr<Value> from, std::shared_ptr<Value> to);

        virtual void writeIL(IntermediateProgram& program);
        
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();
};

} //end of eddic

#endif
