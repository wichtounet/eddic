//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "logging.hpp"
#include "Function.hpp"

#include "mtac/Function.hpp"
#include "mtac/loop.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/basic_block.hpp"

using namespace eddic;

mtac::Function::Function(std::shared_ptr<FunctionContext> c, std::string n, eddic::Function& definition) : context(c), _definition(&definition), name(std::move(n)) {
    //Nothing to do   
}
        
mtac::Function::Function(mtac::Function&& rhs) : 
            context(std::move(rhs.context)), _definition(rhs._definition), 
            statements(std::move(rhs.statements)), 
            _pure(std::move(rhs._pure)), _standard(std::move(rhs._standard)),
            count(std::move(rhs.count)), index(std::move(rhs.index)),
            entry(std::move(rhs.entry)), exit(std::move(rhs.exit)), 
            _use_registers(std::move(rhs._use_registers)), _use_float_registers(std::move(rhs._use_float_registers)),
            _variable_registers(std::move(rhs._variable_registers)), _variable_float_registers(std::move(rhs._variable_float_registers)),
            last_pseudo_registers(std::move(rhs.last_pseudo_registers)), last_float_pseudo_registers(std::move(rhs.last_float_pseudo_registers)),
            m_loops(std::move(rhs.m_loops)), name(std::move(rhs.name))
        {
    //Reset rhs
    rhs.count = 0;
    rhs.index = 0;
    rhs.last_pseudo_registers = 0;
    rhs.last_float_pseudo_registers = 0;
}

mtac::Function& mtac::Function::operator=(mtac::Function&& rhs){
    _definition = rhs._definition;
    context = std::move(rhs.context); 
    statements = std::move(rhs.statements); 
    _pure = std::move(rhs._pure);
    _standard = std::move(rhs._standard);
    count = std::move(rhs.count); 
    index = std::move(rhs.index);
    entry = std::move(rhs.entry); 
    exit = std::move(rhs.exit); 
    _use_registers = std::move(rhs._use_registers); 
    _use_float_registers = std::move(rhs._use_float_registers);
    _variable_registers = std::move(rhs._variable_registers); 
    _variable_float_registers = std::move(rhs._variable_float_registers);
    last_pseudo_registers = std::move(rhs.last_pseudo_registers); 
    last_float_pseudo_registers = std::move(rhs.last_float_pseudo_registers);
    m_loops = std::move(rhs.m_loops); 
    name = std::move(rhs.name);

    //Reset rhs
    rhs.count = 0;
    rhs.index = 0;
    rhs.last_pseudo_registers = 0;
    rhs.last_float_pseudo_registers = 0;
    
    return *this;
}

mtac::Function::~Function(){
    auto block = exit;

    while(block){
        auto next = block->prev;

        block->next = nullptr;
        block->prev = nullptr;
        block->successors.clear();
        block->predecessors.clear();
        block->dominator = nullptr;
        block->statements.clear();
        block->l_statements.clear();

        block = next;
    }
}

bool mtac::Function::is_main() const {
    return name == "_F4main" || name == "_F4mainAS";
}

bool& mtac::Function::pure(){
    return _pure;
}

bool mtac::Function::pure() const {
    return _pure;
}

bool& mtac::Function::standard(){
    return _standard;
}

bool mtac::Function::standard() const {
    return _standard;
}

mtac::Quadruple& mtac::Function::find(std::size_t uid){
    for(auto& block : *this){
        for(auto& quadruple : block){
            if(quadruple.uid() == uid){
                return quadruple;
            }
        }
    }

    eddic_unreachable("The given uid does not exist");
}

mtac::basic_block_iterator mtac::Function::at(std::shared_ptr<basic_block> bb){
    if(bb){
        return basic_block_iterator(bb, bb->prev);
    } else {
        return basic_block_iterator(nullptr, nullptr);
    }
}
        
mtac::basic_block_p mtac::Function::entry_bb(){
    return entry;
}

mtac::basic_block_p mtac::Function::exit_bb(){
    return exit;
}

mtac::basic_block_p mtac::Function::current_bb(){
    return exit;
}
        
void mtac::Function::create_entry_bb(){
    ++count;

    auto new_block = std::make_shared<mtac::basic_block>(-1);
    new_block->context = context;

    entry = exit = new_block;
}

void mtac::Function::create_exit_bb(){
    ++count;

    auto new_block = std::make_shared<mtac::basic_block>(-2);
    new_block->context = context;
    
    exit->next = new_block;
    new_block->prev = exit;

    exit = new_block;
}

mtac::basic_block_p mtac::Function::new_bb(){
    auto bb = std::make_shared<mtac::basic_block>(++index);
    bb->context = context;
    return bb;
}

mtac::basic_block_p mtac::Function::append_bb(){
    auto new_block = new_bb();
    ++count;
    
    exit->next = new_block;
    new_block->prev = exit;

    exit = new_block;

    return new_block;
}   
        
mtac::basic_block_iterator mtac::Function::insert_before(mtac::basic_block_iterator it, mtac::basic_block_p block){
    auto bb = *it;

    eddic_assert(block, "Cannot add null block"); 
    eddic_assert(it != begin(), "Cannot add before entry");

    block->context = context;
    
    ++count;

    block->prev = bb->prev;
    block->next = bb;
    bb->prev->next = block;
    bb->prev = block;
    
    return at(block);
}

mtac::basic_block_iterator mtac::Function::insert_after(mtac::basic_block_iterator it, mtac::basic_block_p block){
    return insert_before(++it, block);
}

