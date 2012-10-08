//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"

#include "mtac/Function.hpp"

using namespace eddic;

mtac::Function::Function(std::shared_ptr<FunctionContext> c, const std::string& n) : context(c), name(n), _cfg(nullptr) {
    //Nothing to do   
}

std::shared_ptr<mtac::ControlFlowGraph> mtac::Function::cfg(){
    if(!_cfg){
        log::emit<Debug>("CFG") << "Rebuild CFG for " << name << log::endl;
        _cfg = mtac::build_control_flow_graph(shared_from_this());
    }

    return _cfg;
}

void mtac::Function::invalidate_cfg(){
    _cfg = nullptr;
}
        
void mtac::Function::add(Statement statement){
    statements.push_back(statement);
}

mtac::Function::BlockPtr mtac::Function::currentBasicBlock(){
    assert(!basic_blocks.empty());

    return basic_blocks.back();
}

mtac::Function::BlockPtr mtac::Function::newBasicBlock(){
    basic_blocks.push_back(std::make_shared<mtac::BasicBlock>(basic_blocks.size() + 1));
    basic_blocks.back()->context = context;
    return basic_blocks.back();
}   

std::string mtac::Function::getName() const {
    return name;
}

std::vector<mtac::Statement>& mtac::Function::getStatements(){
    return statements;
}

std::size_t mtac::Function::size(){
    std::size_t size = 0;

    for(auto block : getBasicBlocks()){
        size += block->statements.size();
    }

    return size;
}

mtac::Function::BlockList& mtac::Function::getBasicBlocks(){
    return basic_blocks;
}

std::pair<mtac::Function::BlockIterator, mtac::Function::BlockIterator> mtac::Function::blocks(){
    return std::make_pair(basic_blocks.begin(), basic_blocks.end());
}
