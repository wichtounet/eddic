//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ParseNode.h"

using std::list;

ParseNode::~ParseNode(){
	list<ParseNode*>::const_iterator it = childs.begin();
	list<ParseNode*>::const_iterator end = childs.end();

	for( ; it != end; ++it){
		delete *it;
	}
}

void ParseNode::write(ByteCodeFileWriter& writer){
	list<ParseNode*>::const_iterator it = childs.begin();
	list<ParseNode*>::const_iterator end = childs.end();

	for( ; it != end; ++it){
		(*it)->write(writer);
	}
}

void ParseNode::checkVariables(Variables& variables) throw (CompilerException){
	list<ParseNode*>::const_iterator it = childs.begin();
	list<ParseNode*>::const_iterator end = childs.end();

	for( ; it != end; ++it){
		(*it)->checkVariables(variables);
	}
}

void ParseNode::checkStrings(StringPool& pool){
	list<ParseNode*>::const_iterator it = childs.begin();
	list<ParseNode*>::const_iterator end = childs.end();

	for( ; it != end; ++it){
		(*it)->checkStrings(pool);
	}
}

void ParseNode::addLast(ParseNode* node){
	childs.push_back(node);

	node->parent = this;
}

void ParseNode::addFirst(ParseNode* node){
	childs.push_front(node);
	
	node->parent = this;
}

void ParseNode::replace(ParseNode* old, ParseNode* node){
	list<ParseNode*>::iterator it = childs.begin();
	list<ParseNode*>::const_iterator end = childs.end();

	old->parent = NULL;
	node->parent = this;

	for( ; it != end; ++it){
		if(*it == old){
			*it = node;

			return;
		}
	}
}

void ParseNode::remove(ParseNode* node){
	childs.remove(node);

	node->parent = NULL;
}

NodeIterator ParseNode::begin(){
	return childs.begin();
}

NodeIterator ParseNode::end(){
	return childs.end();
}
