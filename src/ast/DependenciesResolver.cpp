//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include<string>
#include<unordered_set>

#include "ast/DependenciesResolver.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"

#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"

#include "parser_x3/SpiritParser.hpp"

using namespace eddic;

class DependencyVisitor : public boost::static_visitor<> {
    private:
        parser_x3::SpiritParser& parser;
        ast::SourceFile& source_program;

        std::unordered_set<std::string> imported;

    public:
        DependencyVisitor(parser_x3::SpiritParser& p, ast::SourceFile& source_program) : parser(p), source_program(source_program) {}

        std::vector<ast::SourceFileBlock> blocks;

        AUTO_RECURSE_PROGRAM()

        void operator()(ast::StandardImport& import){
            if(imported.find(import.header) != imported.end()){
                return;
            }

            imported.insert(import.header);

            auto headerFile = "stdlib/" + import.header + ".eddi";

            if(!file_exists(headerFile)){
                throw SemanticalException("The header " + import.header + " does not exist");
            }

            ast::SourceFile dependency;
            if(parser.parse(headerFile, dependency, source_program.context)){
                (*this)(dependency);

                for(ast::SourceFileBlock& block : dependency.blocks){
                    if(auto* ptr = boost::get<ast::TemplateFunctionDeclaration>(&block)){
                        if(!ptr->is_template()){
                            ptr->standard = true;
                            ptr->header = import.header;
                        }
                    } else if(auto* ptr = boost::get<ast::struct_definition>(&block)){
                        ptr->standard = true;
                        ptr->header = import.header;
                    }

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
            if(parser.parse(file, dependency, source_program.context)){
                (*this)(dependency);

                for(ast::SourceFileBlock& block : dependency.blocks){
                    if(auto* ptr = boost::get<ast::TemplateFunctionDeclaration>(&block)){
                        if(!ptr->is_template()){
                            ptr->header = import.file;
                        }
                    } else if(auto* ptr = boost::get<ast::struct_definition>(&block)){
                        ptr->header = import.file;
                    }

                    blocks.push_back(block);
                }
            } else {
                throw SemanticalException("The file " + file + " cannot be imported");
            }
        }

        AUTO_FORWARD()
        AUTO_IGNORE_OTHERS()
};

void ast::resolveDependencies(ast::SourceFile& program, parser_x3::SpiritParser& parser){
    DependencyVisitor visitor(parser, program);
    visitor(program);

    program.blocks.reserve(program.blocks.size() + visitor.blocks.size());

    for(auto& block : visitor.blocks){
       program.blocks.emplace_back(std::move(block));
    }
}
