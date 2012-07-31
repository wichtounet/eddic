//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::is_single_int_register(std::shared_ptr<const Type> type){
   return type == INT || type == BOOL || type->is_pointer(); 
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

namespace {

struct VariableUsageCollector : public boost::static_visitor<> {
    mtac::VariableUsage& usage;

    VariableUsageCollector(mtac::VariableUsage& usage) : usage(usage) {}

    template<typename T>
    void collect(T& arg){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
            ++usage[*variablePtr];
        }
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            collect(*opt);
        }
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        ++usage[quadruple->result];
        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::Param> param){
        collect(param->arg);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        collect(if_->arg1);
        collect_optional(if_->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        collect(if_false->arg1);
        collect_optional(if_false->arg2);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

struct BasicBlockUsageCollector : public boost::static_visitor<> {
    std::unordered_set<std::shared_ptr<mtac::BasicBlock>>& usage;

    BasicBlockUsageCollector(std::unordered_set<std::shared_ptr<mtac::BasicBlock>>& usage) : usage(usage) {}

    void operator()(std::shared_ptr<mtac::Goto> goto_){
        usage.insert(goto_->block);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        usage.insert(if_->block);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        usage.insert(if_false->block);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

} //end of anonymous namespace

mtac::VariableUsage mtac::compute_variable_usage(std::shared_ptr<mtac::Function> function){
    mtac::VariableUsage usage;

    VariableUsageCollector collector(usage);

    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
           visit(collector, statement); 
        }
    }

    return usage;
}

void eddic::mtac::computeBlockUsage(std::shared_ptr<mtac::Function> function, std::unordered_set<std::shared_ptr<mtac::BasicBlock>>& usage){
    BasicBlockUsageCollector collector(usage);

    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
           visit(collector, statement); 
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
