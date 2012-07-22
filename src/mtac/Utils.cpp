//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Type.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::is_single_int_register(std::shared_ptr<const Type> type){
   return type == INT; 
}

bool mtac::is_single_float_register(std::shared_ptr<const Type> type){
    return type == FLOAT;
}

bool mtac::is_recursive(std::shared_ptr<mtac::Function> function){
    for(auto& basic_block : function->getBasicBlocks()){
        for(auto& statement : basic_block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                if((*ptr)->functionDefinition == function->definition){
                    return true;
                }
            }
        }
    }

    return false;
}

template<typename T>
void collect(mtac::VariableUsage& usage, T arg){
    if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
        ++usage[*variablePtr];
    }
}

template<typename T>
void collect_optional(mtac::VariableUsage& usage, T opt){
    if(opt){
        collect(usage, *opt);
    }
}

mtac::VariableUsage mtac::compute_variable_usage(std::shared_ptr<mtac::Function> function){
    mtac::VariableUsage usage;

    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                ++usage[(*ptr)->result];
                collect_optional(usage, (*ptr)->arg1);
                collect_optional(usage, (*ptr)->arg2);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
                collect(usage, (*ptr)->arg);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                collect(usage, (*ptr)->arg1);
                collect_optional(usage, (*ptr)->arg2);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                collect(usage, (*ptr)->arg1);
                collect_optional(usage, (*ptr)->arg2);
            }
        }
    }

    return usage;
}

void eddic::mtac::computeBlockUsage(std::shared_ptr<mtac::Function> function, std::unordered_set<std::shared_ptr<mtac::BasicBlock>>& usage){
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
                usage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                usage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                usage.insert((*ptr)->block);
            }
        }
    }
}

bool eddic::mtac::safe(const std::string& function){
    //These functions are considered as safe because they save/restore all the registers and does not return anything 
    return 
        function == "_F5printB" || function == "_F5printI" || function == "_F5printF" || function == "_F5printS" ||
        function == "_F7printlnB" || function == "_F7printlnI" || function == "_F7printlnF" || function == "_F7printlnS" || 
        function == "_F7println"; 
}

bool eddic::mtac::safe(std::shared_ptr<mtac::Call> call){
    auto function = call->function;

    return safe(function);
}

bool eddic::mtac::erase_result(mtac::Operator op){
   return 
            op != mtac::Operator::DOT_ASSIGN 
        &&  op != mtac::Operator::DOT_FASSIGN 
        &&  op != mtac::Operator::DOT_PASSIGN 
        &&  op != mtac::Operator::RETURN; 
}

bool eddic::mtac::is_distributive(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool eddic::mtac::is_expression(mtac::Operator op){
    return op >= mtac::Operator::ADD && op <= mtac::Operator::FDIV;
}

unsigned int eddic::mtac::compute_member_offset(std::shared_ptr<Variable> var, const std::vector<std::string>& memberNames){
    return compute_member(var, memberNames).first;
}

std::pair<unsigned int, std::shared_ptr<const Type>> eddic::mtac::compute_member(std::shared_ptr<Variable> var, const std::vector<std::string>& memberNames){
    auto type = var->type();

    std::string struct_name;
    if(type->is_pointer() || type->is_array()){
        struct_name = type->data_type()->type();
    } else {
        struct_name = type->type();
    }

    auto struct_type = symbols.get_struct(struct_name);
    std::shared_ptr<const Type> member_type;

    unsigned int offset = 0;

    auto& members = memberNames;
    for(std::size_t i = 0; i < members.size(); ++i){
        auto& member = members[i];

        member_type = (*struct_type)[member]->type;

        offset += symbols.member_offset(struct_type, member);

        if(i != members.size() - 1){
            struct_name = member_type->type();
            struct_type = symbols.get_struct(struct_name);
        }
    }

    return std::make_pair(offset, member_type);
}
