//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include "ParseNode.hpp"
#include "Context.hpp"
#include "StringPool.hpp"

#include "AssemblyFileWriter.hpp" //TODO Remove

#include "il/IntermediateProgram.hpp"

#include "Utils.hpp"

using std::list;
using std::vector;
using std::mem_fun;
using std::bind2nd;

using namespace eddic;

ParseNode::ParseNode(std::shared_ptr<Context> context) : m_context(context) {}
ParseNode::ParseNode(std::shared_ptr<Context> context, const std::shared_ptr<Token> token) : m_context(context), m_token(token){} 

std::shared_ptr<Context> ParseNode::context() {
    return m_context;
}

const std::shared_ptr<Token> ParseNode::token(){
    return m_token;
}

void ParseNode::write(AssemblyFileWriter& writer) {
    for_each(begin(), end(), [&](std::shared_ptr<ParseNode> p){ p->write(writer); });
}

void ParseNode::write(IntermediateProgram& program) {
    for_each(begin(), end(), [&](std::shared_ptr<ParseNode> p){ p->write(program); });
}

void ParseNode::checkFunctions(Program& program){
    for_each(begin(), end(), [&](std::shared_ptr<ParseNode> p){ p->checkFunctions(program); });
}

void ParseNode::checkVariables() {
    for_each(begin(), end(), [](std::shared_ptr<ParseNode> p){ p->checkVariables(); });
}

void ParseNode::checkStrings(StringPool& pool) {
    for_each(begin(), end(), [&](std::shared_ptr<ParseNode> p){ p->checkStrings(pool); });
}

void ParseNode::optimize() {
    for_each(begin(), end(), [](std::shared_ptr<ParseNode> p){ p->optimize(); });
}

void ParseNode::addLast(std::shared_ptr<ParseNode> node) {
    childs.push_back(node);

    node->parent = std::weak_ptr<ParseNode>(shared_from_this());
}

void ParseNode::addFirst(std::shared_ptr<ParseNode> node) {
    childs.push_front(node);

    node->parent = std::weak_ptr<ParseNode>(shared_from_this());
}

void ParseNode::replace(std::shared_ptr<ParseNode> old, std::shared_ptr<ParseNode> node) {
    node->parent = std::weak_ptr<ParseNode>(shared_from_this());

    auto it = find(childs.begin(), childs.end(), old);
    if(it != childs.end()){
        *it = node;
    }
}

void ParseNode::remove(std::shared_ptr<ParseNode> node) {
    childs.remove(node);
}

NodeIterator ParseNode::begin() {
    return childs.begin();
}

NodeIterator ParseNode::end() {
    return childs.end();
}
