//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>

#include "variant.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Warnings.hpp"
#include "Options.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"

#include "ast/WarningsEngine.hpp"
#include "ast/SourceFile.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/Position.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::shared_ptr<Variable>, ast::Position> Positions;  

struct Collector : public boost::static_visitor<> {
    public:
        AUTO_RECURSE_PROGRAM()
        AUTO_RECURSE_STRUCT()
        AUTO_RECURSE_DESTRUCTOR()

        void operator()(ast::FunctionDeclaration& function){
            for(auto& param : function.Content->parameters){
                positions[function.Content->context->getVariable(param.parameterName)] = function.Content->position;
            }

            visit_each(*this, function.Content->instructions);
        }
        
        void operator()(ast::Constructor& function){
            for(auto& param : function.Content->parameters){
                positions[function.Content->context->getVariable(param.parameterName)] = function.Content->position;
            }
            
            visit_each(*this, function.Content->instructions);
        }
        
        void operator()(ast::GlobalVariableDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->variableName)] = declaration.Content->position;
        }

        void operator()(ast::GlobalArrayDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->arrayName)] = declaration.Content->position;
        }
        
        void operator()(ast::ArrayDeclaration& declaration){
            //If this is null, it means that this an array inside a structure
            if(declaration.Content->context){
                positions[declaration.Content->context->getVariable(declaration.Content->arrayName)] = declaration.Content->position;
            }
        }
        
        void operator()(ast::VariableDeclaration& declaration){
            positions[declaration.Content->context->getVariable(declaration.Content->variableName)] = declaration.Content->position;
        }

        AUTO_IGNORE_OTHERS()

        const ast::Position& getPosition(std::shared_ptr<Variable> var){
            assert(positions.find(var) != positions.end());

            return positions[var];   
        }

    private:
        Positions positions;  
};

struct Inspector : public boost::static_visitor<> {
    private:
        Collector& collector;
        
        std::shared_ptr<GlobalContext> context;
        std::shared_ptr<Configuration> configuration;

    public:
        Inspector(Collector& collector, std::shared_ptr<GlobalContext> context, std::shared_ptr<Configuration> configuration) : 
                collector(collector), context(context), configuration(configuration) {}
    
        /* The following constructions can contains instructions with warnings  */
        AUTO_RECURSE_GLOBAL_DECLARATION() 
        AUTO_RECURSE_FUNCTION_CALLS()
        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_RETURN_VALUES()
        AUTO_RECURSE_VARIABLE_OPERATIONS()
        AUTO_RECURSE_TERNARY()
        AUTO_RECURSE_SWITCH()
        AUTO_RECURSE_COMPOSED_VALUES()
        
        void check(std::shared_ptr<Context> context){
            if(configuration->option_defined("warning-unused")){
                auto iter = context->begin();
                auto end = context->end();

                for(; iter != end; iter++){
                    auto var = iter->second;

                    if(var->references() == 0){
                        if(var->position().isStack()){
                            warn(collector.getPosition(var), "unused variable '" + var->name() + "'");
                        } else if(var->position().isGlobal()){
                            warn(collector.getPosition(var), "unused global variable '" + var->name() + "'");
                        } else if(var->position().isParameter()){
                            warn(collector.getPosition(var), "unused parameter '" + var->name() + "'");
                        }
                    }
                }
            }
        }

        void operator()(ast::SourceFile& program){
            check(program.Content->context);

            visit_each(*this, program.Content->blocks);
        }
        
        void operator()(ast::Struct& declaration){
            if(configuration->option_defined("warning-unused")){
                auto struct_ = context->get_struct(declaration.Content->name);

                if(struct_->get_references() == 0){
                    warn(declaration.Content->position, "unused structure '" + declaration.Content->name + "'");
                } else {
                    for(auto& member : struct_->members){
                        if(member->get_references() == 0){
                            warn(declaration.Content->position, "unused member '" + declaration.Content->name + ".'" + member->name);
                        }
                    }
                }
            }
        }

        void operator()(ast::FunctionDeclaration& declaration){
            check(declaration.Content->context);
            
            if(configuration->option_defined("warning-unused")){
                int references = context->referenceCount(declaration.Content->mangledName);

                if(references == 0){
                    warn(declaration.Content->position, "unused function '" + declaration.Content->functionName + "'");
                }
            }
        
            check_each(declaration.Content->instructions);
        }
    
        void operator()(ast::Cast& cast){
            if(configuration->option_defined("warning-cast")){
                auto src_type = visit(ast::GetTypeVisitor(), cast.Content->value);
                auto dest_type = visit(ast::TypeTransformer(context), cast.Content->type);

                if(src_type == dest_type){
                    warn(cast.Content->position, "useless cast");
                }
            }
        }
        
        void operator()(ast::If& if_){
            visit(*this, if_.Content->condition);
            check_each(if_.Content->instructions);
            visit_each_non_variant(*this, if_.Content->elseIfs);
            visit_optional_non_variant(*this, if_.Content->else_);
        }
        
        void operator()(ast::ElseIf& elseIf){
            visit(*this, elseIf.condition);
            check_each(elseIf.instructions);
        }

        void operator()(ast::For& for_){
            visit_optional(*this, for_.Content->start);
            visit_optional(*this, for_.Content->condition);
            visit_optional(*this, for_.Content->repeat);
            check_each(for_.Content->instructions);
        }
        
        void operator()(ast::While& while_){
            visit(*this, while_.Content->condition);
            check_each(while_.Content->instructions);
        }

        void operator()(ast::DoWhile& while_){
            visit(*this, while_.Content->condition);
            check_each(while_.Content->instructions);
        }
        
        void operator()(ast::Else& else_){
            check_each(else_.instructions);
        }

        void operator()(ast::Foreach& foreach_){
            check_each(foreach_.Content->instructions);
        }
        
        void operator()(ast::ForeachIn& foreach_){
            check_each(foreach_.Content->instructions);
        }

        void operator()(ast::SwitchCase& switch_case){
            visit(*this, switch_case.value);
            check_each(switch_case.instructions);
        }

        void operator()(ast::DefaultCase& default_case){
            check_each(default_case.instructions);
        }
        
        void check_each(std::vector<ast::Instruction>& instructions){
            for(auto& instruction : instructions){
                if(auto* ptr = boost::get<ast::Expression>(&instruction)){
                    auto& value = *ptr;
                    bool effects = false;

                    for(auto& op : value.Content->operations){
                        if(op.get<0>() == ast::Operator::INC || op.get<0>() == ast::Operator::DEC || op.get<0>() == ast::Operator::CALL){
                            effects = true;
                            break;
                        }
                    }

                    if(!effects){
                        warn(value.Content->position, "Statement without any effect");
                    }
                }

                visit(*this, instruction);
            }
        }
        
        //No warnings for other types
        AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::WarningsPass::apply_program(ast::SourceFile& program, bool){
    Collector collector;
    visit_non_variant(collector, program);

    Inspector inspector(collector, program.Content->context, configuration);
    visit_non_variant(inspector, program);
}

bool ast::WarningsPass::is_simple(){
    return true;
}
