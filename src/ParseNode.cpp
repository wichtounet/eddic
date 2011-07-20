//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ParseNode.hpp"
#include "Variables.hpp"
#include "StringPool.hpp"
#include "ByteCodeFileWriter.hpp"

using std::list;
using std::vector;

using namespace eddic;

ParseNode::~ParseNode() {
    for (NodeIterator it = childs.begin(); it != childs.end(); ++it) {
        delete *it;
    }

    for (TrashIterator it = trash.begin(); it != trash.end(); ++it) {
        delete *it;
    }
}

void ParseNode::write(ByteCodeFileWriter& writer) {
    NodeIterator it = childs.begin();
    NodeIterator end = childs.end();

    for ( ; it != end; ++it) {
        (*it)->write(writer);
    }
}

void ParseNode::checkVariables(Variables& variables) {
    NodeIterator it = childs.begin();
    NodeIterator end = childs.end();

    for ( ; it != end; ++it) {
        (*it)->checkVariables(variables);
    }
}

void ParseNode::checkStrings(StringPool& pool) {
    NodeIterator it = childs.begin();
    NodeIterator end = childs.end();

    for ( ; it != end; ++it) {
        (*it)->checkStrings(pool);
    }
}

void ParseNode::optimize() {
    NodeIterator it = childs.begin();
    NodeIterator end = childs.end();

    for ( ; it != end; ++it) {
        (*it)->optimize();
    }
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
    list<ParseNode*>::iterator it = childs.begin();
    NodeIterator end = childs.end();

    trash.push_back(old);
    old->parent = NULL;

    node->parent = this;

    for ( ; it != end; ++it) {
        if (*it == old) {
            *it = node;

            return;
        }
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