mtac::basic_block_iterator mtac::Function::remove(mtac::basic_block_p block){
    eddic_assert(block, "Cannot remove null block"); 
    eddic_assert(block != exit, "Cannot remove exit"); 

    LOG<Debug>("CFG") << "Remove basic block B" << block->index << log::endl;

    auto& next = block->next;

    --count;

    for(auto& succ : block->successors){
        auto& pred = succ->predecessors;
        
        pred.erase(std::remove_if(pred.begin(), pred.end(), [&block](auto& p){
            return p == block;
        }), pred.end());
    }
    
    for(auto& pred : block->predecessors){
        auto& succ = pred->successors;
        
        succ.erase(std::remove_if(succ.begin(), succ.end(), [&block](auto& p){
            return p == block;
        }), succ.end());

        //If there is a Fall through edge, redirect it
        if(pred == block->prev){
            mtac::make_edge(pred, block->next);
        }
    }

    //Relink the list
    block->prev->next = next;
    next->prev = block->prev;

    //Make sure the removed block do not hold any more references to other blocks
    block->prev = nullptr;
    block->next = nullptr;
    block->dominator = nullptr;
    block->successors.clear();
    block->predecessors.clear();
    block->statements.clear();
    block->l_statements.clear();

    return at(next);
}

mtac::basic_block_iterator mtac::Function::remove(mtac::basic_block_iterator it){
    return remove(*it);
}

mtac::basic_block_iterator mtac::Function::merge_basic_blocks(basic_block_iterator it, std::shared_ptr<basic_block> block){
    auto source = *it; 

    eddic_assert(source->next == block || source->prev == block, "Can only merge sibling blocks");

    LOG<Debug>("CFG") << "Merge " << source->index << " into " << block->index << log::endl;

    if(!source->statements.empty()){
        //B can have some new successors
        for(auto& succ : source->successors){
            if(succ != source->next){
                mtac::make_edge(block, succ);
            }
        }

        //No need to remove the edges, they will be removed by remove call
    }

    //Insert the statements
    if(source->next == block){
        std::move(block->begin(), block->end(), std::back_inserter(source->statements));
        block->statements = std::move(source->statements);
    } else {
        std::move(source->begin(), source->end(), std::back_inserter(block->statements));
    }
    
    //Remove the source basic block
    remove(source);

    return at(block);
}

std::string mtac::Function::get_name() const {
    return name;
}
      
eddic::Function& mtac::Function::definition(){
    return *_definition;
}

const eddic::Function& mtac::Function::definition() const  {
    return *_definition;
}

std::vector<mtac::Quadruple>& mtac::Function::get_statements(){
    return statements;
}

const std::vector<mtac::Quadruple>& mtac::Function::get_statements() const {
    return statements;
}

void mtac::Function::release_statements(){
    statements.clear();
    statements.shrink_to_fit();
}

std::size_t mtac::Function::bb_count() const {
    return count;
}

const std::set<ltac::Register>& mtac::Function::use_registers() const {
    return _use_registers;
}

const std::set<ltac::FloatRegister>& mtac::Function::use_float_registers() const {
    return _use_float_registers;
}

void mtac::Function::use(ltac::Register reg){
    _use_registers.insert(reg);
}

void mtac::Function::use(ltac::FloatRegister reg){
    _use_float_registers.insert(reg);
}

const std::set<ltac::Register>& mtac::Function::variable_registers() const {
    return _variable_registers;
}

const std::set<ltac::FloatRegister>& mtac::Function::variable_float_registers() const {
    return _variable_float_registers;
}

void mtac::Function::variable_use(ltac::Register reg){
    _variable_registers.insert(reg);
}

void mtac::Function::variable_use(ltac::FloatRegister reg){
    _variable_float_registers.insert(reg);
}

std::size_t mtac::Function::pseudo_registers() const {
    return last_pseudo_registers;
}

void mtac::Function::set_pseudo_registers(std::size_t pseudo_registers){
    this->last_pseudo_registers = pseudo_registers;
}
        
std::size_t mtac::Function::pseudo_float_registers() const {
    return last_float_pseudo_registers;
}

void mtac::Function::set_pseudo_float_registers(std::size_t pseudo_registers){
    this->last_float_pseudo_registers = pseudo_registers;
}

std::size_t mtac::Function::size() const {
    std::size_t size = 0;

    for(auto& block : *this){
        size += block->size();
    }

    return size;
}

std::size_t mtac::Function::size_no_nop() const {
    std::size_t size = 0;

    for(auto& block : *this){
        size += block->size_no_nop();
    }

    return size;
}

std::pair<mtac::basic_block_iterator, mtac::basic_block_iterator> mtac::Function::blocks(){
    return std::make_pair(begin(), end());
}

std::vector<mtac::loop>& mtac::Function::loops(){
    return m_loops;
}

std::size_t mtac::Function::position(const basic_block_p& bb) const {
    std::size_t position = 0;

    auto it = begin();

    while(it != end()){
        if(*it == bb){
            return position;
        }
        
        ++position;
        ++it;
    }

    eddic_unreachable("This basic block is not part of the function");
}

bool mtac::operator==(const mtac::Function& lhs, const mtac::Function& rhs){
    return lhs.get_name() == rhs.get_name();
}

std::ostream& eddic::mtac::operator<<(std::ostream& stream, const mtac::Function& function){
    stream << "Function " << function.get_name() << "(pure:" << function.pure() << ")" << std::endl;

    for(auto& quadruple : function.get_statements()){
        stream << quadruple << std::endl;
    }

    for(auto& block : function){
        pretty_print(block, stream);

        for(auto& quadruple : block->statements){
            stream << quadruple << std::endl;
        }
    }

    return stream << std::endl;
}
