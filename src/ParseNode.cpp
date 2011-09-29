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
#include "AssemblyFileWriter.hpp"
#include "Utils.hpp"

using std::list;
using std::vector;
using std::mem_fun;
using std::bind2nd;

using namespace eddic;

void ParseNode::write(AssemblyFileWriter& writer) {
    for_each(begin(), end(), [&](std::shared_ptr<ParseNode> p){ p->write(writer); });
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
