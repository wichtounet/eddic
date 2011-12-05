//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "DependenciesResolver.hpp"

#include "ast/SourceFile.hpp"

#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

DependenciesResolver::DependenciesResolver(SpiritParser& p) : parser(p) {}

class DependencyVisitor : public boost::static_visitor<> {
    private:
        SpiritParser& parser;

    public:
        DependencyVisitor(SpiritParser& p) : parser(p) {}

        AUTO_RECURSE_PROGRAM()
    
        void operator()(ast::StandardImport& import){
            //TODO
        }
    
        void operator()(ast::Import& import){
            //TODO
        }

        template<typename T>
        void operator()(T&){
            //Nothing to include there
        }
};

void DependenciesResolver::resolve(ast::SourceFile& program) const {
    DependencyVisitor visitor(parser);
    visitor(program);
}
