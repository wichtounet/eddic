//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SWAP_H
#define SWAP_H

#include <string>

#include "ParseNode.hpp"
#include "Types.hpp"

namespace eddic {

class Variable;

class Swap : public ParseNode {
    private:
        std::string m_lhs;
        std::string m_rhs;
        std::shared_ptr<Variable> m_lhs_var;
        std::shared_ptr<Variable> m_rhs_var;
        Type m_type;

    public:
        Swap(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& lhs, const std::string& rhs);

        void checkVariables();
        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
