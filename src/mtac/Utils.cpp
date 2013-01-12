//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Type.hpp"
#include "VisitorUtils.hpp"
#include "GlobalContext.hpp"
#include "mangling.hpp"
#include "Variable.hpp"

#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::is_single_int_register(std::shared_ptr<const Type> type){
   return type == INT || type == BOOL || type == CHAR || type->is_pointer(); 
}

bool mtac::is_single_float_register(std::shared_ptr<const Type> type){
    return type == FLOAT;
}

bool mtac::is_recursive(mtac::Function& function){
    for(auto& basic_block : function){
        for(auto& quadruple : basic_block->statements){
            if(quadruple->op == mtac::Operator::CALL && quadruple->function().mangled_name() == function.definition().mangled_name()){
                return true;
            }
        }
    }

    return false;
}

namespace {

struct VariableUsageCollector {
    mtac::VariableUsage& usage;
    int depth_factor;
    int current_depth;

    VariableUsageCollector(mtac::VariableUsage& usage, int depth_factor) : usage(usage), depth_factor(depth_factor) {}

    void inc_usage(std::shared_ptr<Variable> variable){
        usage[variable] += pow(depth_factor, current_depth);
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*opt)){
                inc_usage(*variablePtr);
            }
        }
    }

    void collect(std::shared_ptr<mtac::Quadruple> quadruple){
        current_depth = quadruple->depth;

        inc_usage(quadruple->result);
        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    }
};

struct BasicBlockUsageCollector {
    std::unordered_set<mtac::basic_block_p>& usage;

    BasicBlockUsageCollector(std::unordered_set<mtac::basic_block_p>& usage) : usage(usage) {}

    void collect(std::shared_ptr<mtac::Quadruple> goto_){
        usage.insert(goto_->block);
    }
};

} //end of anonymous namespace

mtac::VariableUsage mtac::compute_variable_usage(mtac::Function& function){
    return compute_variable_usage_with_depth(function, 1);
}

mtac::VariableUsage mtac::compute_variable_usage_with_depth(mtac::Function& function, int factor){
    mtac::VariableUsage usage;

    VariableUsageCollector collector(usage, factor);

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            collector.collect(quadruple);
        }
    }

    return usage;
}

void eddic::mtac::computeBlockUsage(mtac::Function& function, std::unordered_set<mtac::basic_block_p>& usage){
    BasicBlockUsageCollector collector(usage);

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            collector.collect(quadruple);
        }
    }
}

bool eddic::mtac::safe(const std::string& function){
    //These functions are considered as safe because they save/restore all the registers and does not return anything 
    return 
        function == "_F5printF" || function == "_F5printS" || function == "_F5printC" ||
        function == "_F7printlnF" || function == "_F7printlnS" || function == "_F7printlnC" || 
        function == "_F7println"; 
}

bool eddic::mtac::erase_result(mtac::Operator op){
   return 
           op != mtac::Operator::DOT_ASSIGN 
        && op != mtac::Operator::DOT_FASSIGN 
        && op != mtac::Operator::DOT_PASSIGN 
        && op != mtac::Operator::RETURN
        && op != mtac::Operator::GOTO
        && op != mtac::Operator::NOP
        && op != mtac::Operator::PARAM
        && op != mtac::Operator::CALL
        && op != mtac::Operator::LABEL
        && !(op >= mtac::Operator::IF_UNARY && op <= mtac::Operator::IF_FALSE_FL);
}

bool eddic::mtac::is_distributive(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool eddic::mtac::is_expression(mtac::Operator op){
    return op >= mtac::Operator::ADD && op <= mtac::Operator::FDIV;
}

unsigned int eddic::mtac::compute_member_offset(std::shared_ptr<const GlobalContext> context, std::shared_ptr<const Type> type, const std::string& member){
    return compute_member(context, type, member).first;
}

std::pair<unsigned int, std::shared_ptr<const Type>> eddic::mtac::compute_member(std::shared_ptr<const GlobalContext> context, std::shared_ptr<const Type> type, const std::string& member){
    auto struct_type = context->get_struct(type);
    std::shared_ptr<const Type> member_type;
    unsigned int offset = 0;

    do {
        if(struct_type->member_exists(member)){
            member_type = (*struct_type)[member]->type;
            break;
        }

        offset += context->self_size_of_struct(struct_type);

        struct_type = context->get_struct(struct_type->parent_type);
    } while(struct_type);

    eddic_assert(member_type, "The member must exist");

    offset += context->member_offset(struct_type, member);

    return std::make_pair(offset, member_type);
}

//TODO Use the copy constructor instead

std::shared_ptr<mtac::Quadruple> mtac::copy(const std::shared_ptr<mtac::Quadruple>& quadruple){
    return std::make_shared<mtac::Quadruple>(*quadruple);
}
