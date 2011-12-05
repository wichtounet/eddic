//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>

#include "DependenciesResolver.hpp"

#include "ast/SourceFile.hpp"

#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

bool exists(const std::string& file){
   std::ifstream ifile(file.c_str());
   return ifile; 
}

DependenciesResolver::DependenciesResolver(SpiritParser& p) : parser(p) {}

class DependencyVisitor : public boost::static_visitor<> {
    private:
        SpiritParser& parser;

    public:
        DependencyVisitor(SpiritParser& p) : parser(p) {}

        AUTO_RECURSE_PROGRAM()
    
        void operator()(ast::StandardImport& import){
            auto header = import.header;
            auto headerFile = "stdlib/" + header + ".eddi";
            
            if(!exists(headerFile)){
                throw SemanticalException("The header " + header + " does not exist");
            }
             
            //TODO
        }
    
        void operator()(ast::Import& import){
            auto file = import.file.substr(1, import.file.size() - 2);

            if(!exists(file)){
                throw SemanticalException("The file " + file + " does not exist");
            }

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
