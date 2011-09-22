//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <functional>

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

ParseNode::~ParseNode() {
    for_each(begin(), end(), deleter());
    for_each(trash.begin(), trash.end(), deleter());
}

void ParseNode::write(AssemblyFileWriter& writer) {
    for_each(begin(), end(), bind2nd(mem_fun(&ParseNode::write), writer));
}

void ParseNode::checkFunctions(Program& program){
    for_each(begin(), end(), bind2nd(mem_fun(&ParseNode::checkFunctions), program));
}

void ParseNode::checkVariables() {
    for_each(begin(), end(), mem_fun(&ParseNode::checkVariables));
}

void ParseNode::checkStrings(StringPool& pool) {
    for_each(begin(), end(), bind2nd(mem_fun(&ParseNode::checkStrings), pool));
}

void ParseNode::optimize() {
    for_each(begin(), end(), mem_fun(&ParseNode::optimize));
}

void ParseNode::addLast(ParseNode* node) {
    childs.push_back(node);

    node->parent = this;
}

void ParseNode::addFirst(ParseNode* node) {
    childs.push_front(node);

    node->parent = this;
}

void ParseNode::replace(ParseNode* old, ParseNode* node) {
    trash.push_back(old);
    old->parent = NULL;

    node->parent = this;

    list<ParseNode*>::iterator it = find(childs.begin(), childs.end(), old);
    if(it != childs.end()){
        *it = node;
    }
}

void ParseNode::remove(ParseNode* node) {
    childs.remove(node);

    trash.push_back(node);
    node->parent = NULL;
}

NodeIterator ParseNode::begin() {
    return childs.begin();
}

NodeIterator ParseNode::end() {
    return childs.end();
}
