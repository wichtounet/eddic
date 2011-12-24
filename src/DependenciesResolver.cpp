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

#include "parser/SpiritParser.hpp"

#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "ASTVisitor.hpp"

using namespace eddic;

bool exists(const std::string& file){
   std::ifstream ifile(file.c_str());
   return ifile; 
}

DependenciesResolver::DependenciesResolver(SpiritParser& p) : parser(p) {}

void includeDependencies(ast::SourceFile& program, SpiritParser& parser);

class DependencyVisitor : public boost::static_visitor<> {
    private:
        SpiritParser& parser;
        ast::SourceFile& source;

    public:
        DependencyVisitor(SpiritParser& p, ast::SourceFile& s) : parser(p), source(s) {}

        AUTO_RECURSE_PROGRAM()
    
        void operator()(ast::StandardImport& import){
            auto header = import.header;
            auto headerFile = "stdlib/" + header + ".eddi";
            
            if(!exists(headerFile)){
                throw SemanticalException("The header " + header + " does not exist");
            }
           
            ast::SourceFile dependency; 
            if(parser.parse(headerFile, dependency)){
                includeDependencies(dependency, parser); 

                for(ast::FirstLevelBlock& block : dependency.Content->blocks){
                    source.Content->blocks.push_back(block);
                }
            }
        }
    
        void operator()(ast::Import& import){
            auto file = import.file.substr(1, import.file.size() - 2);

            if(!exists(file)){
                throw SemanticalException("The file " + file + " does not exist");
            }
           
            ast::SourceFile dependency; 
            if(parser.parse(file, dependency)){
                includeDependencies(dependency, parser); 

                for(ast::FirstLevelBlock& block : dependency.Content->blocks){
                    source.Content->blocks.push_back(block);
                }
            }
        }

        template<typename T>
        void operator()(T&){
            //Nothing to include there
        }
};

void includeDependencies(ast::SourceFile& program, SpiritParser& parser){
    DependencyVisitor visitor(parser, program);
    visitor(program);
}

void DependenciesResolver::resolve(ast::SourceFile& program) const {
    includeDependencies(program, parser);
}
