//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "logging.hpp"

#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/basic_block.hpp"

using namespace eddic;

mtac::Function::Function(std::shared_ptr<FunctionContext> c, const std::string& n, eddic::Function& definition) : context(c), _definition(&definition), name(n) {
    //Nothing to do   
}
        
mtac::Function::Function(mtac::Function&& rhs) : 
            context(std::move(rhs.context)), _definition(rhs._definition), 
            statements(std::move(rhs.statements)), 
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

bool mtac::Function::is_main() const {
    return name == "_F4main" || name == "_F4mainAS";
}

bool& mtac::Function::pure(){
    return _pure;
}

bool mtac::Function::pure() const {
    return _pure;
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

mtac::basic_block_iterator mtac::Function::begin(){
    return basic_block_iterator(entry, nullptr);
}

mtac::basic_block_iterator mtac::Function::end(){
    return basic_block_iterator(nullptr, exit);    
}

mtac::basic_block_const_iterator mtac::Function::begin() const {
    return basic_block_const_iterator(entry, nullptr);
}

mtac::basic_block_const_iterator mtac::Function::end() const {
    return basic_block_const_iterator(nullptr, exit);
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

void mtac::Function::add(mtac::Quadruple statement){
    statements.push_back(statement);
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
        auto it = iterate(succ->predecessors);

        while(it.has_next()){
            auto pred = *it;

            if(pred == block){
                it.erase();
            } else {
                ++it;
            }
        }
    }
    
    for(auto& pred : block->predecessors){
        auto it = iterate(pred->successors);

        while(it.has_next()){
            auto succ = *it;

            if(succ == block){
                it.erase();
            } else {
                ++it;
            }
        }

        //If there is a Fall through edge, redirect it
        if(pred == block->prev){
            mtac::make_edge(pred, block->next);
        }
    }

    block->successors.clear();
    block->predecessors.clear();

    block->prev->next = next;
    next->prev = block->prev;

    block->prev = nullptr;
    block->next = nullptr;

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
        block->statements.insert(block->statements.begin(), source->statements.begin(), source->statements.end());
    } else {
        block->statements.insert(block->statements.end(), source->statements.begin(), source->statements.end());
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

std::size_t mtac::Function::pseudo_registers(){
    return last_pseudo_registers;
}

void mtac::Function::set_pseudo_registers(std::size_t pseudo_registers){
    this->last_pseudo_registers = pseudo_registers;
}
        
std::size_t mtac::Function::pseudo_float_registers(){
    return last_float_pseudo_registers;
}

void mtac::Function::set_pseudo_float_registers(std::size_t pseudo_registers){
    this->last_float_pseudo_registers = pseudo_registers;
}

std::size_t mtac::Function::size() const {
    std::size_t size = 0;

    for(auto& block : *this){
        size += block->statements.size();
    }

    return size;
}

std::pair<mtac::basic_block_iterator, mtac::basic_block_iterator> mtac::Function::blocks(){
    return std::make_pair(begin(), end());
}

std::vector<mtac::Loop>& mtac::Function::loops(){
    return m_loops;
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
