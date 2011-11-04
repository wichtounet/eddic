//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/variant/static_visitor.hpp>

#include "StringChecker.hpp"

#include "ast/Program.hpp"
#include "StringPool.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

class StringCheckerVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<StringPool> pool;

    public:
        StringCheckerVisitor(std::shared_ptr<StringPool> p) : pool(p) {}

        AUTO_RECURSE_PROGRAM()
        AUTO_RECURSE_FUNCTION_DECLARATION() 
        AUTO_RECURSE_GLOBAL_DECLARATION() 
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_FOREACH()
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_BINARY_CONDITION()
        AUTO_RECURSE_COMPOSED_VALUES()
        AUTO_RECURSE_VARIABLE_OPERATIONS()

        void operator()(ASTLitteral& litteral){
            litteral.label = pool->label(litteral.value);
        }
        
        void operator()(ASTSwap&){
            //No need for string checking in a swap
        }

        void operator()(ASTVariable&){
            //No need for string checking in a variable reference
        }
        
        void operator()(ASTFalse&){
            //No need for string checking in a boolean
        }
        
        void operator()(ASTTrue&){
            //No need for string checking in a boolean
        }
        
        void operator()(ASTInteger&){
            //No need for string checking in a integer
        }
};

void StringChecker::check(ASTProgram& program, std::shared_ptr<StringPool> pool){
   StringCheckerVisitor visitor(pool);
   visitor(program); 
}
