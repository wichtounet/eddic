//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_FUNCTION_H
#define LTAC_FUNCTION_H

#include "tac/Function.hpp"
#include "ltac/BasicBlock.hpp"
#include "ltac/Statement.hpp"

namespace eddic {

namespace ltac {

class Function {
    public:
        Function(std::shared_ptr<FunctionContext> context, const std::string& name);

        std::shared_ptr<eddic::Function> definition;
        
        std::shared_ptr<FunctionContext> context;

        void add(Statement statement);

        std::string getName() const;

        std::vector<Statement>& getStatements();

    private:
        //Before being partitioned, the function has only statement
        std::vector<Statement> statements;

        std::string name;
};

} //end of mtac

} //end of eddic

#endif
