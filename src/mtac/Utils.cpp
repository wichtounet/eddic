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
#include "mtac/Statement.hpp"

using namespace eddic;

bool mtac::is_single_int_register(std::shared_ptr<const Type> type){
   return type == INT || type == BOOL || type == CHAR || type->is_pointer(); 
}

bool mtac::is_single_float_register(std::shared_ptr<const Type> type){
    return type == FLOAT;
}

bool mtac::is_recursive(mtac::Function& function){
    for(auto& basic_block : function){
        for(auto& statement : basic_block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if((*ptr)->op == mtac::Operator::CALL && (*ptr)->function().mangled_name() == function.definition().mangled_name()){
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
    int depth_factor;
    int current_depth;

    VariableUsageCollector(mtac::VariableUsage& usage, int depth_factor) : usage(usage), depth_factor(depth_factor) {}

    void inc_usage(std::shared_ptr<Variable> variable){
        usage[variable] += pow(depth_factor, current_depth);
    }

    template<typename T>
    void collect(T& arg){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
            inc_usage(*variablePtr);
        }
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            collect(*opt);
        }
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        current_depth = quadruple->depth;

        inc_usage(quadruple->result);
        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        current_depth = if_->depth;

        collect(if_->arg1);
        collect_optional(if_->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        current_depth = if_false->depth;

        collect(if_false->arg1);
        collect_optional(if_false->arg2);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

struct BasicBlockUsageCollector : public boost::static_visitor<> {
    std::unordered_set<mtac::basic_block_p>& usage;

    BasicBlockUsageCollector(std::unordered_set<mtac::basic_block_p>& usage) : usage(usage) {}

    void operator()(std::shared_ptr<mtac::Quadruple> goto_){
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

mtac::VariableUsage mtac::compute_variable_usage(mtac::Function& function){
    return compute_variable_usage_with_depth(function, 1);
}

mtac::VariableUsage mtac::compute_variable_usage_with_depth(mtac::Function& function, int factor){
    mtac::VariableUsage usage;

    VariableUsageCollector collector(usage, factor);

    visit_all_statements(collector, function);

    return usage;
}

void eddic::mtac::computeBlockUsage(mtac::Function& function, std::unordered_set<mtac::basic_block_p>& usage){
    BasicBlockUsageCollector collector(usage);

    visit_all_statements(collector, function);
}

bool eddic::mtac::safe(const std::string& function){
    //These functions are considered as safe because they save/restore all the registers and does not return anything 
    return 
        function == "_F5printI" || function == "_F5printF" || function == "_F5printS" || function == "_F5printC" ||
        function == "_F7printlnI" || function == "_F7printlnF" || function == "_F7printlnS" || function == "_F7printlnC" || 
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
        && op != mtac::Operator::LABEL; 
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

namespace {

//TODO Use the copy constructor instead

struct StatementClone : public boost::static_visitor<mtac::Statement> {
    std::shared_ptr<GlobalContext> global_context;

    StatementClone(std::shared_ptr<GlobalContext> global_context) : global_context(global_context) {}

    mtac::Statement operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        auto copy = std::make_shared<mtac::Quadruple>();

        copy->result = quadruple->result;
        copy->arg1 = quadruple->arg1;
        copy->arg2 = quadruple->arg2;
        copy->op = quadruple->op;
        copy->size = quadruple->size;
        copy->block = quadruple->block;
        copy->address = quadruple->address;
        copy->m_function = quadruple->m_function;
        copy->m_param = quadruple->m_param;
        copy->secondary = quadruple->secondary;

        if(copy->op == mtac::Operator::CALL){
            ++copy->function().references();
        }
        
        return copy;
    }

    mtac::Statement operator()(std::shared_ptr<mtac::IfFalse> if_){
        auto copy = std::make_shared<mtac::IfFalse>();

        copy->op = if_->op;
        copy->arg1 = if_->arg1;
        copy->arg2 = if_->arg2;
        copy->label = if_->label;
        copy->block = if_->block;

        return copy;
    }

    mtac::Statement operator()(std::shared_ptr<mtac::If> if_){
        auto copy = std::make_shared<mtac::If>();

        copy->op = if_->op;
        copy->arg1 = if_->arg1;
        copy->arg2 = if_->arg2;
        copy->label = if_->label;
        copy->block = if_->block;

        return copy;
    }
};

}

mtac::Statement mtac::copy(const mtac::Statement& statement, std::shared_ptr<GlobalContext> context){
    StatementClone cloner(context);
    return visit(cloner, statement);
}
