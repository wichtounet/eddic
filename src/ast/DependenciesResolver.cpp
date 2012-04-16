//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>

#include "ast/DependenciesResolver.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"

#include "parser/SpiritParser.hpp"

using namespace eddic;

class DependencyVisitor : public boost::static_visitor<> {
    private:
        parser::SpiritParser& parser;
        ast::SourceFile& source;
        std::vector<ast::FirstLevelBlock> blocks;

    public:
        DependencyVisitor(parser::SpiritParser& p, ast::SourceFile& s) : parser(p), source(s) {}

        void operator()(ast::SourceFile& program){
            visit_each(*this, program.Content->blocks);

            for(ast::FirstLevelBlock& block : blocks){
                source.Content->blocks.push_back(block);
            }
        }
    
        void operator()(ast::StandardImport& import){
            auto headerFile = "stdlib/" + import.header + ".eddi";
            
            if(!file_exists(headerFile)){
                throw SemanticalException("The header " + import.header + " does not exist");
            }
           
            ast::SourceFile dependency; 
            if(parser.parse(headerFile, dependency)){
                resolveDependencies(dependency, parser); 

                for(ast::FirstLevelBlock& block : dependency.Content->blocks){
                    blocks.push_back(block);
                }
            } else {
                throw SemanticalException("The header " + import.header + " cannot be imported");
            }
        }
    
        void operator()(ast::Import& import){
            auto file = import.file;
            file.erase(0, 1);
            file.resize(file.size() - 1);

            if(!file_exists(file)){
                throw SemanticalException("The file " + file + " does not exist");
            }
           
            ast::SourceFile dependency; 
            if(parser.parse(file, dependency)){
                resolveDependencies(dependency, parser); 

                for(ast::FirstLevelBlock& block : dependency.Content->blocks){
                    blocks.push_back(block);
                }
            } else {
                throw SemanticalException("The file " + file + " cannot be imported");
            }
        }

        template<typename T>
        void operator()(T&){
            //Nothing to include there
        }
};

void ast::resolveDependencies(ast::SourceFile& program, parser::SpiritParser& parser){
    DependencyVisitor visitor(parser, program);
    visitor(program);
}
